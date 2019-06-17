#pragma once
#include <Windows.h>
namespace {
	class ServiceHandle {
	public:
		ServiceHandle(SC_HANDLE handle)
			: m_handle(handle) {}

		~ServiceHandle() {
			if (m_handle) {
				::CloseServiceHandle(m_handle);
			}
		}

		operator SC_HANDLE() {
			return m_handle;
		}

	private:
		SC_HANDLE m_handle = nullptr;
	};
}