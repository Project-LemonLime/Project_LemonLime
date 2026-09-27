/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <QtGlobal>
#ifdef Q_OS_WIN
#include <QString>
#include <type_traits>
#include <utility>
#include <windows.h>

namespace Lemon::Windows {
	class Handle {
	  public:
		explicit Handle(HANDLE value = nullptr) : value(value) {}
		~Handle() { reset(); }

		Handle(const Handle &) = delete;
		Handle &operator=(const Handle &) = delete;

		HANDLE get() const { return value; }
		HANDLE *put() {
			reset();
			return &value;
		}

		explicit operator bool() const { return value && value != INVALID_HANDLE_VALUE; }
		void reset(HANDLE replacement = nullptr) {
			if (*this)
				CloseHandle(value);

			value = replacement;
		}

	  private:
		HANDLE value;
	};

	struct LocalDeleter {
		void operator()(void *memory) const noexcept { LocalFree(memory); }
	};

	template <typename Pointer> class LocalMemory {
		static_assert(std::is_pointer_v<Pointer>, "LocalMemory requires a pointer type");

	  public:
		LocalMemory() noexcept = default;
		explicit LocalMemory(Pointer value) noexcept : value(value) {}
		~LocalMemory() { reset(); }

		LocalMemory(const LocalMemory &) = delete;
		LocalMemory &operator=(const LocalMemory &) = delete;

		LocalMemory(LocalMemory &&other) noexcept : value(other.release()) {}
		LocalMemory &operator=(LocalMemory &&other) noexcept {
			if (this != &other)
				reset(other.release());

			return *this;
		}

		Pointer get() const noexcept { return value; }
		Pointer *put() noexcept {
			reset();
			return &value;
		}
		Pointer release() noexcept { return std::exchange(value, nullptr); }

		explicit operator bool() const noexcept { return value != nullptr; }
		void reset(Pointer replacement = nullptr) noexcept {
			if (value != replacement) {
				if (value)
					LocalDeleter{}(value);

				value = replacement;
			}
		}

	  private:
		Pointer value = nullptr;
	};

	inline const wchar_t *wide(const QString &text) {
		return reinterpret_cast<const wchar_t *>(text.utf16());
	}
} // namespace Lemon::Windows
#endif
