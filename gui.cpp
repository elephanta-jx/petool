#include "gui.h"

#pragma comment(lib, "comctl32.lib")

HIMAGELIST g_hImageList = NULL;

HWND gui_AddToolBar(HWND hWndParent)
{
    // local variables

    const int ImageListID = 0;
    const int numButtons = 12;
    const int bitmapSize = 16;

    
    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE, 0, 0, 0, 0,
        hWndParent, NULL, hInst, NULL);

    if (hWndToolbar == NULL)
        return NULL;

    
    // Create the image list.
    g_hImageList = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
        ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
        numButtons, 0);
    
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_OPEN)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_PRO)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SAVE)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_DOS)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_PE)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_OPT)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_SECTION)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_DIR)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_EX)), NULL);
    ImageList_Add(g_hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_IM)), NULL);

    // Set the image list.
    SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ImageListID, (LPARAM)g_hImageList);

    TBBUTTON tbButtons[numButtons] =
    {
        { MAKELONG(0,  ImageListID), IDB_TOOLBAR_OPEN,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(1,  ImageListID), IDB_TOOLBAR_PRO,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(2,  ImageListID), IDB_TOOLBAR_SAVE,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { 0, 0,  TBSTATE_ENABLED, (BYTE)BTNS_SEP, {0}, 0, NULL },
        { MAKELONG(3,  ImageListID), IDB_TOOLBAR_DOS,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(4,  ImageListID), IDB_TOOLBAR_PE,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(5,  ImageListID), IDB_TOOLBAR_OPT,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(6,  ImageListID), IDB_TOOLBAR_SEC,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(7,  ImageListID), IDB_TOOLBAR_DIR,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { 0, 0,  TBSTATE_ENABLED, (BYTE)BTNS_SEP, {0}, 0, NULL },
        { MAKELONG(8,  ImageListID), IDB_TOOLBAR_EX,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
        { MAKELONG(9,  ImageListID), IDB_TOOLBAR_IM,  TBSTATE_ENABLED, (BYTE)BTNS_BUTTON, {0}, 0, NULL },
    
    }; 

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);



    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}

wstring gui_OpenFile(HWND hwnd)
{
    IFileDialog* pfd = NULL;
    HRESULT hr = NULL;
    
    wstring fileName;
    
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        DWORD dwFlags;
        HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            
        // set the options of files: show hidden files | no deference links
        hr = pfd->GetOptions(&dwFlags);
        hr = pfd->SetOptions(dwFlags | FOS_FORCESHOWHIDDEN | FOS_NODEREFERENCELINKS);
        if (SUCCEEDED(hr))
        {
            // show the dialog
            hr = pfd->Show(hwnd);
            if (SUCCEEDED(hr))
            {
                // get the results
                IShellItem* pSelectItem;
                hr = pfd->GetResult(&pSelectItem);
                if (SUCCEEDED(hr))
                {
                    // get the file path
                    PWSTR pszFilePath = NULL;
                    hr = pSelectItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath);
                    if (SUCCEEDED(hr))
                    {
                        fileName.assign(pszFilePath);   // copy the file path
                        CoTaskMemFree(pszFilePath);     // clean memory
                    }
                }
                pSelectItem->Release();

            }
        }
    }
    pfd->Release();

    return fileName;
}

bool gui_InitProcessList(HWND hDlg)
{
    HWND hListProcess = GetDlgItem(hDlg, IDC_FILE_OPENPRO_PRO);
    if (hListProcess == NULL)
        return false;
    
    // set process columns
    TCHAR buffer[MAX_PATH];
    LVCOLUMN lv;
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hListProcess, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    _stprintf(buffer, TEXT("Name"));
    lv.pszText = buffer;
    lv.cx = 200;
    lv.iSubItem = 0;
    ListView_InsertColumn(hListProcess, 0, &lv);

    _stprintf(buffer, TEXT("PID"));
    lv.cx = 80;
    lv.iSubItem = 1;
    ListView_InsertColumn(hListProcess, 1, &lv);

    // enumarate all processes
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == NULL)
        return false;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    bool more = false;
    
    int count = 0;
    
    LV_ITEM item;
    memset(&item, 0, sizeof(LV_ITEM));
    item.mask = LVIF_TEXT;

    more = Process32First(hProcessSnap, &pe32);
    while (more)
    {
        item.pszText = pe32.szExeFile;
        item.iItem = count;
        item.iSubItem = 0;
        ListView_InsertItem(hListProcess, &item);      

        _stprintf(buffer, TEXT("%d"), pe32.th32ProcessID);
        item.pszText = buffer;
        item.iItem = count;
        item.iSubItem = 1;
        ListView_SetItem(hListProcess, &item);

        count++;
        more = Process32Next(hProcessSnap, &pe32);
    }

    CloseHandle(hProcessSnap);
    
    // initialize module list
    HWND hListMod = NULL;
    hListMod = GetDlgItem(hDlg, IDC_FILE_OPENPRO_MOD);
    if (hListMod == NULL)
        return false;

    // set function column headers
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hListMod, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lv.pszText = LPWSTR(TEXT("Name"));
    lv.cx = 150;
    lv.iSubItem = 0;
    ListView_InsertColumn(hListMod, 0, &lv);

    lv.pszText = LPWSTR(TEXT("ImageBase"));
    lv.cx = 120;
    lv.iSubItem = 1;
    ListView_InsertColumn(hListMod, 1, &lv);

    lv.pszText = LPWSTR(TEXT("ImageSize"));
    lv.cx = 80;
    lv.iSubItem = 2;
    ListView_InsertColumn(hListMod, 2, &lv);

    lv.pszText = LPWSTR(TEXT("EntryPoint"));
    lv.cx = 120;
    lv.iSubItem = 3;
    ListView_InsertColumn(hListMod, 3, &lv);

    lv.pszText = LPWSTR(TEXT("PID"));
    lv.cx = 60;
    lv.iSubItem = 4;
    ListView_InsertColumn(hListMod, 4, &lv);
   
    return true;
}

