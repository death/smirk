#include "stdafx.h"
#include "Util.h"

// Run explorer shell
void RunExplorer(PDESKINFO pdi)
{
    // Log run
    g_pLog->Log(_T("RunExplorer() - %s"), pdi->szName);

    // Set inheritance to false
    USEROBJECTFLAGS uof;
    
    uof.fInherit = FALSE;
    uof.fReserved = FALSE;
    uof.dwFlags = DF_ALLOWOTHERACCOUNTHOOK;

    SetUserObjectInformation(pdi->hDesktop, UOI_FLAGS, (PVOID )&uof, sizeof(USEROBJECTFLAGS));

    // Create explorer process
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = pdi->szName;

    if (CreateProcess(NULL, g_szShell, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SEPARATE_WOW_VDM, NULL, NULL, &si, &pi) == 0) {
        MessageBox(NULL, _T("Could not invoke explorer"), NULL, MB_OK);
    } else {
        // Wait until process has finished initialization
        // WaitForInputIdle(pi.hProcess, INFINITE);

        pdi->dwShellPid = pi.dwProcessId;
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

// Detect other Smirk instances
bool IsSmirkRunning(void)
{
    bool bRunning = false;

    if (FindWindow(_T("Smirk"), _T("Smirk Window")) != NULL) 
        bRunning = true;

    return(bRunning);
}

// Get shell executable file name
void GetShellExecutable(void)
{
    HKEY hkey;
    DWORD dwSize;
    bool bSuccess = false;

    // Try reading shell name from current user class
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
        dwSize = sizeof(g_szShell) / sizeof(TCHAR);
        if (RegQueryValueEx(hkey, _T("shell"), NULL, NULL, (LPBYTE )g_szShell, &dwSize) == ERROR_SUCCESS) {
            bSuccess = true;
        }
        RegCloseKey(hkey);
    }

    if (bSuccess == true) return;

    // Try reading shell name from local machine class
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
        dwSize = sizeof(g_szShell) / sizeof(TCHAR);
        if (RegQueryValueEx(hkey, _T("shell"), NULL, NULL, (LPBYTE )g_szShell, &dwSize) == ERROR_SUCCESS) {
            bSuccess = true;
        }
        RegCloseKey(hkey);
    }

    if (bSuccess == true) return;

    // Assign default shell name
    lstrcpy(g_szShell, _T("explorer"));
}
