// PETool.cpp : Defines the entry point for the application.
//


#include "gui.h"
#include "injection.h"

#define MAX_LOADSTRING 100

extern HIMAGELIST g_hImageList;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
PEFile g_pe;                                     // PE object 

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM); 
INT_PTR CALLBACK    File_OpenProcess(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_Dos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_PE(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_Opt(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_Section(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_Directory(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    Edit_Dir_Export(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit_Dir_Import(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    Tool_Converter(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Tool_Section(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Tool_RemoteInj(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PETOOL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PETOOL));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    _CrtDumpMemoryLeaks();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_KEY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PETOOL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_KEY));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT rect;
   int width = 800;
   int height = 500;

   GetClientRect(GetDesktopWindow(), &rect);
   rect.left = (rect.right / 2) - (width / 2);
   rect.top = (rect.bottom / 2) - (height / 2);


   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       rect.left, rect.top, width, height, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // local variables
    HWND hWndToolBar = NULL;
    wstring fileName;


    // switch
    switch (message)
    {
    case WM_CREATE:
       
        hWndToolBar = gui_AddToolBar(hWnd);
        
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_FILE_OPEN:
            case IDB_TOOLBAR_OPEN:
                fileName = gui_OpenFile(hWnd);
                if (fileName == L"")
                    break;

                if (g_pe.readFile((WCHAR*)fileName.c_str()) == false)
                //    MessageBoxW(hWnd, g_pe->getErrorCode(), L"Error", MB_OK);

                break;
            case IDM_FILE_SAVE:
            case IDB_TOOLBAR_SAVE:
                if (g_pe.getInputType() == UNDEFINED)
                    MessageBox(NULL, L"FILE NOT FOUND", L"ERROR", MB_OK);
                else
                    g_pe.saveFile();
                break;

            case IDM_PROCESS_OPEN:
            case IDB_TOOLBAR_PRO:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_FILE_PROCESS), hWnd, File_OpenProcess);
                //g_pe->openProcess(13496);
                break;

            case IDM_EDIT_DOS:
            case IDB_TOOLBAR_DOS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DOS), hWnd, Edit_Dos);
                break;

            case IDM_EDIT_FILE:
            case IDB_TOOLBAR_PE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_FILE), hWnd, Edit_PE);
                break;

            case IDM_EDIT_OPTIONAL:
            case IDB_TOOLBAR_OPT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_OPT), hWnd, Edit_Opt);
                break;

            case IDM_EDIT_SECTION:
            case IDB_TOOLBAR_SEC:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_SECTION), hWnd, Edit_Section);
                break;

            case IDM_EDIT_DIRECTORY:
            case IDB_TOOLBAR_DIR:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIR), hWnd, Edit_Directory);
                break;

            case IDM_EDIT_IMPORTDIRECTORY:
            case IDB_TOOLBAR_IM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIR_IMPORT), hWnd, Edit_Dir_Import);
                break;

            case IDM_EDIT_EXPORTDIRECTORY:
            case IDB_TOOLBAR_EX:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIR_EXPORT), hWnd, Edit_Dir_Export);
                break;

            case IDM_TOOL_CONVERTER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_TOOL_CONVERTER), hWnd, Tool_Converter);
                break;

            case IDM_TOOL_SECTION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_TOOL_SECTION), hWnd, Tool_Section);
                break;

            case IDM_TOOL_REMOTEINJ:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_TOOL_REMOTEINJ), hWnd, Tool_RemoteInj);
                break;
                
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        ImageList_Destroy(g_hImageList);
        g_pe.reset();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for Import Table
INT_PTR CALLBACK File_OpenProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD pid = 0;
    static DWORD64 imageBase = 0;
    static DWORD imageSize = 0;

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        gui_InitProcessList(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_FILE_OPENPRO_OPEN:
            if (g_pe.openProcess(pid, imageBase, imageSize) == false)
                MessageBox(hDlg, TEXT("Access Denied! Try \"run as administrator\""), TEXT("Error"), MB_OK);
            else 
                EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }
    case WM_NOTIFY:       
        NMHDR* nmhdr = (NMHDR*)lParam;
        if (nmhdr->idFrom == IDC_FILE_OPENPRO_PRO && nmhdr->code == NM_CLICK)
            gui_DisplayModuleList(hDlg, &g_pe);
        else if (nmhdr->idFrom == IDC_FILE_OPENPRO_MOD && nmhdr->code == NM_CLICK)
            gui_GetModuleInfo(hDlg, &pid, &imageBase, &imageSize);
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

