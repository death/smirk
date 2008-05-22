#pragma once

#pragma warning(disable:4311)
#pragma warning(disable:4312)

#include "Log.h"

// Global definitions
#define VERSION         _T("v1.2")
#define IDM_EXIT        100
#define IDM_ABOUT       101
#define IDM_CHANGEDESC  102
#define IDM_OPTIONS     103
#define IDM_DESKTOP     104
#define WM_NICON        (WM_USER + 100)
#define WM_CHANGEDDESKTOP (WM_USER + 200)
#define WM_UPDATEPOPUPMENU (WM_USER + 300)
#define DN_SIZE         32
#define DD_SIZE         64
#define ID_HOTKEY       1

// Constants
const int cNumDesktops = 4;

// Structure definitions
typedef struct DESKINFO_TAG {
    HDESK hDesktop;
    int nIndex;
    TCHAR szName[DN_SIZE];
    TCHAR szDescription[DD_SIZE];
    bool bOpened;
    
    HMENU hMenu;
    HMENU hSettingsMenu;

    HWND hwnd;
    bool bRegisteredNotifyIcon;
    
    HWND hwndChangeDescriptionDialog;
    HWND hwndOptionsDialog;

    DWORD dwShellPid;
} DESKINFO, *PDESKINFO;

// Desktop information vector type definition
typedef std::vector<PDESKINFO> pdiVector;

// Global variables
extern HINSTANCE g_hInstance;
extern PDESKINFO g_pdiDefault;
extern pdiVector g_vDesktops;
extern CLog *g_pLog;
extern TCHAR g_szShell[256];
extern UINT g_uTaskbarCreated;

extern bool g_bCloseShells;
extern bool g_bCloseChildren;
