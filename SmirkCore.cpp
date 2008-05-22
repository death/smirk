#include "Stdafx.h"
#include "SmirkCore.h"
#include "NotifyIcon.h"
#include "EnumChildProcesses.h"
#include "Options.h"
#include "Util.h"

// Private functions
BOOL SafeTerminateProcess(HANDLE hProcess, UINT uExitCode);

// Smirk window procedure
LRESULT CALLBACK SmirkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nid;
    POINT pt;
    LPCREATESTRUCT lpcs;
    PDESKINFO pdi = reinterpret_cast<PDESKINFO>(GetWindowLong(hwnd, GWL_USERDATA));

    // Check message
    switch (uMsg) {
        case WM_CREATE:
            // Window created
            lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pdi = reinterpret_cast<PDESKINFO>(lpcs->lpCreateParams);

            // Set window data
            SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(pdi));

            // Build popup menu
            BuildPopupMenu(pdi->nIndex, pdi->hMenu, pdi->hSettingsMenu);
            pdi->hwnd = hwnd;

            if (pdi->nIndex == 0 || pdi->bOpened == true) {
                // Register notify icon
                RegisterNotifyIcon(pdi);
            }

            if (pdi->nIndex == 0) {
                // Default desktop, register hotkeys
                for (size_t i = 0; i < g_vDesktops.size(); i++) {
                    RegisterHotKey(hwnd, ID_HOTKEY + static_cast<int>(i), MOD_CONTROL | MOD_ALT, VK_F1 + static_cast<int>(i));
                }
            }

            return(0);
        case WM_HOTKEY:
            // Hotkey pressed
            if (wParam >= ID_HOTKEY && wParam < (WPARAM )(ID_HOTKEY + cNumDesktops)) {
                pdi = g_vDesktops[static_cast<int>(wParam) - ID_HOTKEY];
                SetCurrentDesktop(pdi->nIndex);
            }
            return(0);
        case WM_CHANGEDDESKTOP:
            // User has changed to current thread's desktop
            g_pLog->Log(_T("Changed to %s"), pdi->szName);
            return(0);
        case WM_DESTROY:
            // Window destroyed
            if (pdi->nIndex) {
                if (g_bCloseChildren == true) {
                    // Close shell's child processes
                    EnumChildProcesses(pdi->dwShellPid, CloseChildProcess);
                }

                if (g_bCloseShells == true) {
                    // Close the shell
                    
                    if (pdi->dwShellPid) {
                        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pdi->dwShellPid);
                        if (hProcess) {
                            SafeTerminateProcess(hProcess, 1);
                            CloseHandle(hProcess);
                        }
                    }
                    
                }
            }

            PostQuitMessage(0);
            return(0);
        case WM_COMMAND:
            // Window command
            switch (LOWORD(wParam)) {
                case IDM_EXIT:
                    // Exit
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case IDM_ABOUT:
                    // About
                    MessageBox(hwnd, _T("Smirk ") VERSION _T(" by DEATH of Execution"), _T("Smirk"), MB_OK | MB_ICONINFORMATION);
                    break;
                case IDM_CHANGEDESC:
                    // Change desktop description
                    if (pdi->hwndChangeDescriptionDialog == NULL) {
                        ChangeDesktopDescription(pdi);
                    } else {
                        SetForegroundWindow(pdi->hwndChangeDescriptionDialog);
                        SetFocus(pdi->hwndChangeDescriptionDialog);
                    }
                    break;
                case IDM_OPTIONS:
                    // Set Smirk options
                    if (pdi->hwndOptionsDialog == NULL) {
                        ShowOptionsDialog(pdi);
                    } else {
                        SetForegroundWindow(pdi->hwndOptionsDialog);
                        SetFocus(pdi->hwndOptionsDialog);
                    }
                    break;
                default:
                    if (LOWORD(wParam) >= IDM_DESKTOP) {
                        if (LOWORD(wParam) < (IDM_DESKTOP + cNumDesktops)) {
                            // Desktop X
                            SetCurrentDesktop(LOWORD(wParam) - IDM_DESKTOP);
                        }
                    }
                    break;
            }
            return(0);
        case WM_NICON:
            // Notification icon
            switch (lParam) {
                case WM_RBUTTONDOWN:
                    // Right button down
                    if (pdi->hMenu) {
                        SetForegroundWindow(hwnd);
                        GetCursorPos(&pt);
                        TrackPopupMenu(pdi->hMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
                    }
                    break;
                default:
                    break;
            }
            return(0);
        case WM_UPDATEPOPUPMENU:
            // Update popup menu
            {
                if (pdi->hMenu) {
                    DestroyMenu(pdi->hMenu);
                    pdi->hMenu = NULL;
                }
                
                BuildPopupMenu(pdi->nIndex, pdi->hMenu, pdi->hSettingsMenu);
        
                // Modify notify icon tip
                TCHAR szTip[128];
                wsprintf(szTip, _T("Smirk - %s"), pdi->szDescription);
                ModifyNotifyIconTip(hwnd, szTip);
            }
            return(0);
        case WM_CLOSE:
            // Request to close window

            // Switch to default desktop so destroy functions will succeed        
            if (pdi->nIndex) {
                SetCurrentDesktop(0);
            } else {
                // Unregister hot key
                for (size_t i = 0; i < g_vDesktops.size(); i++) {
                    UnregisterHotKey(hwnd, ID_HOTKEY + static_cast<int>(i));
                }
            }

            // Remove notify icon
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uID = WM_NICON;
            Shell_NotifyIcon(NIM_DELETE, &nid);

            // Delete menu
            if (pdi->hMenu) {
                DestroyMenu(pdi->hMenu);
                pdi->hMenu = NULL;
            }

            pdi->hwnd = NULL;

            DestroyWindow(hwnd);
            return(0);
        default:
            if (uMsg == g_uTaskbarCreated) {
                // Taskbar was created
                RegisterNotifyIcon(pdi);
            }
            break;
    }

    // Default handler
    return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}

