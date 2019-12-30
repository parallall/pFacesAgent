/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* Modified by Mahmoud Khaled to be complient with modern C/C++
*
* The file implements functions that install and uninstall the service.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/
#include "WindowsServiceInstaller.h"

DWORD InstallService(LPCSTR pszServiceName,
	LPCSTR pszDisplayName,
	DWORD dwStartType,
	LPCSTR pszDependencies,
	LPCSTR pszAccount,
	LPCSTR pszPassword,
	std::ostream& ss_cout)
{
	char szPath[MAX_PATH];
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	DWORD last_error = 0;

	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) == 0){ 
		last_error = GetLastError();
		ss_cout << "InstallService: GetModuleFileName failed w/err " << last_error;
		return last_error;
	}

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL){
		last_error = GetLastError();
		ss_cout << "InstallService: OpenSCManager failed w/err " << last_error;
		return last_error;
	}

	// Install the service into SCM by calling CreateService
	schService = CreateService(
		schSCManager,                   // SCManager database
		pszServiceName,                 // Name of service
		pszDisplayName,                 // Name to display
		SERVICE_QUERY_STATUS,           // Desired access
		SERVICE_WIN32_OWN_PROCESS,      // Service type
		dwStartType,                    // Service start type
		SERVICE_ERROR_NORMAL,           // Error control type
		szPath,                         // Service's binary
		NULL,                           // No load ordering group
		NULL,                           // No tag identifier
		pszDependencies,                // Dependencies
		pszAccount,                     // Service running account
		pszPassword                     // Password of the account
	);
	if (schService == NULL){
		last_error = GetLastError();
		ss_cout << "InstallService: CreateService failed w/err " << last_error;
		
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
		
		return last_error;
	}


	ss_cout << pszServiceName << " is installed.";

	CloseServiceHandle(schSCManager);
	schSCManager = NULL;
	CloseServiceHandle(schService);
	schService = NULL;

	return last_error;
}

DWORD ChangeServiceDescription(LPCSTR pszServiceName, 
	LPCSTR pszDescription,
	std::ostream& ss_cout)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};
	DWORD last_error = 0;

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL) {
		last_error = GetLastError();
		ss_cout << "UninstallService: OpenSCManager failed w/err " << last_error;
		return last_error;
	}

	// Open the service with full permissions
	schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
	if (schService == NULL) {
		last_error = GetLastError();
		ss_cout << "UninstallService: OpenService failed w/err " << last_error;

		CloseServiceHandle(schSCManager);
		schSCManager = NULL;

		return last_error;
	}

	// setting the service description
	SERVICE_DESCRIPTION descrInfo;
	descrInfo.lpDescription = (LPSTR)pszDescription;
	BOOL cngDescrStatus = ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &descrInfo);
	if (cngDescrStatus == FALSE) {
		last_error = GetLastError();
		ss_cout << "Failed to set the service describtion w/err " << last_error;
		return last_error;
	}


	if (last_error == 0)
		ss_cout << pszServiceName << " has now a new description.";

	// Centralized cleanup for all allocated resources.
	CloseServiceHandle(schSCManager);
	schSCManager = NULL;
	CloseServiceHandle(schService);
	schService = NULL;

	return last_error;
}



DWORD UninstallService(LPCSTR pszServiceName, std::ostream& ss_cout)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};
	DWORD last_error = 0;

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL){
		last_error = GetLastError();
		ss_cout << "UninstallService: OpenSCManager failed w/err " << last_error;
		return last_error;
	}

	// Open the service with delete, stop, and query status permissions
	schService = OpenService(schSCManager, pszServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
	if (schService == NULL){
		last_error = GetLastError();
		ss_cout << "UninstallService: OpenService failed w/err " << last_error;
		
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;

		return last_error;
	}

	// Try to stop the service
	if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus)){
		ss_cout << "Stopping " << pszServiceName << " ." << std::flush;
		Sleep(1000);

		while (QueryServiceStatus(schService, &ssSvcStatus)){
			if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING){
				ss_cout << "." << std::flush;
				Sleep(1000);
			}
			else 
				break;
		}
		ss_cout << std::endl;

		if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
			ss_cout << pszServiceName << " is stopped." << std::endl;
		else
			ss_cout << pszServiceName << " failed to stop." << std::endl;
	}

	// Now remove the service by calling DeleteService.
	if (!DeleteService(schService)){
		last_error = GetLastError();
		ss_cout << "UninstallService: DeleteService failed w/err " << last_error;
	}

	if (last_error == 0)
		ss_cout << pszServiceName << " is removed.";

	// Centralized cleanup for all allocated resources.
	CloseServiceHandle(schSCManager);
	schSCManager = NULL;
	CloseServiceHandle(schService);
	schService = NULL;

	return last_error;
}