bool gui_DisplayModuleList(HWND hDlg, PEFile* pe)
{
    HWND hListProcess = GetDlgItem(hDlg, IDC_FILE_OPENPRO_PRO);
    HWND hListModule = GetDlgItem(hDlg, IDC_FILE_OPENPRO_MOD);
    if (hListProcess == NULL || hListModule == NULL)
        return false;

    const int bufferSize = 0x200;
    TCHAR buffer[bufferSize];
    memset(buffer, 0, sizeof(buffer));

    DWORD selectedRow = 0;
    LV_ITEM ProcessItem;
    memset(&ProcessItem, 0, sizeof(LV_ITEM));
    
    // get the PID of the selected process
    ListView_DeleteAllItems(hListModule);
    selectedRow = (DWORD)ListView_GetNextItem(hListProcess, -1, LVNI_SELECTED);
    ListView_GetItemText(hListProcess, selectedRow, 1, buffer, bufferSize);
    DWORD PID = (DWORD)_tcstoll(buffer, NULL, 10);
    if (PID == 0)
        return false;

    // get debug privilege
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tokenPriv;
    LUID luidDebug;

    memset(&tokenPriv, 0, sizeof(TOKEN_PRIVILEGES));
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

    if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug) != FALSE)
    {
        tokenPriv.PrivilegeCount = 1;
        tokenPriv.Privileges[0].Luid = luidDebug;
        tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, 0, NULL, NULL);
    }
    CloseHandle(hToken);

    // enumarate modules
    HANDLE hPro = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, PID);
    if (hPro == NULL)
        return false;

    DWORD maxSize = 0x100;
    DWORD iNeeded = 0;
    HMODULE* hModule = new HMODULE[maxSize];

    if (EnumProcessModulesEx(hPro, hModule, maxSize * sizeof(HMODULE), &iNeeded, LIST_MODULES_ALL) == 0)
        return false;
    
    if (iNeeded > maxSize * sizeof(HMODULE))
    {
        delete[] hModule;
        maxSize = iNeeded / sizeof(HMODULE);
        hModule = new HMODULE[maxSize];
        EnumProcessModulesEx(hPro, hModule, maxSize * sizeof(HMODULE), &iNeeded, LIST_MODULES_ALL);
    }


    LV_ITEM modItem;
    memset(&modItem, 0, sizeof(LV_ITEM));
    modItem.mask = LVIF_TEXT;

    MODULEINFO modInfo;
    memset(&modInfo, 0, sizeof(MODULEINFO));

    // populate the module list
    for (int i = 0; i < iNeeded / sizeof(HMODULE); i++)
    {
        // module name
        GetModuleBaseName(hPro, hModule[i], buffer, bufferSize);
        modItem.pszText = buffer;
        modItem.iItem = i;
        modItem.iSubItem = 0;
        ListView_InsertItem(hListModule, &modItem);
        
        bool res = GetModuleInformation(hPro, hModule[i], &modInfo, sizeof(MODULEINFO));
        if (res == true)
        {
            // base address
            _stprintf(buffer, TEXT("%I64X"), (DWORD64)modInfo.lpBaseOfDll);
            modItem.pszText = buffer;
            modItem.iItem = i;
            modItem.iSubItem = 1;
            ListView_SetItem(hListModule, &modItem);

            // image size
            _stprintf(buffer, TEXT("%X"), modInfo.SizeOfImage);
            modItem.iSubItem = 2;
            ListView_SetItem(hListModule, &modItem);

            // entry point
            _stprintf(buffer, TEXT("%I64X"), (DWORD64)modInfo.EntryPoint);
            modItem.iSubItem = 3;
            ListView_SetItem(hListModule, &modItem);

            // pid
            _stprintf(buffer, TEXT("%d"), PID);
            modItem.iSubItem = 4;
            ListView_SetItem(hListModule, &modItem);
        }
    }

    CloseHandle(hPro);
    delete[] hModule;

    return true;
}

bool gui_GetModuleInfo(HWND hDlg, PDWORD pPID, PDWORD64 pImageBase, PDWORD pImageSize)
{
    HWND hListModule = GetDlgItem(hDlg, IDC_FILE_OPENPRO_MOD);
    if (hListModule == NULL)
        return false;

    const int bufferSize = 0x200;
    TCHAR buffer[bufferSize];
    memset(buffer, 0, sizeof(buffer));

    DWORD selectedRow = 0;
    LV_ITEM ProcessItem;
    memset(&ProcessItem, 0, sizeof(LV_ITEM));

    selectedRow = (DWORD)ListView_GetNextItem(hListModule, -1, LVNI_SELECTED);

    // get info from the selected row
    ListView_GetItemText(hListModule, selectedRow, 4, buffer, bufferSize);
    *pPID = (DWORD)_tcstoll(buffer, NULL, 10);
    
    ListView_GetItemText(hListModule, selectedRow, 1, buffer, bufferSize);
    *pImageBase = (DWORD64)_tcstoll(buffer, NULL, 16);

    ListView_GetItemText(hListModule, selectedRow, 2, buffer, bufferSize);
    *pImageSize = (DWORD)_tcstoll(buffer, NULL, 16);

    return true;
}

bool gui_DisplayDos(HWND hDlg, PEFile* pe)
{
    IMAGE_DOS_HEADER tempDos;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    int bufferByte = sizeof(TCHAR) * bufferSize;

    if (pe != NULL && pe->isImage())
    {
        tempDos = pe->getDosHeader();
        if (tempDos.e_lfanew != 0)
        {
            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_magic);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MAGIC), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_cp);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CP), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_cblp);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CBLP), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_crlc);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CRIC), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_cparhdr);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CPARHDR), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_minalloc);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MIN), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_maxalloc);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MAX), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_ss);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_SS), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_sp);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_SP), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_csum);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CSUM), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_ip);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_IP), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_cs);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CS), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_lfarlc);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_LFARLC), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_ovno);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OVNO), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X%0.4X%0.4X%0.4X"),
                tempDos.e_res[0], tempDos.e_res[1], tempDos.e_res[2], tempDos.e_res[3]);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_RES4), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_oemid);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OEMID), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempDos.e_oeminfo);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OEMINFO), buffer);

            TCHAR* pBuffer = buffer;
            for (unsigned int i = 0; i < 10; i++, pBuffer += 4)
            {
                StringCbPrintf(pBuffer, bufferByte - i * sizeof(TCHAR) * 2, TEXT("%0.4X"), tempDos.e_res2[i]);
            }
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_RES10), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempDos.e_lfanew);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_LFANEW), buffer);

            return true;
        }
    }
   
    return false;
}

