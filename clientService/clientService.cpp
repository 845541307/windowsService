// clientService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#define TestEventID		128
#define SVCNAME		TEXT("TestService")
BOOL ControlSampleService(DWORD fdwControl)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;
	DWORD fdwAccess;
	DWORD dwStartTickCount, dwWaitTime;

	// Access
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP:
		fdwAccess = SERVICE_STOP;
		break;
	case SERVICE_CONTROL_PAUSE:
	case SERVICE_CONTROL_CONTINUE:
		fdwAccess = SERVICE_PAUSE_CONTINUE;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		fdwAccess = SERVICE_INTERROGATE;
		break;
	default:
		if (128 <= fdwControl && 255 >= fdwControl)
			fdwAccess = SERVICE_USER_DEFINED_CONTROL;
		else
			fdwAccess = SERVICE_INTERROGATE;
	}


	// 打开 SCM
	schSCManager = OpenSCManager(
		NULL,                    // local machine 
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
		printf("OpenSCManager failed (%d)\n", GetLastError());

	// 打开服务
	schService = OpenService(
		schSCManager,        // SCManager 句柄 
		SVCNAME,		 // 服务名
		fdwAccess);          // 存取权限
	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		return FALSE;
	}

	// 发送控制码
	if (!ControlService(
		schService,   // 服务的句柄
		fdwControl,   // 控制码
		&ssStatus))  // 状态
	{
		printf("ControlService failed (%d)\n", GetLastError());
		return FALSE;
	}

	// 显示状态
	printf("\nStatus of Sample_Srv: \n");
	printf("  Service Type: 0x%x\n", ssStatus.dwServiceType);
	printf("  Current State: 0x%x\n", ssStatus.dwCurrentState);
	printf("  Controls Accepted: 0x%x\n",
		ssStatus.dwControlsAccepted);
	printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
	printf("  Service Specific Exit Code: %d\n",
		ssStatus.dwServiceSpecificExitCode);
	printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
	printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);

	return TRUE;
}
int main()
{
	ControlSampleService(TestEventID);
    std::cout << "Hello World!\n"; 
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
