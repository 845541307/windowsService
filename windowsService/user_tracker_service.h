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
                 SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE,
				 true)
	  , m_hStopEvent(NULL) {}
 private:
	bool OnInit(DWORD argc, TCHAR* argv[])override;
    void OnStart() override;
    void OnStop() override;
    void OnSessionChange(DWORD evtType,
                        WTSSESSION_NOTIFICATION* notification) override;
   void OnCustomHandle(DWORD eventId) override;

#ifdef UNICODE
   using tofstream = std::wofstream;
#else
   using tofstream = std::ofstream;
#endif

   tofstream m_logFile;

   HANDLE m_hStopEvent;
};

#endif // USER_TRACKER_SERVICE_H_