bool gui_SaveDos(HWND hDlg, PEFile* pe)
{
    IMAGE_DOS_HEADER tempDos;
    const int bufferSize = 0x50;
    const int subSize = 0x10;
    TCHAR buffer[bufferSize];
    TCHAR subBuffer[subSize];

    memset(&tempDos, 0, sizeof(IMAGE_DOS_HEADER));

    if (pe != NULL && pe->isImage())
    {
        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MAGIC), buffer, bufferSize);
        tempDos.e_magic = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CP), buffer, bufferSize);
        tempDos.e_cp = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CBLP), buffer, bufferSize);
        tempDos.e_cblp = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CRIC), buffer, bufferSize);
        tempDos.e_crlc = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CPARHDR), buffer, bufferSize);
        tempDos.e_cparhdr = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MIN), buffer, bufferSize);
        tempDos.e_minalloc = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_MAX), buffer, bufferSize);
        tempDos.e_maxalloc = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_SS), buffer, bufferSize);
        tempDos.e_ss = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_SP), buffer, bufferSize);
        tempDos.e_sp = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CSUM), buffer, bufferSize);
        tempDos.e_csum = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_IP), buffer, bufferSize);
        tempDos.e_ip = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_CS), buffer, bufferSize);
        tempDos.e_cs = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_LFARLC), buffer, bufferSize);
        tempDos.e_lfarlc = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OVNO), buffer, bufferSize);
        tempDos.e_ovno = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_RES4), buffer, bufferSize);
        for (size_t i = 0; i < 4; i++)
        {
            if (_tcslen(buffer) < i * 4 + 4)
                tempDos.e_res[i] = 0;
            else
            {
                _tcsncpy(subBuffer, buffer + i * 4, 4);
                subBuffer[4] = L'\0';
                tempDos.e_res[i] = (WORD)_tcstoll(subBuffer, NULL, 16);
            }
        }
        
        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OEMID), buffer, bufferSize);
        tempDos.e_oemid = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_OEMINFO), buffer, bufferSize);
        tempDos.e_oeminfo = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_RES10), buffer, bufferSize);
        for (size_t i = 0; i < 10; i++)
        {
            if (_tcslen(buffer) < i * 4 + 4)
                tempDos.e_res2[i] = 0;
            else
            {
                _tcsncpy(subBuffer, buffer + i * 4, 4);
                subBuffer[4] = L'\0';
                tempDos.e_res2[i] = (WORD)_tcstoll(subBuffer, NULL, 16);
            }
        }

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_DOS_LFANEW), buffer, bufferSize);
        tempDos.e_lfanew = (WORD)_tcstoll(buffer, NULL, 16);

        pe->setDosHeader(&tempDos);

        return true;
    }
    else
        return false;
}

bool gui_DisplayPEHeader(HWND hDlg, PEFile* pe)
{
    IMAGE_FILE_HEADER tempFile;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    int bufferByte = sizeof(TCHAR) * bufferSize;

    if (pe != NULL && pe->isImage())
    {
        tempFile = pe->getFileHeader();
        if (tempFile.NumberOfSections != 0)
        {
            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempFile.Machine);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_MACHINE), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempFile.NumberOfSections);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_SECTIONS), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempFile.TimeDateStamp);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_TIME), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempFile.PointerToSymbolTable);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_ST), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempFile.NumberOfSymbols);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_NUMSYM), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempFile.SizeOfOptionalHeader);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_SIZEOPT), buffer);

            StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempFile.Characteristics);
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_CHAR), buffer);

            return true;
        }
    }

    return false;
}

bool gui_SavePE(HWND hDlg, PEFile* pe)
{
    IMAGE_FILE_HEADER tempPE;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];

    memset(&tempPE, 0, sizeof(IMAGE_FILE_HEADER));

    if (pe != NULL && pe->isImage())
    {
        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_MACHINE), buffer, bufferSize);
        tempPE.Machine = (WORD)_tcstoll(buffer, NULL, 16);
 
        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_SECTIONS), buffer, bufferSize);
        tempPE.NumberOfSections = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_TIME), buffer, bufferSize);
        tempPE.TimeDateStamp = (DWORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_ST), buffer, bufferSize);
        tempPE.PointerToSymbolTable = (DWORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_NUMSYM), buffer, bufferSize);
        tempPE.NumberOfSymbols = (DWORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_SIZEOPT), buffer, bufferSize);
        tempPE.SizeOfOptionalHeader = (WORD)_tcstoll(buffer, NULL, 16);

        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FILE_CHAR), buffer, bufferSize);
        tempPE.Characteristics = (WORD)_tcstoll(buffer, NULL, 16);

        pe->setPEHeader(&tempPE);

        return true;
    }
    else
        return false;
}


