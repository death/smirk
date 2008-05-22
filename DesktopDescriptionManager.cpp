#include "StdAfx.h"
#include "DesktopDescriptionManager.h"

CDesktopDescriptionManager::CDesktopDescriptionManager(void)
: m_hRegKey(0)
{
    Open();
}

CDesktopDescriptionManager::~CDesktopDescriptionManager(void)
{
    Close();
}

bool CDesktopDescriptionManager::GetDescription(LPCTSTR pszName, LPTSTR pszDescription, DWORD cbDescription)
{
    if (m_hRegKey) {
        if (RegQueryValueEx(m_hRegKey, pszName, NULL, NULL, reinterpret_cast<LPBYTE>(pszDescription), &cbDescription) == ERROR_SUCCESS) {
            return(true);
        }
    }

    return(false);
}

bool CDesktopDescriptionManager::SetDescription(LPCTSTR pszName, LPCTSTR pszDescription)
{
    if (m_hRegKey) {
        if (RegSetValueEx(m_hRegKey, pszName, NULL, REG_SZ, reinterpret_cast<const BYTE *>(pszDescription), sizeof(TCHAR) * (lstrlen(pszDescription) + 1)) == ERROR_SUCCESS) {
            return(true);
        }
    }
    return(false);
}

void CDesktopDescriptionManager::Close(void)
{
    if (m_hRegKey) {
        RegCloseKey(m_hRegKey);
        m_hRegKey = 0;
    }
}

void CDesktopDescriptionManager::Open(void)
{
    if (m_hRegKey == 0) {
        RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Execution\\Smirk"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hRegKey, NULL);
    }
}
