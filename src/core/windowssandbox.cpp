/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "windowssandbox.h"
#ifdef Q_OS_WIN
#include "base/LemonLog.hpp"
#include "windowsprocessutils.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QDirListing>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUuid>
#include <aclapi.h>
#include <userenv.h>
#include <vector>

#define LEMON_MODULE_NAME "WindowsSandbox"

using namespace Lemon::Windows;

namespace {
	QMutex permissionsMutex;
	QMutex sessionMutex;
	std::weak_ptr<WindowsSandboxSession> activeSession;
	constexpr DWORD PrivateFileAccess =
	    FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | DELETE;

	class SidArray {
	  public:
		SidArray() = default;
		SidArray(const SidArray &) = delete;
		SidArray &operator=(const SidArray &) = delete;
		~SidArray() {
			for (DWORD i = 0; values && i < count; ++i)
				LocalFree(values[i]);

			LocalFree(values);
		}

		PSID **getAddress() { return &values; }
		DWORD *getCountAddress() { return &count; }
		LocalMemory<PSID> takeFirst() {
			if (! count)
				return {};

			return LocalMemory<PSID>(std::exchange(values[0], nullptr));
		}

	  private:
		PSID *values = nullptr;
		DWORD count = 0;
	};

	QString errorText(DWORD code) {
		LocalMemory<LPWSTR> buffer;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		                   FORMAT_MESSAGE_IGNORE_INSERTS,
		               nullptr, code, 0, reinterpret_cast<wchar_t *>(buffer.put()), 0, nullptr);
		const auto text = buffer ? QString::fromWCharArray(buffer.get()).trimmed() : QString();
		return QString("%1 (Windows error %2)").arg(text).arg(code);
	}

	DWORD daclInformation(PSECURITY_DESCRIPTOR descriptor) {
		SECURITY_DESCRIPTOR_CONTROL control = 0;
		DWORD revision = 0;
		GetSecurityDescriptorControl(descriptor, &control, &revision);
		return DACL_SECURITY_INFORMATION |
		       ((control & SE_DACL_PROTECTED) ? PROTECTED_DACL_SECURITY_INFORMATION
		                                      : UNPROTECTED_DACL_SECURITY_INFORMATION);
	}

	Handle openAclFile(const QString &path, bool privateFile) {
		const auto nativePath = QDir::toNativeSeparators(path);
		const DWORD attributes = GetFileAttributesW(wide(nativePath));
		const bool directory =
		    attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
		DWORD flags = FILE_FLAG_BACKUP_SEMANTICS;
		DWORD access = READ_CONTROL | WRITE_DAC | FILE_READ_ATTRIBUTES;
		if (privateFile) {
			flags |= FILE_FLAG_OPEN_REPARSE_POINT;
		} else if (directory) {
			access = MAXIMUM_ALLOWED;
		}
		HANDLE file =
		    CreateFileW(wide(nativePath), access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                nullptr, OPEN_EXISTING, flags, nullptr);
		if (file == INVALID_HANDLE_VALUE && ! directory && ! privateFile &&
		    GetLastError() == ERROR_ACCESS_DENIED) {
			file = CreateFileW(wide(nativePath), READ_CONTROL | FILE_READ_ATTRIBUTES,
			                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
			                   flags, nullptr);
		}
		return Handle(file);
	}

	void revokeSid(const QString &path, PSID sid, bool privateFile = false) {
		auto file = openAclFile(path, privateFile);
		if (! file) {
			const DWORD code = GetLastError();
			if (code != ERROR_FILE_NOT_FOUND && code != ERROR_PATH_NOT_FOUND)
				WARN("Cannot open file to revoke sandbox permissions.", path, errorText(code));
			return;
		}

		LocalMemory<PSECURITY_DESCRIPTOR> descriptor;
		PACL acl = nullptr;
		DWORD status = GetSecurityInfo(file.get(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr,
		                               nullptr, &acl, nullptr, descriptor.put());
		if (status != ERROR_SUCCESS) {
			WARN("Cannot read sandbox permissions for cleanup.", path, errorText(status));
			return;
		}

		bool changed = false;
		for (DWORD i = acl ? acl->AceCount : 0; i > 0; --i) {
			void *entry = nullptr;
			if (! GetAce(acl, i - 1, &entry))
				continue;
			const auto *ace = static_cast<ACCESS_ALLOWED_ACE *>(entry);
			if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&
			    EqualSid(const_cast<DWORD *>(&ace->SidStart), sid)) {
				DeleteAce(acl, i - 1);
				changed = true;
			}
		}
		if (changed) {
			status = SetSecurityInfo(file.get(), SE_FILE_OBJECT, daclInformation(descriptor.get()), nullptr,
			                         nullptr, acl, nullptr);
			if (status != ERROR_SUCCESS)
				WARN("Cannot revoke sandbox permissions.", path, errorText(status));
		}
	}

	struct RuntimeAcl {
		QString path;
		QByteArray sid;
	};

	// QDir("") 表示当前工作目录；此处保留空值，以表示未配置的目录或运行工具。
	QString canonical(const QString &path) { return path.isEmpty() ? QString() : QDir(path).canonicalPath(); }

	bool within(const QString &path, const QString &root) {
		return path == root || path.startsWith(root + '/');
	}

	QString hash(const QString &text) {
		return QString::fromLatin1(
		    QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256).toHex());
	}

	QString stamp(const QString &path) {
		const QFileInfo info(path);
		return QString("%1:%2").arg(info.size()).arg(info.lastModified().toMSecsSinceEpoch());
	}

	bool hasGrant(PACL acl, PSID sid, DWORD rights) {
		if (! acl || ! rights)
			return true;

		DWORD remaining = rights;
		GENERIC_MAPPING mapping{FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS};
		for (DWORD i = 0; i < acl->AceCount; ++i) {
			void *entry = nullptr;
			if (! GetAce(acl, i, &entry))
				return false;

			const auto *header = static_cast<ACE_HEADER *>(entry);
			if (header->AceFlags & INHERIT_ONLY_ACE)
				continue;
			if (header->AceType != ACCESS_ALLOWED_ACE_TYPE && header->AceType != ACCESS_DENIED_ACE_TYPE)
				continue;
			const auto *ace = static_cast<ACCESS_ALLOWED_ACE *>(entry);
			if (! EqualSid(const_cast<DWORD *>(&ace->SidStart), sid))
				continue;

			DWORD mask = ace->Mask;
			MapGenericMask(&mask, &mapping);
			if (header->AceType == ACCESS_DENIED_ACE_TYPE) {
				if (mask & remaining)
					return false;
			} else {
				remaining &= ~mask;
				if (! remaining)
					return true;
			}
		}
		return false;
	}
} // namespace