bool gui_DisplayOptionalHeader(HWND hDlg, PEFile* pe)
{
    byte* tempOpt = NULL;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    int bufferByte = sizeof(TCHAR) * bufferSize;

    if (pe != NULL && pe->isImage())
    {
        int bit = pe->getBit();
        pe->getOptionalHeader((void**)(&tempOpt));
            

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)tempOpt);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_MAGIC), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.2X"), *(tempOpt + 2));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ALINDER), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.2X"), *(tempOpt + 3));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ILINKER), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 4));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SIZECODE), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 8));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IDATA), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 12));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_UDATA), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 16));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ENTRY), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 20));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_BASECODE), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("NULL"));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 24));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_BASEDATA), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.16I64X"), *(PDWORD64)(tempOpt + 24));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 28));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IMAGEBASE), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 32));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SA), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 36));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_FA), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 40));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_AOSV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 42));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IOSV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 44));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_AIV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 46));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IIV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 48));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ASV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 50));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ISV), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 52));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_WIN32), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 56));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IMAGESIZE), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 60));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HSIZE), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 64));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_CHECKSUM), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 68));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SUB), buffer);

        StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), *(PWORD)(tempOpt + 70));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_DLL), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.16I64X"), *(PDWORD64)(tempOpt + 72));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 72));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_STACKR), buffer);
        
        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.16I64X"), *(PDWORD64)(tempOpt + 80));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 76));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_STACKC), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.16I64X"), *(PDWORD64)(tempOpt + 88));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 80));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HEAPR), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.16I64X"), *(PDWORD64)(tempOpt + 96));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 84));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HEAPC), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 104));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 88));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_FLAGS), buffer);

        if (bit == 64)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 108));
        else if (bit == 32)
            StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), *(PDWORD)(tempOpt + 92));
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_DIRNUM), buffer);
        
        delete[] tempOpt;
        tempOpt = NULL;

        return true;
       
    }

    return false;
}

bool gui_SaveOptional(HWND hDlg, PEFile* pe)
{
    int bit = pe->getBit();
    int size = pe->getFileHeader().SizeOfOptionalHeader;
    byte* tempOpt = new byte[size];
    memset(tempOpt, 0, size);

    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];

    if (pe == NULL || pe->isImage() == false)
        return false;

   
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_MAGIC), buffer, bufferSize);
    *(PWORD)tempOpt = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ALINDER), buffer, bufferSize);
    *(tempOpt + 2) = (byte)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ILINKER), buffer, bufferSize);
    *(tempOpt + 3) = (byte)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SIZECODE), buffer, bufferSize);
    *(PDWORD)(tempOpt + 4) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IDATA), buffer, bufferSize);
    *(PDWORD)(tempOpt + 8) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_UDATA), buffer, bufferSize);
    *(PDWORD)(tempOpt + 12) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ENTRY), buffer, bufferSize);
    *(PDWORD)(tempOpt + 16) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_BASECODE), buffer, bufferSize);
    *(PDWORD)(tempOpt + 20) = (DWORD)_tcstoll(buffer, NULL, 16);

    if (bit == 32)
    {
        GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_BASEDATA), buffer, bufferSize);
        *(PDWORD)(tempOpt + 24) = (DWORD)_tcstoll(buffer, NULL, 16);
    }
        
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IMAGEBASE), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD64)(tempOpt + 24) = _tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 28) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SA), buffer, bufferSize);
    *(PDWORD)(tempOpt + 32) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_FA), buffer, bufferSize);
    *(PDWORD)(tempOpt + 36) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_AOSV), buffer, bufferSize);
    *(PWORD)(tempOpt + 40) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IOSV), buffer, bufferSize);
    *(PWORD)(tempOpt + 42) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_AIV), buffer, bufferSize);
    *(PWORD)(tempOpt + 44) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IIV), buffer, bufferSize);
    *(PWORD)(tempOpt + 46) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ASV), buffer, bufferSize);
    *(PWORD)(tempOpt + 48) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_ISV), buffer, bufferSize);
    *(PWORD)(tempOpt + 50) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_WIN32), buffer, bufferSize);
    *(PDWORD)(tempOpt + 52) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_IMAGESIZE), buffer, bufferSize);
    *(PDWORD)(tempOpt + 56) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HSIZE), buffer, bufferSize);
    *(PDWORD)(tempOpt + 60) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_CHECKSUM), buffer, bufferSize);
    *(PDWORD)(tempOpt + 64) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_SUB), buffer, bufferSize);
    *(PWORD)(tempOpt + 68) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_DLL), buffer, bufferSize);
    *(PWORD)(tempOpt + 70) = (WORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_STACKR), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD64)(tempOpt + 72) = _tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 72) = (DWORD)_tcstoll(buffer, NULL, 16);
        
    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_STACKC), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD64)(tempOpt + 80) = _tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 76) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HEAPR), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD64)(tempOpt + 88) = _tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 80) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_HEAPC), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD64)(tempOpt + 96) = _tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 84) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_FLAGS), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD)(tempOpt + 104) = (DWORD)_tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 88) = (DWORD)_tcstoll(buffer, NULL, 16);

    GetWindowText(GetDlgItem(hDlg, IDC_EDIT_OPT_DIRNUM), buffer, bufferSize);
    if (bit == 64)
        *(PDWORD)(tempOpt + 108) = (DWORD)_tcstoll(buffer, NULL, 16);
    else if (bit == 32)
        *(PDWORD)(tempOpt + 92) = (DWORD)_tcstoll(buffer, NULL, 16);

    pe->setOptionalHeader(tempOpt);
    delete[] tempOpt;

    return true;
    
}

