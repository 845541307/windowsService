#include "user_tracker_service.h"
#include "def.h"
#include <WtsApi32.h>

#pragma comment(lib, "Wtsapi32.lib")


BOOL UserTrackerService::DoRegisterDeviceInterface(
	IN GUID InterfaceClassGuid,
	IN HANDLE hSev,
	OUT HDEVNOTIFY *hDeviceNotify
)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = InterfaceClassGuid;

	*hDeviceNotify = RegisterDeviceNotification(
		hSev,                       // events recipient
		&NotificationFilter,        // type of device
		DEVICE_NOTIFY_SERVICE_HANDLE // type of recipient handle
	);

	if (NULL == *hDeviceNotify)
	{
		WriteToEventLog(_T("RegisterDeviceNotification file"), EVENTLOG_ERROR_TYPE);
		return FALSE;
	}

	return TRUE;
}

BOOL UserTrackerService::DoUnregisterDeviceNotification(HDEVNOTIFY Handle)
{
	return UnregisterDeviceNotification(Handle);
}

bool UserTrackerService::OnInit(DWORD argc, TCHAR* argv[])
{
	bool bSuccess = true;	
	bSuccess = bSuccess && (NULL != (m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL)));
	return bSuccess;
}

void UserTrackerService::OnStart() {
  m_logFile.close();

  // TODO(Olster): Read this path from registry of from command line arguments.
  // This doesn't create non-existent dirs.
  m_logFile.open(_T("D:\\userLog.log"));

  if (!m_logFile.is_open()) {
    WriteToEventLog(_T("Can't open log file"), EVENTLOG_ERROR_TYPE);
  }
  
  bool bSuccess = true;
  ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL,
	  SC_MANAGER_ALL_ACCESS);

  bSuccess = (nullptr != svcControlManager);

  bSuccess = bSuccess && DoRegisterDeviceInterface(GUID_DEVINTERFACE_VOLUME, GetServiceStatusHandle(), &m_hDeviceNotify);

  bSuccess = (NULL != m_hDeviceNotify);
  WaitForSingleObject(m_hStopEvent, INFINITE);

  //end all threads, call service stop status to SCM
  bSuccess = bSuccess && DoUnregisterDeviceNotification(m_hDeviceNotify);
}

void UserTrackerService::OnStop() {
  // Doesn't matter if it's open.
  m_logFile.close();
  
  SetEvent(m_hStopEvent);
}

void UserTrackerService::OnSessionChange(DWORD evtType,
                                         WTSSESSION_NOTIFICATION* notification) {
  // Let's get user name and the action they did.
  TCHAR* buf = nullptr;
  DWORD size = 0;

  BOOL res = ::WTSQuerySessionInformation(nullptr, notification->dwSessionId,
                                          WTSUserName, &buf, &size);

  CString message;

  if (!res) {
    message = _T("Can't get user name ");
  } else {
    SYSTEMTIME sysTime = {0};
    ::GetSystemTime(&sysTime);

    message.Format(_T("%2d.%2d.%4d|%2d:%2d:%2d|User name: %s "),
                   sysTime.wDay, sysTime.wMonth, sysTime.wYear,
                   sysTime.wHour, sysTime.wMinute, sysTime.wSecond, buf);
  }

  ::WTSFreeMemory(buf);

  // Get the event type.
  switch (evtType) {
    case WTS_CONSOLE_CONNECT:
      message.Append(_T("connected."));
    break;

    case WTS_CONSOLE_DISCONNECT:
      message.Append(_T("disconnected."));
    break;

    case WTS_REMOTE_CONNECT:
      message.Append(_T("connected remotely."));
    break;

    case WTS_REMOTE_DISCONNECT:
      message.Append(_T("disconnected remotely."));
    break;

    case WTS_SESSION_LOGON:
      message.Append(_T("logged on."));
    break;

    case WTS_SESSION_LOGOFF:
      message.Append(_T("logged off."));
    break;
   
    case WTS_SESSION_LOCK:
      message.Append(_T("locked the PC."));
    break;

    case WTS_SESSION_UNLOCK:
      message.Append(_T("unlocked the PC."));
    break;

    // Didn't add WTS_SESSION_REMOTE_CONTROL handler.

    default:
      message.Append(_T("performed untracked operation."));
    break;
  }

  if (m_logFile.is_open()) {
    m_logFile << message.GetString() << std::endl;
  }
}

void UserTrackerService::OnDeviceChange(DWORD evtType, DEV_BROADCAST_HDR* devData)
{
	switch (evtType)
	{
	case DBT_DEVICEARRIVAL:
		int a = 1;
		break;
	}
}

void UserTrackerService::OnCustomHandle(DWORD eventId)
{
	switch (eventId)
	{
	case TestEventID:
		int a = 1;
		break;
	}
}