// Message handler for about Dialog.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Dos Dialog
INT_PTR CALLBACK Edit_Dos(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (g_pe.getInputType() != FILE)
            EnableWindow(GetDlgItem(hDlg, IDB_DOS_SAVE), false);
        gui_DisplayDos(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_DOS_REFRESH:
            gui_DisplayDos(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDB_DOS_SAVE:
            gui_SaveDos(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        
        } 
        
    }
    return (INT_PTR)FALSE;
}

// Message handler for PE file Dialog
INT_PTR CALLBACK Edit_PE(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (g_pe.getInputType() != FILE)
            EnableWindow(GetDlgItem(hDlg, IDB_PE_SAVE), false);
        gui_DisplayPEHeader(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_PE_REFRESH:
            gui_DisplayPEHeader(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDB_PE_SAVE:
            gui_SavePE(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }
       
    }
    return (INT_PTR)FALSE;
}

// Message handler for Optional Header Dialog
INT_PTR CALLBACK Edit_Opt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (g_pe.getInputType() != FILE)
            EnableWindow(GetDlgItem(hDlg, IDB_OPT_SAVE), false);
        gui_DisplayOptionalHeader(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_OPT_REFRESH:
            gui_DisplayOptionalHeader(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDB_OPT_SAVE:
            gui_SaveOptional(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }

    }
    return (INT_PTR)FALSE;
}

// Message handler for Section Header Dialog
INT_PTR CALLBACK Edit_Section(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        InitSectionListView(hDlg, &g_pe);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    case WM_NOTIFY:
        NMHDR* nmhdr = (NMHDR*)lParam;
        if (nmhdr->idFrom == IDC_SEC_LIST && nmhdr->code == NM_CLICK)
            gui_showSectAttribute(hDlg);      
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Directory Dialog
INT_PTR CALLBACK Edit_Directory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        gui_DisplayDirectory(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_EDIT_DIR_EX:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIR_EXPORT), hDlg, Edit_Dir_Export);
            break;
        case IDB_EDIT_DIR_IM:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_EDIT_DIR_IMPORT), hDlg, Edit_Dir_Import);            
            break;
        case IDB_EDIT_DIR_RES:
            break;
        case IDB_EDIT_DIR_REL:
            break;
        case IDB_EDIT_DIR_DEB:
            break;
        case IDB_EDIT_DIR_IAT:
            break;
        case IDB_EDIT_DIR_COM:
            break;

        case IDB_EDIT_DIR_REFRESH:
            gui_DisplayDirectory(hDlg, &g_pe);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }

    }
    return (INT_PTR)FALSE;
}

// Message handler for Export Table
INT_PTR CALLBACK Edit_Dir_Export(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        gui_DisplayExport(hDlg, &g_pe);
        InitExportList(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }

    }
    return (INT_PTR)FALSE;
}

// Message handler for Import Table
INT_PTR CALLBACK Edit_Dir_Import(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        InitImportList(hDlg, &g_pe);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }
    case WM_NOTIFY:
        NMHDR* nmhdr = (NMHDR*)lParam;
        if (nmhdr->idFrom == IDC_EDIT_DIR_IMPORT_DLL && nmhdr->code == NM_CLICK)
            gui_DisplayImportFuncList(hDlg, &g_pe);
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Address Converter
INT_PTR CALLBACK Tool_Converter(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_CONV_MRVA, IDC_CONV_MFOA, IDC_CONV_MRVA);    // default radio button
        gui_setConverterEdit(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CONV_MRVA:
            gui_setConverterEdit(hDlg);
            return (INT_PTR)TRUE;
        case IDC_CONV_MFOA:
            gui_setConverterEdit(hDlg);
            return (INT_PTR)TRUE;
        case IDB_CONV_CALC:
            gui_ConverterCalculate(hDlg, &g_pe);
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        }
    case WM_NOTIFY:
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Section Tool
INT_PTR CALLBACK Tool_Section(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        gui_initializeCombineSection(hDlg, &g_pe);
        // create a status bar
        CreateWindowEx(
            0,                          // no extended styles
            STATUSCLASSNAME,            // name of status bar class
            (PCTSTR)NULL,               // no text when first created
            WS_CHILD | WS_VISIBLE,      // creates a visible child window
            0, 0, 0, 0,                 // ignores size and position
            hDlg,                       // handle to parent window
            (HMENU)IDC_TOOL_SEC_BAR,    // child window identifier
            hInst,                      // handle to application instance
            NULL);                      // no window creation data
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_TOOL_SEC_COMBINE:
            gui_toolSectionCombine(hDlg, &g_pe);
            return (INT_PTR)TRUE;
        case IDB_TOOL_SEC_ADD:
            gui_toolSectionAdd(hDlg, &g_pe);
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
   
    }
    return (INT_PTR)FALSE;
}

// Message handler for Section Tool
INT_PTR CALLBACK Tool_RemoteInj(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR buffer[MAX_PATH];
    DWORD pid = 0;
    int res = 0;

    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        // create a status bar
        CreateWindowEx(
            0,                          // no extended styles
            STATUSCLASSNAME,            // name of status bar class
            (PCTSTR)NULL,               // no text when first created
            WS_CHILD | WS_VISIBLE,      // creates a visible child window
            0, 0, 0, 0,                 // ignores size and position
            hDlg,                       // handle to parent window
            (HMENU)IDC_TOOL_REMOTE_BAR, // child window identifier
            hInst,                      // handle to application instance
            NULL);                      // no window creation data
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_TOOL_REMOTE_INJ:
            GetWindowText(GetDlgItem(hDlg, IDC_TOOL_REMOTE_PID), buffer, MAX_PATH);
            pid = _tstoi(buffer);
            GetWindowText(GetDlgItem(hDlg, IDC_TOOL_REMOTE_DLLNAME), buffer, MAX_PATH);
            res = RemoteInjection(pid, buffer, _tcslen(buffer) + 1);
            switch (res)
            {
            case 0:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)SUCCEED);
                break;
            case 1:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_ARGUMENT);
                break;
            case 2:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_DLLNAME);
                break;
            case 3:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_OPEN_PROCESS);
                break;
            case 4:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_READ_MEMORY);
                break;
            case 5:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_WRITE_MEMORY);
                break;
            case 6:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_REMOTE_THREAD);
                break;
            case 7:
                SendMessage(GetDlgItem(hDlg, IDC_TOOL_REMOTE_BAR), SB_SETTEXT, SB_SIMPLEID, (LPARAM)INVALID_COMPATIBILITY);
                break;
            }
            return (INT_PTR)TRUE;
            
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

    }
    return (INT_PTR)FALSE;
}