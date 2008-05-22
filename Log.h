#pragma once

class CLog
{
public:
    CLog(LPCTSTR pszFileName, bool bActive = true);
    ~CLog(void);

    void Log(LPCTSTR pFormat, ...);

private:
    FILE *m_pFile;
};
