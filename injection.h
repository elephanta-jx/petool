#pragma once
#include "framework.h"

HMODULE GetModule(HANDLE hProcess);
int RemoteInjection(DWORD dwPid, PTCHAR pszName, size_t dwNameSize);