bool InitSectionListView(HWND hDlg, PEFile* pe)
{
    HWND hSectionList = GetDlgItem(hDlg, IDC_SEC_LIST);
    if (hSectionList == NULL)
        return false;

    // initialize column headers
    LVCOLUMN lv;
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hSectionList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lv.pszText = (LPWSTR)TEXT("Name");
    lv.cx = 80;
    lv.iSubItem = 0;
    ListView_InsertColumn(hSectionList, 0, &lv);

    lv.pszText = (LPWSTR)TEXT("VirtualAddress");
    lv.cx = 100;
    lv.iSubItem = 1;
    ListView_InsertColumn(hSectionList, 1, &lv);

    lv.pszText = (LPWSTR)TEXT("VirtualSize");
    lv.cx = 100;
    lv.iSubItem = 2;
    ListView_InsertColumn(hSectionList, 2, &lv);

    lv.pszText = (LPWSTR)TEXT("PointerToRawData");
    lv.cx = 110;
    lv.iSubItem = 3;
    ListView_InsertColumn(hSectionList, 3, &lv);

    lv.pszText = (LPWSTR)TEXT("SizeOfRawData");
    lv.cx = 100;
    lv.iSubItem = 4;
    ListView_InsertColumn(hSectionList, 4, &lv);

    lv.pszText = (LPWSTR)TEXT("Characteristics");
    lv.cx = 100;
    lv.iSubItem = 5;
    ListView_InsertColumn(hSectionList, 5, &lv);

    if (pe == NULL || pe->getBit() == 0)
        return false;

    // set column values
    IMAGE_FILE_HEADER PEHeader = pe->getFileHeader();

    const int bufferSize = 0x10;
    TCHAR buffer[bufferSize];
    memset(buffer, 0, bufferSize);

    LVITEM item;
    memset(&item, 0, sizeof(LVITEM));
    item.mask = LVIF_TEXT;

    for (int i = 0; i < PEHeader.NumberOfSections; i++)
    {
        const int nameLength = 8;
        IMAGE_SECTION_HEADER tempSec;
        tempSec = pe->getSecHeader(i);

        //section name
        if (sizeof(TCHAR) == 2)
            mbstowcs(buffer, (char*)(tempSec.Name), nameLength);
        else
            strncpy((char*)buffer, (char*)tempSec.Name, nameLength);
        buffer[nameLength] = TEXT('\0');
        
        item.pszText = buffer;
        item.iItem = i;
        item.iSubItem = 0;
        ListView_InsertItem(hSectionList, &item);
            
        //virtual address
        _stprintf(buffer, TEXT("%0.8X"), tempSec.VirtualAddress);
        item.iSubItem = 1;
        ListView_SetItem(hSectionList, &item);

        //virtual size
        _stprintf(buffer, TEXT("%0.8X"), tempSec.Misc.VirtualSize);
        item.iSubItem = 2;
        ListView_SetItem(hSectionList, &item);
            
        //disk address
        _stprintf(buffer, TEXT("%0.8X"), tempSec.PointerToRawData);
        item.iSubItem = 3;
        ListView_SetItem(hSectionList, &item);

        //disk size
        _stprintf(buffer, TEXT("%0.8X"), tempSec.SizeOfRawData);
        item.iSubItem = 4;
        ListView_SetItem(hSectionList, &item);

        //characteristics
        _stprintf(buffer, TEXT("%0.8X"), tempSec.Characteristics);
        item.iSubItem = 5;
        ListView_SetItem(hSectionList, &item);
    }

    return true;
}

void gui_showSectAttribute(HWND hDlg)
{
    HWND hSecList;
    DWORD selectedRow = 0;
    LV_ITEM secItem;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];

    memset(&secItem, 0, sizeof(secItem));
    memset(buffer, 0, sizeof(buffer));
    hSecList = GetDlgItem(hDlg, IDC_SEC_LIST);

    // get the RVA of the lookup table of the selected dll
    selectedRow = (DWORD)ListView_GetNextItem(hSecList, -1, LVNI_SELECTED);
    ListView_GetItemText(hSecList, selectedRow, 5, buffer, bufferSize);
    DWORD attribute = (DWORD)_tcstoll(buffer, NULL, 16);

    
    if (attribute & 0x00000020)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK1), BST_CHECKED);
    else 
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK1), BST_UNCHECKED);

    if (attribute & 0x00000040)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK2), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK2), BST_UNCHECKED);

    if (attribute & 0x00000080)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK3), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK3), BST_UNCHECKED);

    if (attribute & 0x00008000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK4), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK4), BST_UNCHECKED);

    if (attribute & 0x01000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK5), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK5), BST_UNCHECKED);

    if (attribute & 0x02000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK6), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK6), BST_UNCHECKED);

    if (attribute & 0x04000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK7), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK7), BST_UNCHECKED);

    if (attribute & 0x08000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK8), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK8), BST_UNCHECKED);

    if (attribute & 0x10000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK9), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK9), BST_UNCHECKED);

    if (attribute & 0x20000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK10), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK10), BST_UNCHECKED);

    if (attribute & 0x40000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK11), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK11), BST_UNCHECKED);

    if (attribute & 0x80000000)
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK12), BST_CHECKED);
    else
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEC_CHECK12), BST_UNCHECKED);
    
}

bool gui_DisplayDirectory(HWND hDlg, PEFile* pe)
{
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    int bufferByte = sizeof(TCHAR) * bufferSize;

    if (pe == NULL || pe->isImage() == false)
        return false;

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(0).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EXA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(0).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EXS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(1).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_IMA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(1).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_IMS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(2).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RESA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(2).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RESS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(3).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EXCA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(3).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EXCS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(4).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_SECA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(4).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_SECS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(5).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RELA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(5).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RELS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(6).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_DEBA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(6).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_DEBS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(7).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_ARCHA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(7).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_ARCHS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(8).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_GLOA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(8).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_GLOS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(9).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_TLSA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(9).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_TLSS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(10).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_LOADA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(10).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_LOADS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(11).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_BIMPA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(11).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_BIMPS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(12).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_IATA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(12).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_IATS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(13).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_DELA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(13).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_DELS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(14).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_COMA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(14).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_COMS), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(15).VirtualAddress);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RA), buffer);
    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), pe->getDirectory(15).Size);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_RS), buffer);

    return true;
}

bool gui_DisplayExport(HWND hDlg, PEFile* pe)
{
    IMAGE_EXPORT_DIRECTORY tempExport;
    
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    int bufferByte = sizeof(TCHAR) * bufferSize;

    if (pe == NULL || pe->isExport() == false)
        return false;

    string name;
    tempExport = pe->getExport();
    name = pe->getString(tempExport.Name);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.Characteristics);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_CHA), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.TimeDateStamp);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_TIME), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempExport.MajorVersion);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_MAJ), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.4X"), tempExport.MinorVersion);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_MIN), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.Name);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_NAMER), buffer);

    mbstowcs(buffer, name.c_str(), bufferSize);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_NAMES), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.Base);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_BASE), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.NumberOfFunctions);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_NOFUN), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.NumberOfNames);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_NONAME), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.AddressOfFunctions);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_ADFUN), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.AddressOfNames);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_ADNAME), buffer);

    StringCbPrintf(buffer, bufferByte, TEXT("%0.8X"), tempExport.AddressOfNameOrdinals);
    SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DIR_EX_ADORD), buffer);
     
    return true;
}

