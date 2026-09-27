/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <winsock2.h>

static bool readsSecret(HANDLE handle) {
#ifdef _MSC_VER
	__try {
#endif
		char buffer[64]{};
		DWORD length = 0;
		return ReadFile(handle, buffer, sizeof(buffer), &length, nullptr) && length >= 6 &&
		       memcmp(buffer, "secret", 6) == 0;
#ifdef _MSC_VER
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
#endif
}

static void printJsonPath(const std::wstring &path) {
	putchar('"');
	for (wchar_t c : path)
		if (c == L'\\')
			putchar('/');
		else if (c >= 0x20 && c <= 0x7e && c != L'"')
			putchar(static_cast<char>(c));
		else
			printf("\\u%04x", static_cast<unsigned int>(c));
	putchar('"');
}

int wmain(int argc, wchar_t **argv) {
	if (argc < 2)
		return 1;
	const std::wstring mode = argv[1];
	if (mode == L"arguments") {
		for (int i = 2; i < argc; ++i)
			wprintf(L"%ls\n", argv[i]);
		return 0;
	}

	if (mode == L"-I") {
		wchar_t executable[32768]{};
		if (! GetModuleFileNameW(nullptr, executable, 32768))
			return 27;
		const std::wstring path = executable;
		const auto separator = path.find_last_of(L"\\/");
		if (path.substr(separator + 1) == L"python.exe") {
			// Supply a small runtime fixture for the Python policy without requiring an installation.
			putchar('[');
			printJsonPath(path.substr(0, separator));
			putchar(',');
			printJsonPath(path.substr(0, separator));
			putchar(',');
			printJsonPath(path);
			puts("]");
			return 0;
		}
		if (path.substr(separator + 1) != L"slow-python.exe")
			return 1;
		// Simulate a Python interpreter that remains busy during runtime discovery.
		const auto marker = path + L".started";
		FILE *file = _wfopen(marker.c_str(), L"w");
		if (! file)
			return 28;
		fprintf(file, "%lu\n", GetCurrentProcessId());
		fclose(file);
		Sleep(10000);
		return 29;
	}

	if (mode == L"sum") {
		int a = 0, b = 0;
		if (scanf("%d%d", &a, &b) != 2)
			return 2;
		printf("%d\n", a + b);
		return 0;
	}

	if (mode == L"file-io") {
		const bool standardInput = argc > 4 && _wtoi(argv[4]) != 0;
		const bool standardOutput = argc > 5 && _wtoi(argv[5]) != 0;
		const HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
		const HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if ((inputHandle && inputHandle != INVALID_HANDLE_VALUE) != standardInput ||
		    (outputHandle && outputHandle != INVALID_HANDLE_VALUE) != standardOutput)
			return 26;
		FILE *input = standardInput ? stdin : _wfopen(argv[2], L"r");
		FILE *output = standardOutput ? stdout : _wfopen(argv[3], L"w");
		if (! input || ! output)
			return 23;
		int a = 0, b = 0;
		const int count = fscanf(input, "%d%d", &a, &b);
		fprintf(output, "%d\n", a + b);
		fclose(input);
		fclose(output);
		return count == 2 ? 0 : 24;
	}

	if (mode == L"stderr") {
		const std::string text(1100, 'a');
		fwrite(text.data(), 1, text.size(), stderr);
		fputs("tail", stderr);
		return 17;
	}

	if (mode == L"isolation") {
		HANDLE token = nullptr;
		DWORD app = 0, size = 0;
		if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token) ||
		    ! GetTokenInformation(token, TokenIsAppContainer, &app, sizeof(app), &size) || ! app)
			return 3;
		CloseHandle(token);
		HANDLE secret = CreateFileW(argv[2], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
		                            OPEN_EXISTING, 0, nullptr);
		if (secret != INVALID_HANDLE_VALUE) {
			CloseHandle(secret);
			return 4;
		}
		HANDLE own = CreateFileW(L"own.txt", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
		if (own == INVALID_HANDLE_VALUE)
			return 5;
		CloseHandle(own);
		if (argc > 3) {
			const HANDLE inherited = reinterpret_cast<HANDLE>(_wcstoui64(argv[3], nullptr, 10));
			if (readsSecret(inherited))
				return 6;
		}
		puts("isolated");
		return 0;
	}

	if (mode == L"host") {
		HANDLE token = nullptr;
		DWORD app = 1, size = 0;
		if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token) ||
		    ! GetTokenInformation(token, TokenIsAppContainer, &app, sizeof(app), &size) || app)
			return 20;
		CloseHandle(token);
		HANDLE secret = CreateFileW(argv[2], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
		                            OPEN_EXISTING, 0, nullptr);
		if (secret == INVALID_HANDLE_VALUE || ! readsSecret(secret))
			return 21;
		CloseHandle(secret);
		wchar_t value[32]{};
		if (! GetEnvironmentVariableW(L"LEMON_SANDBOX_TEST", value, 32) ||
		    std::wstring(value) != L"preserved")
			return 22;
		puts("host");
		return 0;
	}

	if (mode == L"work-files") {
		if (! CreateDirectoryW(L"created", nullptr) || ! CreateDirectoryW(L"created\\nested", nullptr))
			return 30;
		const auto name = L"created\\nested\\data.txt";
		HANDLE file =
		    CreateFileW(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return 31;
		DWORD written = 0;
		const bool success = WriteFile(file, "created", 7, &written, nullptr) && written == 7;
		CloseHandle(file);
		if (! success)
			return 32;
		file = CreateFileW(name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return 33;
		char contents[7]{};
		DWORD read = 0;
		const bool reopened = ReadFile(file, contents, sizeof(contents), &read, nullptr) && read == 7 &&
		                      memcmp(contents, "created", 7) == 0;
		CloseHandle(file);
		return reopened ? 0 : 34;
	}

	if (mode == L"runtime") {
		HANDLE file = CreateFileW(argv[2], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
		                          OPEN_EXISTING, 0, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return 7;
		CloseHandle(file);
		file = CreateFileW(argv[2], GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
		                   0, nullptr);
		if (file != INVALID_HANDLE_VALUE) {
			CloseHandle(file);
			return 8;
		}
		puts("readonly");
		return 0;
	}

	if (mode == L"network") {
		WSADATA data{};
		WSAStartup(MAKEWORD(2, 2), &data);
		SOCKET socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		u_long nonblocking = 1;
		ioctlsocket(socketHandle, FIONBIO, &nonblocking);
		sockaddr_in address{};
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		address.sin_port = htons(static_cast<u_short>(_wtoi(argv[2])));
		const int connected = connect(socketHandle, reinterpret_cast<sockaddr *>(&address), sizeof(address));
		int error = WSAGetLastError();
		bool blocked = connected == SOCKET_ERROR && error == WSAEACCES;
		if (connected == SOCKET_ERROR && error == WSAEWOULDBLOCK) {
			fd_set writable;
			FD_ZERO(&writable);
			FD_SET(socketHandle, &writable);
			timeval timeout{0, 500000};
			blocked = select(0, nullptr, &writable, nullptr, &timeout) == 0;
			if (! blocked) {
				int size = sizeof(error);
				getsockopt(socketHandle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &size);
				blocked = error == WSAEACCES;
			}
		}
		closesocket(socketHandle);
		WSACleanup();
		printf("connect=%d error=%d\n", connected, error);
		return blocked ? 0 : 9;
	}

	if (mode == L"burn" || mode == L"metrics") {
		printf("process=%lu\n", GetCurrentProcessId());
		fflush(stdout);
		if (mode == L"metrics") {
			// Separate committed memory from the working set without touching these pages.
			if (! VirtualAlloc(nullptr, 64 * 1024 * 1024, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))
				return 25;
			// Exercise kernel time before consuming user time in the same process.
			const ULONGLONG kernelDeadline = GetTickCount64() + 1000;
			for (;;) {
				FILETIME created{}, exited{}, kernel{}, user{};
				if (! GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
					return 35;
				const ULONGLONG kernelTicks = (ULONGLONG(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
				if (kernelTicks >= 10000)
					break;
				if (GetTickCount64() >= kernelDeadline)
					return 36;
			}
		}
		const ULONGLONG until = GetTickCount64() + (argc > 2 ? _wtoi(argv[2]) : 400);
		volatile unsigned long value = 1;
		while (GetTickCount64() < until)
			for (int i = 0; i < 10000; ++i)
				value = value * 1664525 + 1013904223;
		puts("burned");
		return 0;
	}

	if (mode == L"spawn" || mode == L"deny-child") {
		wchar_t executable[32768]{};
		GetModuleFileNameW(nullptr, executable, 32768);
		std::wstring command = L"\"" + std::wstring(executable) + L"\" burn " + (argc > 2 ? argv[2] : L"400");
		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION child{};
		const BOOL created = CreateProcessW(executable, command.data(), nullptr, nullptr, FALSE,
		                                    CREATE_NO_WINDOW, nullptr, nullptr, &startup, &child);
		if (! created) {
			printf("child-denied=%lu\n", GetLastError());
			return mode == L"deny-child" ? 0 : 10;
		}
		printf("parent=%lu child=%lu\n", GetCurrentProcessId(), child.dwProcessId);
		fflush(stdout);
		CloseHandle(child.hThread);
		if (mode == L"deny-child") {
			TerminateProcess(child.hProcess, 1);
			WaitForSingleObject(child.hProcess, INFINITE);
			CloseHandle(child.hProcess);
			return 11;
		}
		WaitForSingleObject(child.hProcess, INFINITE);
		DWORD code = 0;
		GetExitCodeProcess(child.hProcess, &code);
		CloseHandle(child.hProcess);
		return int(code);
	}

	if (mode == L"memory") {
		for (int i = 0; i < 256; ++i) {
			auto *memory = static_cast<volatile char *>(
			    VirtualAlloc(nullptr, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
			if (! memory) {
				Sleep(100);
				return 12;
			}
			for (int j = 0; j < 1024 * 1024; j += 4096)
				memory[j] = 1;
		}
		return 13;
	}
	return 14;
}
