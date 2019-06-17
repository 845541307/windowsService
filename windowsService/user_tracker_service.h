#ifndef USER_TRACKER_SERVICE_H_
#define USER_TRACKER_SERVICE_H_

#include <fstream>

#include "service_base.h"


#define TestEventID		128

class UserTrackerService : public ServiceBase {
 public:
  UserTrackerService(const UserTrackerService& other) = delete;
  UserTrackerService& operator=(const UserTrackerService& other) = delete;

  UserTrackerService(UserTrackerService&& other) = delete;
  UserTrackerService& operator=(UserTrackerService&& other) = delete;

  UserTrackerService()
   : ServiceBase(_T("TestService"),
                 _T("TestService"),
                 SERVICE_DEMAND_START,
                 SERVICE_ERROR_NORMAL,
                 SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE | SERVICE_ACCEPT_PAUSE_CONTINUE,
				 true)
	  , m_hStopEvent(NULL) {}
 private:
	bool OnInit(DWORD argc, TCHAR* argv[])override;
    void OnStart() override;
    void OnStop() override;
    void OnSessionChange(DWORD evtType,
                        WTSSESSION_NOTIFICATION* notification) override;
	void OnDeviceChange(DWORD evtType,
						DEV_BROADCAST_HDR* devData)override;
   void OnCustomHandle(DWORD eventId) override;
private:
	BOOL DoRegisterDeviceInterface(
		IN GUID InterfaceClassGuid,
		IN HANDLE hSev,
		OUT HDEVNOTIFY *hDeviceNotify
	);
	BOOL DoUnregisterDeviceNotification(
		HDEVNOTIFY Handle
	);
 private:
#ifdef UNICODE
   using tofstream = std::wofstream;
#else
   using tofstream = std::ofstream;
#endif

   tofstream m_logFile;

   HANDLE m_hStopEvent;
   HDEVNOTIFY m_hDeviceNotify;
};

#endif // USER_TRACKER_SERVICE_H_