bool InitExportList(HWND hDlg, PEFile* pe)
{  
    HWND hListExport = GetDlgItem(hDlg, IDC_EDIT_DIR_EX_LIST);
    if (hListExport == NULL)
        return false;

    // set columns
    LVCOLUMN lv;
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hListExport, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lv.pszText = LPWSTR(TEXT("Ordinals"));
    lv.cx = 100;
    lv.iSubItem = 0;
    ListView_InsertColumn(hListExport, 0, &lv);

    lv.pszText = LPWSTR(TEXT("Function Address"));
    lv.cx = 100;
    lv.iSubItem = 1;
    ListView_InsertColumn(hListExport, 1, &lv);

    lv.pszText = LPWSTR(TEXT("Names"));
    lv.cx = 250;
    lv.iSubItem = 2;
    ListView_InsertColumn(hListExport, 2, &lv);

    if (pe == NULL || pe->isExport() == false)
        return false;

    // populating value to the list
         
    int funcSize = 0;
    ExportFunction* tempExportFunc = NULL;

    funcSize = pe->getExportFunctions(&tempExportFunc);     

    if (tempExportFunc != NULL)
    {
        const int bufferSize = 0x30;
        TCHAR buffer[bufferSize] = { 0 };

        LV_ITEM item;
        memset(&item, 0, sizeof(LV_ITEM));
        item.mask = LVIF_TEXT;

        // assign values in the list
        for (int i = 0; i < funcSize; i++)
        {
            // ordinal
            _stprintf(buffer, TEXT("%0.4x"), tempExportFunc[i].ordinal);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 0;
            ListView_InsertItem(hListExport, &item);

            // function RVA
            _stprintf(buffer, TEXT("%0.8x"), tempExportFunc[i].address);
            item.iSubItem = 1;
            ListView_SetItem(hListExport, &item);

            // function name
            if (tempExportFunc[i].name == "-")
                _stprintf(buffer, TEXT("-"));
            else
                mbstowcs(buffer, tempExportFunc[i].name.c_str(), bufferSize);
                    
            item.iSubItem = 2;
            ListView_SetItem(hListExport, &item);
                    
        }

        delete[] tempExportFunc;
    }
 
    return true;
}