class WindowsSandboxSession {
  public:
	QString id = QUuid::createUuid().toString(QUuid::Id128);
	QByteArray allPackagesSid;
	QHash<QString, QString> runtimes;
	QHash<QString, QStringList> pythonRoots;
	std::vector<RuntimeAcl> grants;

	~WindowsSandboxSession() {
		QMutexLocker lock(&permissionsMutex);
		for (auto it = grants.rbegin(); it != grants.rend(); ++it)
			revokeSid(it->path, it->sid.data());
	}
};

struct WindowsSandbox::Data {
	const ProcessRunnerConfig &config;
	QElapsedTimer timer;

	QString profileName;
	PSID packageSid = nullptr;
	QString workingDirectory;

	std::shared_ptr<WindowsSandboxSession> session;
	QProcessEnvironment childEnvironment;
	SandboxSettings::Runtime runtime = SandboxSettings::Native;

	std::vector<SID_AND_ATTRIBUTES> capabilityList;
	std::vector<LocalMemory<PSID>> capabilitySids;
	Handle job;
	std::vector<BYTE> attributeBuffer;
	LPPROC_THREAD_ATTRIBUTE_LIST attributeList = nullptr;
	SECURITY_CAPABILITIES processCapabilities{};
	HANDLE inheritedHandles[3]{};
	HANDLE jobs[1]{};

