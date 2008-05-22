#include "stdafx.h"
#include "Resource.h"
#include "Options.h"

// Change desktop description
void ChangeDesktopDescription(PDESKINFO pdi)
{
    switch (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CHANGEDESC), pdi->hwnd, DlgChangeDescription, reinterpret_cast<LPARAM>(pdi))) {
        case IDOK:
            // Update menus
            for (int i = 0; i < cNumDesktops; i++) {
                SendMessage(g_vDesktops[i]->hwnd, WM_UPDATEPOPUPMENU, 0, 0);
            }
            break;
        default:
            break;
    }
    pdi->hwndChangeDescriptionDialog = NULL;
}

// Change desktop description dialog procedure
BOOL CALLBACK DlgChangeDescription(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PDESKINFO pdi = reinterpret_cast<PDESKINFO>(GetWindowLong(hDlg, GWL_USERDATA));

    switch (uMsg) {
        case WM_INITDIALOG:
            // Initialize dialog
            SetWindowLong(hDlg, GWL_USERDATA, (LONG )lParam);
            pdi = reinterpret_cast<PDESKINFO>(GetWindowLong(hDlg, GWL_USERDATA));
            pdi->hwndChangeDescriptionDialog = hDlg;
            SetDlgItemText(hDlg, IDT_DESCRIPTION, pdi->szDescription);
            SendDlgItemMessage(hDlg, IDT_DESCRIPTION, EM_SETSEL, 0, -1);
            SetFocus(GetDlgItem(hDlg, IDT_DESCRIPTION));
            return(FALSE);
        case WM_COMMAND:
            // Dialog command
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case IDOK:
                        // OK
                        {
                            int nLength = GetWindowTextLength(GetDlgItem(hDlg, IDT_DESCRIPTION));
                            GetDlgItemText(hDlg, IDT_DESCRIPTION, pdi->szDescription, min(nLength + 1, DD_SIZE));
                            EndDialog(hDlg, IDOK);
                        }
                        return(TRUE);
                    case IDCANCEL:
                        // Cancel
                        EndDialog(hDlg, IDCANCEL);
                        return(TRUE);
                    default:
                        break;
                }
            } else if (HIWORD(wParam) == EN_CHANGE) {
                // Edit was changed
                TCHAR szText[DD_SIZE];
                int nLength = GetWindowTextLength(GetDlgItem(hDlg, IDT_DESCRIPTION));
                GetDlgItemText(hDlg, IDT_DESCRIPTION, szText, min(nLength + 1, DD_SIZE));

                // If edit is empty, disable OK button
                if (szText[0] == '\0') {
                    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                } else {
                    EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                }
                return(TRUE);
            }
            break;
        case WM_SETFOCUS:
            // Focus was set to this window
            SetFocus(GetDlgItem(hDlg, IDT_DESCRIPTION));
            return(TRUE);
        case WM_CLOSE:
            // Close dialog
            EndDialog(hDlg, IDCANCEL);
            return(TRUE);
        default:
            break;
    }

    return(FALSE);
}

// Show options dialog
void ShowOptionsDialog(PDESKINFO pdi)
{
    switch (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), pdi->hwnd, DlgOptions, reinterpret_cast<LPARAM>(pdi))) {
        case IDOK:
            // Update options
            break;
        default:
            break;
    }
    pdi->hwndOptionsDialog = NULL;
}

// Options dialog procedure
BOOL CALLBACK DlgOptions(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PDESKINFO pdi = reinterpret_cast<PDESKINFO>(GetWindowLong(hDlg, GWL_USERDATA));

    switch (uMsg) {
        case WM_INITDIALOG:
            // Initialize dialog
            SetWindowLong(hDlg, GWL_USERDATA, (LONG )lParam);
            pdi = reinterpret_cast<PDESKINFO>(GetWindowLong(hDlg, GWL_USERDATA));
            pdi->hwndOptionsDialog = hDlg;

            // Set values
            if (g_bCloseShells == true)
                CheckDlgButton(hDlg, IDC_CLOSESHELLS, BST_CHECKED);
            if (g_bCloseChildren == true)
                CheckDlgButton(hDlg, IDC_CLOSECHILDREN, BST_CHECKED);

            break;
        case WM_COMMAND:
            // Dialog command
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case IDOK:
                        // OK
                        {
                            if (IsDlgButtonChecked(hDlg, IDC_CLOSESHELLS) == BST_CHECKED) {
                                g_bCloseShells = true;
                            } else {
                                g_bCloseShells = false;
                            }

                            if (IsDlgButtonChecked(hDlg, IDC_CLOSECHILDREN) == BST_CHECKED) {
                                g_bCloseChildren = true;
                            } else {
                                g_bCloseChildren = false;
                            }

                            EndDialog(hDlg, IDOK);
                        }
                        break;
                    case IDCANCEL:
                        // Cancel
                        EndDialog(hDlg, IDCANCEL);
                        break;
                    default:
                        break;
                }
            }
            break;
        case WM_CLOSE:
            // Close dialog
            EndDialog(hDlg, IDCANCEL);
            break;
        default:
            break;
    }

    return(FALSE);
}

// Get a boolean setting from registry
bool GetBooleanSetting(LPCTSTR pszName, bool bDefault)
{
    HKEY hRegKey;
    bool bRet = bDefault;

    RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Execution\\Smirk"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
    if (hRegKey) {
        DWORD dw;
        DWORD cbdw = sizeof(DWORD);
        RegQueryValueEx(hRegKey, pszName, NULL, NULL, (LPBYTE )&dw, &cbdw);
        (dw) ? bRet = true : bRet = false;
        RegCloseKey(hRegKey);
    }
    
    return(bRet);
}

// Set a boolean settings in registry
void SetBooleanSetting(LPCTSTR pszName, bool bValue)
{
    HKEY hRegKey;

    RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Execution\\Smirk"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hRegKey, NULL);
    if (hRegKey) {
        DWORD dw = (DWORD )bValue;
        RegSetValueEx(hRegKey, pszName, 0, REG_DWORD, reinterpret_cast<BYTE *>(&dw), sizeof(DWORD));
        RegCloseKey(hRegKey);
    }
}