// Create a Smirk window
HWND CreateSmirkWindow(PDESKINFO pdi)
{
    HWND hwnd = CreateWindow(_T("Smirk"), _T("Smirk window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInstance, (LPVOID )pdi);

    return(hwnd);
}

// Enter window message loop
int EnterMessageLoop(HWND hwnd)
{
    MSG msg;
    BOOL bRet;
    
    while ((bRet = GetMessage(&msg, hwnd, 0, 0)) != 0) {
        if (bRet == -1) {
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return(static_cast<int>(msg.wParam));
}

// Build the popup menu
void BuildPopupMenu(int nMarked, HMENU & hMenu, HMENU & hSettingsMenu)
{
    hSettingsMenu = CreatePopupMenu();

    if (hSettingsMenu) {
        AppendMenu(hSettingsMenu, MF_ENABLED, IDM_CHANGEDESC, _T("Change Description"));
        AppendMenu(hSettingsMenu, MF_ENABLED, IDM_OPTIONS, _T("Options"));
    }

    hMenu = CreatePopupMenu();
    
    if (hMenu) {
        // Add desktop items
        for (int i = 0; i < cNumDesktops; i++) {
            PDESKINFO pdi = g_vDesktops[i];

            if (pdi) {
                if (nMarked == i)
                    AppendMenu(hMenu, MF_ENABLED | MF_CHECKED, IDM_DESKTOP + i, pdi->szDescription);
                else
                    AppendMenu(hMenu, MF_ENABLED, IDM_DESKTOP + i, pdi->szDescription);
            }
        }

        // Add other items
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

        if (hSettingsMenu)
            AppendMenu(hMenu, MF_ENABLED | MF_POPUP, reinterpret_cast<UINT_PTR>(hSettingsMenu), _T("Settings"));

        AppendMenu(hMenu, MF_ENABLED, IDM_ABOUT, _T("About"));
        AppendMenu(hMenu, MF_ENABLED, IDM_EXIT, _T("Exit"));
    }
}

// Set current desktop
void SetCurrentDesktop(int nIndex)
{
    PDESKINFO pdi = g_vDesktops[nIndex];
    if (pdi) {
        if (pdi->hDesktop) {
            // Switch to desktop and notify desktop's window
            SwitchDesktop(pdi->hDesktop);
            if (pdi->hwnd) {
                g_pLog->Log(_T("Sending notification message to %s"), pdi->szName);
                PostMessage(pdi->hwnd, WM_CHANGEDDESKTOP, NULL, NULL);
            }
        }
    }
}

// Desktop thread function
void DesktopThread(LPVOID pData)
{
    PDESKINFO pdi = reinterpret_cast<PDESKINFO>(pData);

    // Set current thread's desktop
    SetThreadDesktop(pdi->hDesktop);

    // If close shells option was set, treat desktop as created
    if ((pdi->nIndex != 0) && (g_bCloseShells == true))
        pdi->bOpened = false;
        
    // Run explorer if desktop created
    if (pdi->bOpened == false)
        RunExplorer(pdi);

    // Create notify icon and enter message loop
    HWND hwnd = CreateSmirkWindow(pdi);
    EnterMessageLoop(hwnd);

    if (pdi->nIndex) {
        // Pass to default desktop
        SetThreadDesktop(g_pdiDefault->hDesktop);
        CloseDesktop(pdi->hDesktop);
        pdi->hDesktop = NULL;

        // Close main notify icon instance
        if (g_pdiDefault->hwnd) {
            PostMessage(g_pdiDefault->hwnd, WM_CLOSE, NULL, NULL);
        }
    } else {
        // Destroy all windows and explorer instances
        g_pLog->Log(_T("%s - Destroying all other threads"), pdi->szName);

        for (size_t i = 1; i < g_vDesktops.size(); i++) {
            pdi = g_vDesktops[i];
            if (pdi) {
                // Close window
                if (pdi->hwnd) {
                    SendMessage(pdi->hwnd, WM_CLOSE, NULL, NULL);
                }
            }
        }
    }
}

// Create a desktop thread
void CreateDesktopThread(PDESKINFO pdi)
{
    // Open/create the desktop
    pdi->hDesktop = OpenDesktop(pdi->szName, 0, FALSE, GENERIC_ALL);
    if (pdi->hDesktop == NULL) {
        pdi->hDesktop = CreateDesktop(pdi->szName, NULL, NULL, 0, MAXIMUM_ALLOWED, NULL);
    } else {
        pdi->bOpened = true;
    }

    // Create desktop's thread
    if (pdi->hDesktop) {
        DWORD dwTid;
        CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )DesktopThread, (LPVOID )pdi, 0, &dwTid));
    }
}

// Close child process
BOOL CALLBACK CloseChildProcess(DWORD dwPid)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (hProcess) {
        SafeTerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
    return(TRUE);
}

BOOL SafeTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    DWORD dwTID, dwCode, dwErr = 0;
    HANDLE hProcessDup = INVALID_HANDLE_VALUE;
    HANDLE hRT = NULL;
    HINSTANCE hKernel = GetModuleHandle(_T("Kernel32"));
    BOOL bSuccess = FALSE;

    BOOL bDup = DuplicateHandle(GetCurrentProcess(), 
                                hProcess, 
                                GetCurrentProcess(), 
                                &hProcessDup, 
                                PROCESS_ALL_ACCESS, 
                                FALSE, 
                                0);

    // Detect the special case where the process is 
    // already dead...
    if ( GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && 
         (dwCode == STILL_ACTIVE) ) 
    {
        FARPROC pfnExitProc;
           
        pfnExitProc = GetProcAddress(hKernel, "ExitProcess");

        hRT = CreateRemoteThread((bDup) ? hProcessDup : hProcess, 
                                 NULL, 
                                 0, 
                                 (LPTHREAD_START_ROUTINE)pfnExitProc,
                                 (PVOID)uExitCode, 0, &dwTID);

        if ( hRT == NULL )
            dwErr = GetLastError();
    }
    else
    {
        dwErr = ERROR_PROCESS_ABORTED;
    }


    if ( hRT )
    {
        // Must wait process to terminate to 
        // guarantee that it has exited...
        WaitForSingleObject((bDup) ? hProcessDup : hProcess, 
                            INFINITE);

        CloseHandle(hRT);
        bSuccess = TRUE;
    }

    if ( bDup )
        CloseHandle(hProcessDup);

    if ( !bSuccess )
        SetLastError(dwErr);

    return bSuccess;
}
