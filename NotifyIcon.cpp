#include "stdafx.h"
#include "Resource.h"
#include "NotifyIcon.h"

// Add an icon to shell notification area
void RegisterNotifyIcon(PDESKINFO pdi)
{
    if (pdi->bRegisteredNotifyIcon == false) {
        // Add notify icon
        NOTIFYICONDATA nid;
    
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SMIRK));
        nid.hWnd = pdi->hwnd;
        wsprintf(nid.szTip, _T("Smirk - %s"), pdi->szDescription);
        nid.uID = WM_NICON;
        nid.uCallbackMessage = WM_NICON;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        Shell_NotifyIcon(NIM_ADD, &nid);

        pdi->bRegisteredNotifyIcon = true;
    }
}

// Modify notify icon tip
void ModifyNotifyIconTip(HWND hwnd, LPCTSTR pszTip)
{
    NOTIFYICONDATA nid;
    
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uID = WM_NICON;
    nid.hWnd = hwnd;
    lstrcpy(nid.szTip, pszTip);
    nid.uFlags = NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}
