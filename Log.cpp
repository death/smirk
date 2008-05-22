#include "StdAfx.h"
#include "log.h"

CLog::CLog(LPCTSTR pszFileName, bool bActive)
: m_pFile(NULL)
{
    if (bActive == true)
        m_pFile = _tfopen(pszFileName, _T("at"));
}

CLog::~CLog(void)
{
    if (m_pFile) {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

void CLog::Log(LPCTSTR pFormat, ...)
{
    if (m_pFile) {
        va_list ap;

        va_start(ap, pFormat);
        _vftprintf(m_pFile, pFormat, ap);
        va_end(ap);

        _ftprintf(m_pFile, _T("\n"));
    }
}
