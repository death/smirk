#pragma once

typedef BOOL (CALLBACK *ENUMCHILDPROCESSESPROC)(DWORD);

BOOL EnumChildProcesses(DWORD dwParentPid, ENUMCHILDPROCESSESPROC pfnCallback);