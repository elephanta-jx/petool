// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <CommCtrl.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <windowsx.h>
#include <Winternl.h>
#include <psapi.h>
#include <tlhelp32.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <ctype.h>

// C++ Header Files
#include <iostream>
#include <fstream>
#include <string>
#include <exception>


using namespace std;

#include "MemoryLeak.h"

#define SUCCEED (TEXT("Succeed"))
#define INVALID_PE (TEXT("Invalid PE file"))
#define INVALID_ARGUMENT (TEXT("Invalid Arguments"))
#define INVALID_DLLNAME (TEXT("Invalid Dll Name"))
#define INVALID_OPEN_PROCESS (TEXT("Can't open the target process"))
#define INVALID_READ_MEMORY (TEXT("Can't read the memory"))
#define INVALID_WRITE_MEMORY (TEXT("Can't write the memory"))
#define INVALID_REMOTE_THREAD (TEXT("Failure in creating remote thread"))
#define INVALID_COMPATIBILITY (TEXT("Dll and target process are not compatibale!"))
#define COMSEC_INDEXERROR (TEXT("At least one of the selected sections is invalid!"))
#define FAIL_ADDSECTION (TEXT("Fail to add the section"))
