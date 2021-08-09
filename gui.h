#pragma once

#include "resource.h"
#include "PEFile.h"
#include "framework.h"




extern HINSTANCE hInst;

HWND gui_AddToolBar(HWND hWndParent);
wstring gui_OpenFile(HWND hwnd);

// edit menu functions
bool gui_DisplayDos(HWND hDlg, PEFile* pe);
bool gui_SaveDos(HWND hDlg, PEFile* pe);
bool gui_DisplayPEHeader(HWND hDlg, PEFile* pe);
bool gui_SavePE(HWND hDlg, PEFile* pe);
bool gui_DisplayOptionalHeader(HWND hDlg, PEFile* pe);
bool gui_SaveOptional(HWND hDlg, PEFile* pe);

bool InitSectionListView(HWND hDlg, PEFile* pe);
void gui_showSectAttribute(HWND hDlg);
bool gui_DisplayDirectory(HWND hDlg, PEFile* pe);

bool gui_DisplayExport(HWND hDlg, PEFile* pe);
bool InitExportList(HWND hDlg, PEFile* pe);
bool InitImportList(HWND hDlg, PEFile* pe);
bool gui_DisplayImportFuncList(HWND hDlg, PEFile* pe);

// RVA and FOA converter 
void gui_setConverterEdit(HWND hDlg);
void gui_ConverterCalculate(HWND hDlg, PEFile* pe);

// Modify Section
void gui_initializeCombineSection(HWND hDlg, PEFile* pe);
bool gui_toolSectionCombine(HWND hDlg, PEFile* pe);
bool gui_toolSectionAdd(HWND hDlg, PEFile* pe);



// openprocess
bool gui_InitProcessList(HWND hDlg);
bool gui_DisplayModuleList(HWND hDlg, PEFile* pe);
bool gui_GetModuleInfo(HWND hDlg, PDWORD pPID, PDWORD64 pImageBase, PDWORD pImageSize);

