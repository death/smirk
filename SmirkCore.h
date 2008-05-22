#pragma once

#include "Defs.h"

LRESULT CALLBACK SmirkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateSmirkWindow(PDESKINFO pdi);
int EnterMessageLoop(HWND hwnd);
void BuildPopupMenu(int nMarked, HMENU & hMenu, HMENU & hSettingsMenu);
void SetCurrentDesktop(int nIndex);
void DesktopThread(LPVOID pData);
void CreateDesktopThread(PDESKINFO pdi);
BOOL CALLBACK CloseChildProcess(DWORD dwPid);
