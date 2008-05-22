#pragma once

#include "Defs.h"

void ChangeDesktopDescription(PDESKINFO pdi);
BOOL CALLBACK DlgChangeDescription(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowOptionsDialog(PDESKINFO pdi);
BOOL CALLBACK DlgOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool GetBooleanSetting(LPCTSTR pszName, bool bDefault);
void SetBooleanSetting(LPCTSTR pszName, bool bValue);
