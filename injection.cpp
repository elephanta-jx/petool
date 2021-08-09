#include "injection.h"
#include "PEFile.h"

int RemoteInjection(DWORD dwPid, PTCHAR pszName, size_t dwNameSize)
{
	// check parameters
	if (dwPid == 0 || pszName == NULL || dwNameSize == 0)
		return 1;
	
	// check dll file
	PEFile dllFile;
	if (dllFile.readFile(wstring(pszName)) == false)
		return 2;
	
	// open target process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPid);
	if (hProcess == NULL)
		return 3;

	// check compatibility 
	HMODULE hExe = GetModule(hProcess);
	MODULEINFO modinfo;
	GetModuleInformation(hProcess, hExe, &modinfo, sizeof(MODULEINFO));
	PEFile process;
	if (process.openProcess(dwPid, (DWORD64)modinfo.lpBaseOfDll, modinfo.SizeOfImage) == false)
		return 3;
	if (dllFile.getBit() != process.getBit())
		return 7;

	// bit check and assign func address
	DWORD(WINAPI *threadproc) (LPVOID) = NULL;
	if (dllFile.getBit() == 32)
		threadproc = (LPTHREAD_START_ROUTINE)0x75fb9cc0;
	else if (dllFile.getBit() == 64)
		threadproc = (unsigned long(__stdcall*)(void*))LoadLibrary;

	// allocate remote memory
	PCHAR remoteAddr = (PCHAR)VirtualAllocEx(hProcess, NULL, dwNameSize * sizeof(TCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (remoteAddr == NULL)
		return 4;

	// write dll name
	if (WriteProcessMemory(hProcess, remoteAddr, pszName, dwNameSize * sizeof(TCHAR), NULL) == false)
		return 5;

	// create remote thread
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, threadproc, remoteAddr, 0, NULL);
	if (hThread == NULL)
		return 6;

/*	// wait for the thread terminates
	WaitForSingleObject(hThread, -1);
	DWORD retValue = 0;
	GetExitCodeThread(hThread, &retValue);

	// free remote address
	VirtualFreeEx(hProcess, remoteAddr, 0, MEM_RELEASE);
	*/
	CloseHandle(hProcess);
	CloseHandle(hThread);

	return 0;
}

HMODULE GetModule(HANDLE hProcess)
{
	// get exe name
	TCHAR buffer[MAX_PATH];
	if (GetProcessImageFileName(hProcess, buffer, MAX_PATH) == 0)
		return NULL;

	TCHAR* name = buffer;
	for (int slashCount = 0; slashCount < 3; name++)
	{
		if (*name == TEXT('\\'))
			slashCount++;
	}

	for (int i = 0; name[i] != TEXT('\0'); i++)	// to lowercase
	{
		name[i] = _totlower(name[i]);
	}

	// enumarate modules
	DWORD maxSize = 0x100;
	DWORD iNeeded = 0;
	HMODULE* hModule = new HMODULE[maxSize];
	HMODULE target = NULL;

	if (EnumProcessModulesEx(hProcess, hModule, maxSize * sizeof(HMODULE), &iNeeded, LIST_MODULES_ALL) == 0)
		return NULL;

	if (iNeeded > maxSize * sizeof(HMODULE))
	{
		delete[] hModule;
		maxSize = iNeeded / sizeof(HMODULE);
		hModule = new HMODULE[maxSize];
		EnumProcessModulesEx(hProcess, hModule, maxSize * sizeof(HMODULE), &iNeeded, LIST_MODULES_ALL);
	}

	TCHAR modName[MAX_PATH];
	for (int i = 0; i < iNeeded / sizeof(HMODULE); i++)
	{
		GetModuleFileNameEx(hProcess, hModule[i], modName, MAX_PATH);
		for (int i = 0; modName[i] != TEXT('\0'); i++)	// to lowercase
		{
			modName[i] = _totlower(modName[i]);
		}

		if (_tcsstr(modName, name) != NULL)
		{
			target = hModule[i];
			break;
		}

	}

	delete[] hModule;
	return target;
}

void* getLoadLibraryFuncAddr32()
{
	TCHAR version;
	if (sizeof(TCHAR) == 1)
		version = TEXT('a');
	else
		version = TEXT('w');

	TCHAR command[100];
	memset(command, 0, 100);

	_stprintf(command, TEXT("%c "), version);

	HANDLE read, write;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	CreatePipe(&read, &write, &sa, 100);
	_stprintf(command, TEXT("%c %llx"), version, (INT64)write);

	CreateProcess(TEXT("loadlibrary32.exe"), command, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, -1);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return NULL;

}