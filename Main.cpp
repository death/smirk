// Includes
#include "stdafx.h"
#include "Resource.h"
#include "Defs.h"
#include "Util.h"
#include "SmirkCore.h"
#include "Options.h"
#include "DesktopDescriptionManager.h"

using namespace std;

// Global variables
HINSTANCE g_hInstance;
PDESKINFO g_pdiDefault;
pdiVector g_vDesktops;
CLog *g_pLog;
TCHAR g_szShell[256];
UINT g_uTaskbarCreated;

bool g_bCloseShells;
bool g_bCloseChildren;

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    const TCHAR szAppName[] = _T("Smirk");

    // Assign instance to global variable
    g_hInstance = hInstance;

    // Initialize logger (required for programme run)
    g_pLog = new CLog(_T("smirk.log"), false);
    if (g_pLog == NULL) 
        return(0);
    g_pLog->Log(_T("----------------------------------------------------------------------"));

    // Only one instance of Smirk should be running
    if (IsSmirkRunning() == true) {
        MessageBox(NULL, _T("Smirk is already running"), _T("Smirk"), MB_OK | MB_ICONINFORMATION);
        return(0);
    }

    // Initialize window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMIRK));
    wc.hInstance = hInstance;
    wc.lpszClassName = szAppName;
    wc.lpszMenuName = NULL;
    wc.lpfnWndProc = SmirkWndProc;

    // Register window class
    if (RegisterClass(&wc) == 0)
        return(0);

    // Get shell executable
    GetShellExecutable();

    // Get Taskbar creation notification message
    g_uTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

    // Get settings from registry
    g_bCloseShells = GetBooleanSetting(_T("CloseShells"), true);
    g_bCloseChildren = GetBooleanSetting(_T("CloseChildren"), true);

    // Allocate default desktop
    g_pdiDefault = new DESKINFO;
    g_vDesktops.push_back(g_pdiDefault);

    // Construct a desktop description manager
    CDesktopDescriptionManager ddm;

    // Create default desktop information
    g_pdiDefault->hDesktop = GetThreadDesktop(GetCurrentThreadId());
    g_pdiDefault->nIndex = 0;
    g_pdiDefault->bOpened = true;
    g_pdiDefault->bRegisteredNotifyIcon = false;
    g_pdiDefault->hwndChangeDescriptionDialog = NULL;
    g_pdiDefault->hwndOptionsDialog = NULL;
    g_pdiDefault->dwShellPid = 0;
    DWORD dwLength = DN_SIZE;
    GetUserObjectInformation(g_pdiDefault->hDesktop, UOI_NAME, g_pdiDefault->szName, DN_SIZE, &dwLength);
    
    if (ddm.GetDescription(g_pdiDefault->szName, g_pdiDefault->szDescription, DD_SIZE * sizeof(TCHAR)) == false) {
        wsprintf(g_pdiDefault->szDescription, _T("Desktop 0"));
    }

    // Create desktop threads
    for (int i = 1; i < cNumDesktops; i++) {
        PDESKINFO pdi = new DESKINFO;
        pdi->hDesktop = NULL;
        pdi->nIndex = i;
        pdi->bOpened = false;
        pdi->bRegisteredNotifyIcon = false;
        pdi->hwndChangeDescriptionDialog = NULL;
        pdi->hwndOptionsDialog = NULL;
        pdi->dwShellPid = 0;

        wsprintf(pdi->szName, _T("%s%d"), szAppName, i);

        // Load desktop description
        if (ddm.GetDescription(pdi->szName, pdi->szDescription, DD_SIZE * sizeof(TCHAR)) == false) {
            // Assign default description
            wsprintf(pdi->szDescription, _T("Desktop %d"), i);
        }

        g_vDesktops.push_back(pdi);
    }

    // Close manager
    ddm.Close();

    // Create desktop threads
    for (int i = 1; i < cNumDesktops; i++) {
        PDESKINFO pdi = g_vDesktops[i];
        if (pdi) {
            CreateDesktopThread(pdi);
        }
    }

    // Turn into a desktop thread
    DesktopThread(reinterpret_cast<LPVOID>(g_pdiDefault));

    // Open description manager
    ddm.Open();

    // Save desktop descriptions and remove all desktop information structures
    for (i = 0; i < static_cast<int>(g_vDesktops.size()); i++) {
        PDESKINFO pdi = g_vDesktops[i];
        if (pdi) {
            ddm.SetDescription(pdi->szName, pdi->szDescription);
            delete pdi;
        }
    }

    g_vDesktops.clear();

    // Close desktop manager
    ddm.Close();

    // Save settings
    SetBooleanSetting(_T("CloseShells"), g_bCloseShells);
    SetBooleanSetting(_T("CloseChildren"), g_bCloseChildren);

    // Deinitialize stuff
    UnregisterClass(szAppName, hInstance);
    delete g_pLog;

    return(0);
}