bool InitImportList(HWND hDlg, PEFile* pe)
{
    HWND hListDll = GetDlgItem(hDlg, IDC_EDIT_DIR_IMPORT_DLL);
    if (hListDll == NULL)
        return false;

    // set columns
    LVCOLUMN lv;
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hListDll, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lv.pszText = LPWSTR(TEXT("Dll Name"));
    lv.cx = 150;
    lv.iSubItem = 0;
    ListView_InsertColumn(hListDll, 0, &lv);

    lv.pszText = LPWSTR(TEXT("TimeDateStamp"));
    lv.cx = 80;
    lv.iSubItem = 1;
    ListView_InsertColumn(hListDll, 1, &lv);

    lv.pszText = LPWSTR(TEXT("INT RVA"));
    lv.cx = 80;
    lv.iSubItem = 2;
    ListView_InsertColumn(hListDll, 2, &lv);

    lv.pszText = LPWSTR(TEXT("IAT RVA"));
    lv.cx = 80;
    lv.iSubItem = 3;
    ListView_InsertColumn(hListDll, 3, &lv);
        
    lv.pszText = LPWSTR(TEXT("Name RVA"));
    lv.cx = 80;
    lv.iSubItem = 4;
    ListView_InsertColumn(hListDll, 4, &lv);

    lv.pszText = LPWSTR(TEXT("ForwarderChain"));
    lv.cx = 80;
    lv.iSubItem = 5;
    ListView_InsertColumn(hListDll, 5, &lv);
        
    // populating value for all import Dlls
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = NULL;
    int numOfImport = 0;
            
    if (pe != NULL && pe->isImport())
    {         
        LV_ITEM item;
        const int bufferSize = 0x50;
        TCHAR buffer[bufferSize] = { 0 };
            
        numOfImport = pe->getImport(&importDescriptor);
        memset(&item, 0, sizeof(LV_ITEM));
        item.mask = LVIF_TEXT;

        // assign values to the dll list
        for (int i = 0; i < numOfImport; i++)
        {
            // Dll Name
            string name = pe->getString(importDescriptor[i].Name);
            mbstowcs(buffer, name.c_str(), bufferSize);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 0;
            ListView_InsertItem(hListDll, &item);


            // TimeDateStamp
            _stprintf(buffer, TEXT("%0.8X"), importDescriptor[i].TimeDateStamp);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 1;
            ListView_SetItem(hListDll, &item);

            // INT RVA
            _stprintf(buffer, TEXT("%0.8X"), importDescriptor[i].Characteristics);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 2;
            ListView_SetItem(hListDll, &item);

            // IAT RVA
            _stprintf(buffer, TEXT("%0.8X"), importDescriptor[i].FirstThunk);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 3;
            ListView_SetItem(hListDll, &item);

            // Name RVA
            _stprintf(buffer, TEXT("%0.8X"), importDescriptor[i].Name);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 4;
            ListView_SetItem(hListDll, &item);

            // Forwarder Chain
            _stprintf(buffer, TEXT("%0.8X"), importDescriptor[i].ForwarderChain);
            item.pszText = buffer;
            item.iItem = i;
            item.iSubItem = 5;
            ListView_SetItem(hListDll, &item);
        }
            
    }
        
    // initialize function list
    HWND hListFunc = GetDlgItem(hDlg, IDC_EDIT_DIR_IMPORT_FUNC);
    if (hListFunc == NULL)
        return false;

    // set function column headers
    memset(&lv, 0, sizeof(LVCOLUMN));
    SendMessage(hListFunc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lv.pszText = LPWSTR(TEXT("Lookup Value"));
    lv.cx = 120;
    lv.iSubItem = 0;
    ListView_InsertColumn(hListFunc, 0, &lv);

    lv.pszText = LPWSTR(TEXT("Ordinal"));
    lv.cx = 60;
    lv.iSubItem = 1;
    ListView_InsertColumn(hListFunc, 1, &lv);

    lv.pszText = LPWSTR(TEXT("Hint"));
    lv.cx = 60;
    lv.iSubItem = 2;
    ListView_InsertColumn(hListFunc, 2, &lv);

    lv.pszText = LPWSTR(TEXT("Function Name"));
    lv.cx = 150;
    lv.iSubItem = 3;
    ListView_InsertColumn(hListFunc, 3, &lv);

    lv.pszText = LPWSTR(TEXT("Function Address"));
    lv.cx = 120;
    lv.iSubItem = 4;
    ListView_InsertColumn(hListFunc, 4, &lv);

    delete[] importDescriptor;
    return true;
}

bool gui_DisplayImportFuncList(HWND hDlg, PEFile* pe)
{
    HWND hListDll = GetDlgItem(hDlg, IDC_EDIT_DIR_IMPORT_DLL);
    HWND hListFunc = GetDlgItem(hDlg, IDC_EDIT_DIR_IMPORT_FUNC);
    if (hListDll == NULL || hListFunc == NULL)
        return false;

    DWORD selectedRow = 0;
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    memset(buffer, 0, sizeof(buffer));

    // get the RVA of the lookup table of the selected dll
    ListView_DeleteAllItems(hListFunc);
    selectedRow = (DWORD)ListView_GetNextItem(hListDll, -1, LVNI_SELECTED);
    ListView_GetItemText(hListDll, selectedRow, 2, buffer, bufferSize);
    int INT_RVA = (DWORD)_tcstoll(buffer, NULL, 16);
    
    // get the RVA of the address table of the selected dll
    ListView_GetItemText(hListDll, selectedRow, 3, buffer, bufferSize);
    int IAT_RVA = (DWORD)_tcstoll(buffer, NULL, 16);

    // populate the function list
    if (pe != NULL && pe->isImport())
    {
        LV_ITEM funcItem;

        byte* lookup = NULL;
        byte* address = NULL;

        int bit = pe->getBit();
        int size = pe->getImportLookupThunk((void**)&lookup, INT_RVA);
        pe->getImportLookupThunk((void**)&address, INT_RVA);
        memset(&funcItem, 0, sizeof(funcItem));
        funcItem.mask = LVIF_TEXT;

        for (int i = 0; i < size; i++)
        {
            // 32-bit value
            if (bit == 32)
            {
                PDWORD lookup32 = (PDWORD)lookup;
                PDWORD address32 = (PDWORD)address;

                // Lookup value
                _stprintf(buffer, TEXT("%0.8X"), lookup32[i]);
                funcItem.pszText = buffer;
                funcItem.iItem = i;
                funcItem.iSubItem = 0;
                ListView_InsertItem(hListFunc, &funcItem);

                // import by ordinal
                if (lookup32[i] & 0x80000000)
                {
                    _stprintf(buffer, TEXT("%0.4X"), lookup32[i] & 0xFFFF);
                    funcItem.iSubItem = 1;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("-"));              
                    funcItem.iSubItem = 2;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("-"));
                    funcItem.iSubItem = 3;
                    ListView_SetItem(hListFunc, &funcItem);
                }
                // import by name
                else
                {                 
                    _stprintf(buffer, TEXT("-"));
                    funcItem.pszText = buffer;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("%0.4X"), pe->getHint(lookup32[i]));
                    funcItem.iSubItem = 2;
                    ListView_SetItem(hListFunc, &funcItem);

                    string str = pe->getString(lookup32[i] + 2);
                    mbstowcs(buffer, str.c_str(), bufferSize); 
                    funcItem.iSubItem = 3;
                    ListView_SetItem(hListFunc, &funcItem);
                }
                // address
                _stprintf(buffer, TEXT("%0.8X"), address32[i]);
                funcItem.iSubItem = 4;
                ListView_SetItem(hListFunc, &funcItem);
            }
            // 64-bit value
            else if (bit == 64)
            {
                PDWORD64 lookup64 = (PDWORD64)lookup;
                PDWORD64 address64 = (PDWORD64)address;

                // Lookup value
                _stprintf(buffer, TEXT("%0.16I64X"), lookup64[i]);
                funcItem.pszText = buffer;
                funcItem.iItem = i;
                funcItem.iSubItem = 0;
                ListView_InsertItem(hListFunc, &funcItem);

                // import by ordinal
                if (lookup64[i] & 0x8000000000000000)
                {
                    _stprintf(buffer, TEXT("%0.4X"), (short)(lookup64[i] & 0xFFFF));
                    funcItem.iSubItem = 1;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("-"));
                    funcItem.iSubItem = 2;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("-"));
                    funcItem.iSubItem = 3;
                    ListView_SetItem(hListFunc, &funcItem);
                }
                // import by name
                else
                {                                 
                    _stprintf(buffer, TEXT("-"));
                    funcItem.iSubItem = 1;
                    ListView_SetItem(hListFunc, &funcItem);

                    _stprintf(buffer, TEXT("%0.4X"), pe->getHint((int)(lookup64[i] & 0x7FFFFFFF)));
                    funcItem.iSubItem = 2;
                    ListView_SetItem(hListFunc, &funcItem);

                    string str = pe->getString((int)(lookup64[i] + 2));
                    mbstowcs(buffer, str.c_str(), bufferSize);
                    funcItem.iSubItem = 3;
                    ListView_SetItem(hListFunc, &funcItem);
                        
                }
                // address
                _stprintf(buffer, TEXT("%0.16I64X"), address64[i]);
                funcItem.iSubItem = 4;
                ListView_SetItem(hListFunc, &funcItem);
            }
            

        }
        
        if (lookup != NULL)
            delete[] lookup;
        if (address != NULL)
            delete[] address;
    }
    return true;
}