	int updates = 0;
	bool hit = true;
	QString error;

	explicit Data(const ProcessRunnerConfig &config)
	    : config(config),
	      session(config.sandboxSession ? config.sandboxSession : WindowsSandbox::createSession()) {}

	~Data() {
		job.reset();

		if (! workingDirectory.isEmpty()) {
			QMutexLocker lock(&permissionsMutex);
			revokeSid(workingDirectory, packageSid, true);
			const QDirListing files(workingDirectory, QDirListing::IteratorFlag::Recursive |
			                                              QDirListing::IteratorFlag::IncludeHidden);
			for (const auto &file : files)
				revokeSid(file.filePath(), packageSid, true);
		}

		if (attributeList)
			DeleteProcThreadAttributeList(attributeList);
		if (packageSid) {
			DeleteAppContainerProfile(wide(profileName));
			FreeSid(packageSid);
		}
	}

	bool checkBudget() {
		if (timer.elapsed() >= config.sandboxSettings.preparationTimeLimit) {
			return fail(
			    QObject::tr(
			        "Windows sandbox preparation exceeded %1 seconds. Increase the preparation limit in "
			        "compiler settings.")
			        .arg(config.sandboxSettings.preparationTimeLimit / 1000));
		}
		return true;
	}

	bool fail(const QString &text) {
		error = text;
		return false;
	}

	bool fail(const QString &operation, const QString &path, DWORD code = GetLastError()) {
		WARN(operation, path, errorText(code));
		error = QObject::tr("Windows sandbox preparation failed. See the log for details.");
		return false;
	}

	bool processFailure(const QString &operation, DWORD code = GetLastError()) {
		WARN(operation, errorText(code));
		error = QObject::tr("Internal error (See log for further information)");
		return false;
	}

