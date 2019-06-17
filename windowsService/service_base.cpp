#include "service_base.h"
#include <cassert>

ServiceBase* ServiceBase::m_service = nullptr;

ServiceBase::ServiceBase(const CString& name,
                         const CString& displayName,
                         DWORD dwStartType,
                         DWORD dwErrCtrlType,
                         DWORD dwAcceptedCmds,
						 bool  bHangUpMoude,
                         const CString& depends,
                         const CString& account,
                         const CString& password)
 : m_name(name),
   m_displayName(displayName),
   m_dwStartType(dwStartType),
   m_dwErrorCtrlType(dwErrCtrlType),
   m_dwAcceptedCmds(dwAcceptedCmds),
   m_IsHandUpMode(bHangUpMoude),
   m_depends(depends),
   m_account(account),
   m_password(password),
   m_svcStatusHandle(nullptr) {  

  m_svcStatus.dwControlsAccepted = dwAcceptedCmds;
  m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_svcStatus.dwCurrentState = SERVICE_START_PENDING;
  m_svcStatus.dwWin32ExitCode = NO_ERROR;
  m_svcStatus.dwServiceSpecificExitCode = 0;
  m_svcStatus.dwCheckPoint = 0;
  m_svcStatus.dwWaitHint = 0;
}

void ServiceBase::SetStatus(DWORD dwState, DWORD dwErrCode, DWORD dwWait, DWORD dwSpecificExitCode) {

	static DWORD dwCheckPoint = 1;

	if (dwState == SERVICE_START_PENDING)
		m_svcStatus.dwControlsAccepted = 0;
	else
		m_svcStatus.dwControlsAccepted = m_dwAcceptedCmds;

	if (dwState == SERVICE_RUNNING || dwState == SERVICE_STOPPED)
		m_svcStatus.dwCheckPoint = 0;
	else
		m_svcStatus.dwCheckPoint = dwCheckPoint++;

	if (dwErrCode == ERROR_SERVICE_SPECIFIC_ERROR)
		m_svcStatus.dwServiceSpecificExitCode = dwSpecificExitCode;

  m_svcStatus.dwCurrentState = dwState;
  m_svcStatus.dwWin32ExitCode = dwErrCode;
  m_svcStatus.dwWaitHint = dwWait;

  ::SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

void ServiceBase::WriteToEventLog(const CString& msg, WORD type) {
  HANDLE hSource = RegisterEventSource(nullptr, m_name);
  if (hSource) {
    const TCHAR* msgData[2] = {m_name, msg};
    ReportEvent(hSource, type, 0, 0, nullptr, 2, 0, msgData, nullptr);
    DeregisterEventSource(hSource);
  }
}

// static
void WINAPI ServiceBase::SvcMain(DWORD argc, TCHAR* argv[]) {
  assert(m_service);

  Sleep(10 * 1000);
  m_service->m_svcStatusHandle = ::RegisterServiceCtrlHandlerEx(m_service->GetName(),
                                                                ServiceCtrlHandler, NULL);
  if (!m_service->m_svcStatusHandle) {
    m_service->WriteToEventLog(_T("Can't set service control handler"),
                               EVENTLOG_ERROR_TYPE);
    return;
  }
  if (m_service->Init(argc, argv) == false)
  {
	  m_service->SetStatus(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0, -1);
	  return;
  }

  m_service->Start();
}

// static
DWORD WINAPI ServiceBase::ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType,
                                             void* evtData, void* /*context*/) {
  switch (ctrlCode) {
    case SERVICE_CONTROL_STOP:
      m_service->Stop();
    break;

    case SERVICE_CONTROL_PAUSE:
      m_service->Pause();
    break;

    case SERVICE_CONTROL_CONTINUE:
      m_service->Continue();
    break;

    case SERVICE_CONTROL_SHUTDOWN:
      m_service->Shutdown();
    break;

    case SERVICE_CONTROL_SESSIONCHANGE:
      m_service->OnSessionChange(evtType, reinterpret_cast<WTSSESSION_NOTIFICATION*>(evtData));
    break;

	case SERVICE_CONTROL_DEVICEEVENT:
		m_service->OnDeviceChange(evtType, reinterpret_cast<DEV_BROADCAST_HDR*>(evtData));
	break;

	case SERVICE_CONTROL_INTERROGATE:
		m_service->SetStatus(m_service->m_svcStatus.dwCurrentState);
	break;

    default:
		if (128 <= ctrlCode && 255 >= ctrlCode)
		{
			m_service->OnCustomHandle(ctrlCode);
		}
    break;
  }

  return 0;
}

bool ServiceBase::RunInternal(ServiceBase* svc) {
  m_service = svc;

  TCHAR* svcName = const_cast<CString&>(m_service->GetName()).GetBuffer();

  SERVICE_TABLE_ENTRY tableEntry[] = {
    {svcName, SvcMain},
    {nullptr, nullptr}
  };

  return ::StartServiceCtrlDispatcher(tableEntry) == TRUE;
}

bool ServiceBase::Init(DWORD argc, TCHAR* argv[])
{
	SetStatus(SERVICE_START_PENDING, 0, 3000);
	if (OnInit(argc, argv) == false)
	{
		SetStatus(SERVICE_STOP_PENDING);
		return false;
	}
	SetStatus(SERVICE_RUNNING);
	return true;
}

void ServiceBase::Start() {
	OnStart();
    if(m_IsHandUpMode)
		SetStatus(SERVICE_STOPPED);
	else
		SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Stop() {
  SetStatus(SERVICE_STOP_PENDING);
  OnStop();
  if (m_IsHandUpMode == false)
	  SetStatus(SERVICE_STOPPED);
}

void ServiceBase::Pause() {
  SetStatus(SERVICE_PAUSE_PENDING);
  OnPause();
  SetStatus(SERVICE_PAUSED);
}

void ServiceBase::Continue() {
  SetStatus(SERVICE_CONTINUE_PENDING);
  OnContinue();
  SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Shutdown() {
  OnShutdown();
  SetStatus(SERVICE_STOPPED);
}