void gui_setConverterEdit(HWND hDlg)
{
 
    if (IsDlgButtonChecked(hDlg, IDC_CONV_MRVA) == BST_CHECKED)
    {
        Edit_SetReadOnly(GetDlgItem(hDlg, IDC_CONV_RVA), false);
        Edit_SetReadOnly(GetDlgItem(hDlg, IDC_CONV_FOA), true);
    }
    else if (IsDlgButtonChecked(hDlg, IDC_CONV_MFOA) == BST_CHECKED)
    {
        Edit_SetReadOnly(GetDlgItem(hDlg, IDC_CONV_RVA), true);
        Edit_SetReadOnly(GetDlgItem(hDlg, IDC_CONV_FOA), false);
    }
    else
        throw logic_error("None of Mode is selected");
}

void gui_ConverterCalculate(HWND hDlg, PEFile* pe)
{
    DWORD input, result;
    const int bufferSize = 0x30;
    TCHAR buffer[bufferSize];

    if (pe == NULL || !pe->isImage())
        return;
    
    if (IsDlgButtonChecked(hDlg, IDC_CONV_MRVA) == BST_CHECKED)
    {
        GetWindowText(GetDlgItem(hDlg, IDC_CONV_RVA), buffer, bufferSize);
        input = (DWORD)_tcstoll(buffer, NULL, 16);
        result = pe->RVAtoFOA(input);
        if (result == OUTRANGE)
            SetWindowText(GetDlgItem(hDlg, IDC_CONV_FOA), TEXT("Out Range"));
        else
        {
            StringCbPrintf(buffer, bufferSize, TEXT("%0.8X"), result);
            SetWindowText(GetDlgItem(hDlg, IDC_CONV_FOA), buffer);
        }

    }
    else if (IsDlgButtonChecked(hDlg, IDC_CONV_MFOA) == BST_CHECKED)
    {
        GetWindowText(GetDlgItem(hDlg, IDC_CONV_FOA), buffer, bufferSize);
        input = (DWORD)_tcstoll(buffer, NULL, 16);
        result = pe->FOAtoRVA(input);
        if (result == OUTRANGE)
            SetWindowText(GetDlgItem(hDlg, IDC_CONV_RVA), TEXT("Out Range"));
        else
        {
            StringCbPrintf(buffer, bufferSize, TEXT("%0.8X"), result);
            SetWindowText(GetDlgItem(hDlg, IDC_CONV_RVA), buffer);
        }
    }
    else
        throw logic_error("None of Mode is selected");

}

void gui_initializeCombineSection(HWND hDlg, PEFile* pe)
{
    if (pe == NULL || pe->isImage() == false)
        return;

    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];

    IMAGE_FILE_HEADER tempPE = pe->getFileHeader();
    IMAGE_SECTION_HEADER tempSec;

    for (int i = 0; i < tempPE.NumberOfSections; i++)
    {
        tempSec = pe->getSecHeader(i);
        mbstowcs(buffer, (char*)tempSec.Name, bufferSize);
        buffer[8] = L'\0';
        ComboBox_AddString(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO1), buffer);
        ComboBox_AddString(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO2), buffer);
    }

}

bool gui_toolSectionCombine(HWND hDlg, PEFile* pe)
{
    if (pe == NULL || pe->isImage() == false)
    {
        SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_PE);
        return false;
    } 

    int index1, index2;
    // get selected sections from section1 and section2
    index1 = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO1));
    index2 = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO2));

    // combine sections
    if (pe->combineSection(index1, index2) == false)
    {
        // get error code
        DWORD error = pe->getErrorCode();

        if ((error | EMPTY_IMAGEBUFFER) == EMPTY_IMAGEBUFFER)
            SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_PE);
        else if ((error | ERROR_COMSEC_INDEX) == ERROR_COMSEC_INDEX)
            SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)COMSEC_INDEXERROR);

        return false;
    }
    
    ComboBox_DeleteString(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO1), index1 < index2 ? index2 : index1);
    ComboBox_DeleteString(GetDlgItem(hDlg, IDC_TOOL_SEC_COMBO2), index1 < index2 ? index2 : index1);
    SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)SUCCEED);
    return true;
}

bool gui_toolSectionAdd(HWND hDlg, PEFile* pe)
{
    if (pe == NULL || pe->isImage() == false)
    {
        SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_PE);
        return false;
    }

    IMAGE_SECTION_HEADER tempSec = { 0 };
    const int bufferSize = 0x50;
    TCHAR buffer[bufferSize];
    const int bufferByte = sizeof(TCHAR) * bufferSize;

    // name
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_NAME), buffer, bufferSize);
    if (_tcslen(buffer) != 0)
        for (int i = 0; i < 8; i++)
        {
            tempSec.Name[i] = (byte)buffer[i];
            if (tempSec.Name[i] == '\0')
                break;
        }
    else
        sprintf((char*)tempSec.Name, "new");
    
    // virtual size
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_VSIZE), buffer, bufferSize);
    tempSec.Misc.VirtualSize = (DWORD)_tcstoll(buffer, NULL, 16);

    // size of raw data
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_FSIZE), buffer, bufferSize);
    tempSec.SizeOfRawData = (DWORD)_tcstoll(buffer, NULL, 16);

    // pointer to relocation
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_PRELOC), buffer, bufferSize);
    tempSec.PointerToRelocations = (DWORD)_tcstoll(buffer, NULL, 16);

    // number of relocation
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_RELOCNUM), buffer, bufferSize);
    tempSec.NumberOfRelocations = (WORD)_tcstoll(buffer, NULL, 16);

    // characteristics
    GetWindowText(GetDlgItem(hDlg, IDC_TOOL_SEC_CHARA), buffer, bufferSize);
    tempSec.Characteristics = (DWORD)_tcstoll(buffer, NULL, 16);
      
    if (pe->addSection(tempSec) == false)
    {
        SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)FAIL_ADDSECTION);
        return false;
    }
    
    SendMessage(GetDlgItem(hDlg, IDC_TOOL_SEC_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)SUCCEED);
    return true;
}