	bool prepareProcess(STARTUPINFOEXW &startup) {
		DWORD handleCount = 0;
		for (HANDLE handle :
		     {startup.StartupInfo.hStdInput, startup.StartupInfo.hStdOutput, startup.StartupInfo.hStdError}) {
			if (handle)
				inheritedHandles[handleCount++] = handle;
		}
		for (DWORD i = 0; i < handleCount; ++i)
			if (! inheritedHandles[i] || inheritedHandles[i] == INVALID_HANDLE_VALUE)
				return processFailure("Cannot open sandbox standard streams.", ERROR_INVALID_HANDLE);

		job.reset(CreateJobObjectW(nullptr, nullptr));
		if (! job)
			return processFailure("Cannot create sandbox job.");
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
		limits.BasicLimitInformation.LimitFlags =
		    JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		// Use the job limit because the child-process creation
		// attribute can fail AppContainer DLL initialization on Windows.
		limits.BasicLimitInformation.ActiveProcessLimit = 1;
		if (! SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
			return processFailure("Cannot set sandbox process limits.");
		JOBOBJECT_BASIC_UI_RESTRICTIONS uiLimits{};
		uiLimits.UIRestrictionsClass =
		    JOB_OBJECT_UILIMIT_DESKTOP | JOB_OBJECT_UILIMIT_DISPLAYSETTINGS | JOB_OBJECT_UILIMIT_EXITWINDOWS |
		    JOB_OBJECT_UILIMIT_GLOBALATOMS | JOB_OBJECT_UILIMIT_HANDLES | JOB_OBJECT_UILIMIT_READCLIPBOARD |
		    JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS | JOB_OBJECT_UILIMIT_WRITECLIPBOARD;
		if (! SetInformationJobObject(job.get(), JobObjectBasicUIRestrictions, &uiLimits, sizeof(uiLimits)))
			return processFailure("Cannot restrict sandbox desktop access.");
		SIZE_T attributeSize = 0;
		InitializeProcThreadAttributeList(nullptr, 3, 0, &attributeSize);
		attributeBuffer.resize(attributeSize);
		auto *list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeBuffer.data());
		if (! InitializeProcThreadAttributeList(list, 3, 0, &attributeSize))
			return processFailure("Cannot initialize sandbox process attributes.");
		attributeList = list;
		startup.lpAttributeList = attributeList;
		processCapabilities = {packageSid, capabilityList.data(), DWORD(capabilityList.size()), 0};
		jobs[0] = job.get();
		if (! UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
		                                &processCapabilities, sizeof(processCapabilities), nullptr,
		                                nullptr) ||
		    ! UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritedHandles,
		                                handleCount * sizeof(HANDLE), nullptr, nullptr) ||
		    ! UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_JOB_LIST, jobs, sizeof(jobs),
		                                nullptr, nullptr)) {
			return processFailure("Cannot configure sandbox process attributes.");
		}
		return true;
	}

	// Runtime directory grants do not propagate. Work directory grants retain Windows ACL inheritance.
	bool grantFile(const QString &path, PSID sid) {
		if (! checkBudget())
			return false;
		const bool privateFile = sid == packageSid;
		DWORD access = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
		if (privateFile)
			access = PrivateFileAccess;
		auto file = openAclFile(path, privateFile);
		if (! file)
			return fail("Cannot open sandbox resource.", path);
		BY_HANDLE_FILE_INFORMATION info{};
		if (! GetFileInformationByHandle(file.get(), &info))
			return fail("Cannot inspect sandbox resource.", path);
		if (privateFile &&
		    ((info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) || info.nNumberOfLinks > 1)) {
			return fail(QObject::tr("Sandbox files must not be reparse points or hard links: %1").arg(path));
		}
		PACL acl = nullptr;
		LocalMemory<PSECURITY_DESCRIPTOR> descriptor;
		LocalMemory<PACL> updatedAcl;
		DWORD status = GetSecurityInfo(file.get(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr,
		                               nullptr, &acl, nullptr, descriptor.put());
		if (status != ERROR_SUCCESS)
			return fail("Cannot read sandbox resource permissions.", path, status);
		if (! acl || (! privateFile &&
		              (hasGrant(acl, sid, access) || hasGrant(acl, session->allPackagesSid.data(), access))))
			return true;
		EXPLICIT_ACCESSW entry{};
		entry.grfAccessPermissions = access;
		entry.grfAccessMode = GRANT_ACCESS;
		// Work directories pass sandbox permissions to new files; runtime grants stay non-inheritable.
		entry.grfInheritance = privateFile && (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		                           ? SUB_CONTAINERS_AND_OBJECTS_INHERIT
		                           : NO_INHERITANCE;
		BuildTrusteeWithSidW(&entry.Trustee, sid);
		status = SetEntriesInAclW(1, &entry, acl, updatedAcl.put());
		if (status != ERROR_SUCCESS)
			return fail("Cannot construct sandbox resource permissions.", path, status);
		status = SetSecurityInfo(file.get(), SE_FILE_OBJECT, daclInformation(descriptor.get()), nullptr,
		                         nullptr, updatedAcl.get(), nullptr);
		if (status != ERROR_SUCCESS) {
			return fail("Cannot set sandbox permissions. Use a user-owned runtime installation or have its "
			            "owner prepare read and execute access.",
			            path, status);
		}
		if (! privateFile) {
			QByteArray savedSid(GetLengthSid(sid), Qt::Uninitialized);
			CopySid(DWORD(savedSid.size()), savedSid.data(), sid);
			session->grants.push_back({path, std::move(savedSid)});
			++updates;
		}
		return true;
	}

	PSID addCapability(const QString &name) {
		const auto derive = reinterpret_cast<decltype(&DeriveCapabilitySidsFromName)>(
		    GetProcAddress(GetModuleHandleW(L"kernelbase.dll"), "DeriveCapabilitySidsFromName"));
		if (! derive) {
			fail("Cannot load named sandbox capability API.", {});
			return nullptr;
		}
		SidArray groups;
		SidArray sids;
		if (! derive(wide(name), groups.getAddress(), groups.getCountAddress(), sids.getAddress(),
		             sids.getCountAddress())) {
			fail("Cannot derive sandbox capability.", name);
			return nullptr;
		}
		auto sid = sids.takeFirst();
		if (! sid) {
			fail("Cannot derive sandbox capability.", name, ERROR_INVALID_SID);
			return nullptr;
		}
		const auto result = sid.get();
		capabilitySids.push_back(std::move(sid));
		capabilityList.push_back({result, SE_GROUP_ENABLED});
		return result;
	}

	bool prepareRuntime(const QString &root, const QString &tool, bool dllsOnly) {
		const auto key = hash("lemonlime.runtime.v1:" + root + (dllsOnly ? ":dlls" : ":tree"));
		PSID sid = addCapability("lemonlime.runtime." + session->id + '.' + key);
		if (! sid)
			return false;
		const auto cacheKey = key + '/' + hash(tool);
		const auto expected = stamp(tool) + ':' + stamp(root);
		const int previous = updates;
		if (! grantFile(root, sid))
			return false;
		if (session->runtimes.value(cacheKey) == expected && updates == previous)
			return true;
		hit = false;
		QDirIterator iterator(root, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
		                      dllsOnly ? QDirIterator::NoIteratorFlags : QDirIterator::Subdirectories);
		while (iterator.hasNext()) {
			if (! checkBudget())
				return false;
			iterator.next();
			const auto info = iterator.fileInfo();
			if (dllsOnly && (info.isDir() || info.suffix().compare("dll", Qt::CaseInsensitive) != 0))
				continue;
			if (! grantFile(info.filePath(), sid))
				return false;
		}
		session->runtimes[cacheKey] = expected;
		return true;
	}

	bool discoverJava(const QString &tool, QStringList &roots) {
		QDir home(QFileInfo(tool).absolutePath());
		if (home.dirName().compare("bin", Qt::CaseInsensitive) == 0)
			home.cdUp();
		if (! QFileInfo::exists(home.filePath("lib")))
			return fail(QObject::tr("Select java.exe inside a Java runtime installation: %1").arg(tool));
		roots.append(home.absolutePath());
		return true;
	}

	bool discoverPython(const QString &tool, QStringList &roots) {
		const auto key = tool + ':' + stamp(tool);
		roots = session->pythonRoots.value(key);
		if (roots.isEmpty()) {
			// Only probe the explicitly configured, trusted interpreter. Never execute a submission here.
			QTemporaryDir discoveryDirectory;
			if (! discoveryDirectory.isValid())
				return fail("Cannot create Python discovery directory.", {});
			QProcess probe;
			probe.setWorkingDirectory(discoveryDirectory.path());
			probe.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
			probe.start(tool,
			            {"-I", "-S", "-c",
			             "import sys,json;print(json.dumps([sys.base_prefix,sys.prefix,sys.executable]))"});
			if (! probe.waitForStarted(1000))
				return fail("Cannot inspect the selected Python interpreter.", tool, ERROR_FILE_NOT_FOUND);
			while (! probe.waitForFinished(25)) {
				if (! checkBudget())
					return false;
				if (probe.bytesAvailable() > 65536)
					return fail(QObject::tr("Python runtime discovery produced excessive output."));
			}
			const auto values = QJsonDocument::fromJson(probe.readAllStandardOutput()).array();
			if (probe.exitStatus() != QProcess::NormalExit || probe.exitCode() != 0 || values.size() != 3) {
				return fail(QObject::tr("Cannot discover the Python runtime: %1\n%2")
				                .arg(tool, QString::fromUtf8(probe.readAllStandardError().left(2048))));
			}
			roots = {values[0].toString(), values[1].toString(),
			         QFileInfo(values[2].toString()).absolutePath()};
			session->pythonRoots[key] = roots;
		}
		QDir venv(QFileInfo(tool).absolutePath());
		if (venv.dirName().compare("Scripts", Qt::CaseInsensitive) == 0 && venv.cdUp() &&
		    QFileInfo::exists(venv.filePath("pyvenv.cfg"))) {
			roots.append(venv.absolutePath());
		}
		return true;
	}

	bool discover(const QString &tool, QStringList &roots) {
		if (runtime == SandboxSettings::Java) {
			if (! discoverJava(tool, roots))
				return false;
		} else if (runtime == SandboxSettings::Python) {
			if (! discoverPython(tool, roots))
				return false;
		} else if (! tool.isEmpty()) {
			roots.append(QFileInfo(tool).absolutePath());
		}
		for (auto &root : roots)
			root = canonical(root);
		roots.removeDuplicates();
		return true;
	}

	bool prepareRuntimePermissions(const QString &tool, QStringList &roots) {
		while (! permissionsMutex.tryLock(25)) {
			if (! checkBudget())
				return false;
		}

		auto unlock = qScopeGuard([&] { permissionsMutex.unlock(); });
		if (session->allPackagesSid.isEmpty()) {
			QByteArray sid(SECURITY_MAX_SID_SIZE, Qt::Uninitialized);
			DWORD size = DWORD(sid.size());
			if (! CreateWellKnownSid(WinBuiltinAnyPackageSid, nullptr, sid.data(), &size))
				return fail("Cannot create the all-packages identity.", {});
			sid.resize(size);
			session->allPackagesSid = std::move(sid);
		}

		const auto firstGrant = session->grants.size();
		const auto previousRuntimes = session->runtimes;
		auto rollback = qScopeGuard([&] {
			while (session->grants.size() > firstGrant) {
				auto &grant = session->grants.back();
				revokeSid(grant.path, grant.sid.data());
				session->grants.pop_back();
			}
			session->runtimes = previousRuntimes;
		});

		if (! discover(tool, roots))
			return false;

		for (const auto &root : roots) {
			if (! prepareRuntime(root, tool, runtime == SandboxSettings::Native))
				return false;
		}

		for (const auto &root : config.sandboxSettings.readOnlyDirectories) {
			const auto resolved = canonical(root);
			if (! prepareRuntime(resolved, tool, false))
				return false;
			roots.append(resolved);
		}

		if (! tool.isEmpty() && runtime != SandboxSettings::Native) {
			bool covered = false;
			for (const auto &root : roots)
				covered = covered || within(tool, root);
			if (! covered) {
				// A launcher outside the runtime (for example uv) needs access to its own file.
				PSID sid = addCapability("lemonlime.launcher." + session->id + '.' + hash(tool));
				if (! sid || ! grantFile(tool, sid))
					return false;
			}
		}

		rollback.dismiss();
		return true;
	}

	bool preparePrivateFiles() {
		workingDirectory = config.workingDirectory;
		while (! permissionsMutex.tryLock(25)) {
			if (! checkBudget())
				return false;
		}
		auto unlock = qScopeGuard([&] { permissionsMutex.unlock(); });
		QStringList paths;
		QDirIterator files(workingDirectory,
		                   QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
		                   QDirIterator::Subdirectories);
		while (files.hasNext()) {
			if (! checkBudget())
				return false;
			paths.prepend(files.next());
		}
		// Validate and grant descendants before their parents can propagate inheritable permissions.
		for (const auto &path : paths) {
			if (! grantFile(path, packageSid))
				return false;
		}
		return grantFile(workingDirectory, packageSid);
	}

	bool prepare() {
		timer.start();
		if (! checkBudget())
			return false;

		profileName = "LemonLime.Run." + QUuid::createUuid().toString(QUuid::Id128);
		const HRESULT status = CreateAppContainerProfile(wide(profileName), wide(profileName),
		                                                 L"LemonLime judging", nullptr, 0, &packageSid);
		if (FAILED(status))
			return fail("Cannot create AppContainer.", {}, DWORD(status));

		QString tool = config.runtimeExecutable;
		if (! tool.isEmpty() && QFileInfo(tool).isRelative()) {
			tool = QStandardPaths::findExecutable(
			    tool, config.environment.value("PATH").split(';', Qt::SkipEmptyParts));
		}
		tool = canonical(tool);
		if (! config.runtimeExecutable.isEmpty() && tool.isEmpty()) {
			return fail(QObject::tr("The configured sandbox runtime executable does not exist: %1")
			                .arg(config.runtimeExecutable));
		}

		runtime = config.sandboxSettings.runtime;
		if (runtime == SandboxSettings::Automatic) {
			const auto name = QFileInfo(tool).baseName();
			if (name.startsWith("python", Qt::CaseInsensitive) ||
			    name.startsWith("pypy", Qt::CaseInsensitive))
				runtime = SandboxSettings::Python;
			else if (name.compare("java", Qt::CaseInsensitive) == 0)
				runtime = SandboxSettings::Java;
			else
				runtime = SandboxSettings::Native;
		}

		QStringList roots;
		if (! prepareRuntimePermissions(tool, roots) || ! preparePrivateFiles())
			return false;

		const auto system = QProcessEnvironment::systemEnvironment();
		// Preserve explicitly configured variables, not the host's entire environment.
		childEnvironment = config.runtimeEnvironment;
		for (const auto &name :
		     {"SystemRoot", "WINDIR", "SystemDrive", "NUMBER_OF_PROCESSORS", "PROCESSOR_ARCHITECTURE"}) {
			if (system.contains(name))
				childEnvironment.insert(name, system.value(name));
		}

		QStringList path;
		if (! tool.isEmpty())
			path.append(QFileInfo(tool).absolutePath());
		for (const auto &root : roots) {
			path.append(root);
			if (QFileInfo::exists(root + "/bin"))
				path.append(root + "/bin");
		}
		path.append(system.value("SystemRoot") + "/System32");
		path.removeDuplicates();
		childEnvironment.insert("PATH", QDir::toNativeSeparators(path.join(';')));

		// Avoid ERROR_ENVVAR_NOT_FOUND during AppContainer creation when LOCALAPPDATA is absent.
		if (! childEnvironment.contains("LOCALAPPDATA"))
			childEnvironment.insert("LOCALAPPDATA", QDir::toNativeSeparators(workingDirectory));
		return checkBudget();
	}
};

std::shared_ptr<WindowsSandboxSession> WindowsSandbox::createSession() {
	QMutexLocker lock(&sessionMutex);
	auto session = activeSession.lock();
	if (! session) {
		session = std::make_shared<WindowsSandboxSession>();
		activeSession = session;
	}
	return session;
}

WindowsSandbox::WindowsSandbox(const ProcessRunnerConfig &config) : data(std::make_unique<Data>(config)) {}
WindowsSandbox::~WindowsSandbox() = default;
bool WindowsSandbox::prepare(QString &error) {
	const bool result = data->prepare();
	error = data->error;
	return result;
}

bool WindowsSandbox::prepareProcess(STARTUPINFOEXW &startup, QString &error) {
	const bool result = data->prepareProcess(startup);
	error = data->error;
	return result;
}
const QProcessEnvironment &WindowsSandbox::environment() const { return data->childEnvironment; }
int WindowsSandbox::aclUpdates() const { return data->updates; }
bool WindowsSandbox::cacheHit() const { return data->hit; }
#endif
