#pragma once

class CDesktopDescriptionManager
{
public:
    CDesktopDescriptionManager(void);
    ~CDesktopDescriptionManager(void);
    bool GetDescription(LPCTSTR pszName, LPTSTR pszDescription, DWORD cbDescription);
    bool SetDescription(LPCTSTR pszName, LPCTSTR pszDescription);
    void Close(void);
    void Open(void);

private:
    HKEY m_hRegKey;
};
