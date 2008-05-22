#include "stdafx.h"
#include <tlhelp32.h>
#include "EnumChildProcesses.h"

// Enum child processes
BOOL EnumChildProcesses(DWORD dwParentPid, ENUMCHILDPROCESSESPROC pfnCallback)
{
    // Create a snapshot of processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        // Scan through processes
        if (Process32First(hSnapshot, &pe) == TRUE) {
            do {
                // Verify that the parent process ID is the one we want
                if (pe.th32ParentProcessID == dwParentPid) {
                    pfnCallback(pe.th32ProcessID);
                }
            } while (Process32Next(hSnapshot, &pe) == TRUE);
        }
        
        CloseHandle(hSnapshot);
        return(TRUE);
    }

    return(FALSE);
}