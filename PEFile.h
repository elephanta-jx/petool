// PEFile.h: PEFile class
//
//////////////////////////////////////////////////////////////////////


#include "framework.h"

#define OUTRANGE -1

#define UNDEFINED 0
#define FILE 1
#define PROCESS 2


using namespace std;

///////////////////////// error_code ////////////////////////////
#define ERROR_DOS_SIGNATURE	0x00000001
#define ERROR_PE_SIGNATURE	0x00000002

#define ERROR_FILEOPEN		0x00000010
#define ERROR_FILETOIMAGE	0x00000020
#define ERROR_ASSIGNHEADER	0x00000040
#define ERROR_IMAGETOFILE	0x00000080

#define EMPTY_FILEBUFFER	0x00000100
#define EMPTY_IMAGEBUFFER	0x00000200
#define EMPTY_HEADER		0x00000400

#define ERROR_COMSEC_INDEX	0x00001000
#define ERROR_ADDSEC_FULL	0x00002000
#define ERROR_ADDSEC_ARGS	0x00004000
#define ADD_SECTION_FAIL	0x00002000

#define EMPTY_EXPORT		0x00010000
#define EMPTY_RELOCATION	0x00020000

#define ERROR_BIT			0x00100000

struct ExportFunction
{
	DWORD address = 0;
	string name;
	int ordinal = 0;
};


class PEFile
{
public:

	PEFile();
	~PEFile();

	// check
	bool isExport();
	bool isImport();
	bool isImage();

	//print functions
	bool Print();

	//file processing 
	bool readFile(wstring fName);
	bool saveFile();

	bool openProcess(DWORD PID, DWORD64 imageBase, DWORD imageSize);

	// getter
	IMAGE_DOS_HEADER getDosHeader();
	IMAGE_FILE_HEADER getFileHeader();
	IMAGE_SECTION_HEADER getSecHeader(int n);
	IMAGE_DATA_DIRECTORY getDirectory(int n);
	IMAGE_EXPORT_DIRECTORY getExport();
	int getOptionalHeader(OUT void** optionalHeader);
	int getImport(OUT IMAGE_IMPORT_DESCRIPTOR** importDescriptor);
	int getExportFunctions(OUT ExportFunction** pExportFunc);

	int getBit();
	int getInputType();
	string getString(DWORD RVA);
	int getImportLookupThunk(OUT void** lookup, IN DWORD lookupRVA);
	int getHint(IN DWORD RVA);

	DWORD getErrorCode();

	// setter
	void setDosHeader(PIMAGE_DOS_HEADER);
	void setPEHeader(PIMAGE_FILE_HEADER);
	void setOptionalHeader(void*);

	void reset();

	// section manipulation
	bool addSection(IN IMAGE_SECTION_HEADER newSection);
	//add section£¬
	//secName: the name of the new section (8 characters maximum)
	//secSize: the size of the new section	
	
	bool combineSection(IN int first, IN int second);
	//combine two sections. the arguments are section indexes that must be in the range from 0 to (number of sections - 1) 


	BOOL insertCode(IN PBYTE fileBuf, IN PBYTE codeBuf, IN DWORD lengthOfCode);
	//insert a piece of codes into the target file

	//fileBuf: a pointer to file
	//codeBuf: a pointer to code
	//lengthOfCode: the length of code
	//return value£ºtrue for success, false for failure
	

//////////////////////////////////////////////////////////////////////////////////////////
	BOOL SetRelocation(IN DWORD NewImageBase);

	BOOL MoveExportTable(IN PVOID);
	BOOL MoveImportTableDll(IN PVOID pDes);
	BOOL FixImportTableDll();
	BOOL AddImportTableDll(PTCHAR DllName, PTCHAR FuncName);
	PVOID FindEmptySpace(IN DWORD dwSize);

//////////////////////////////////////////////////////////////////////////////////////////
	//file/memory address convertion
	int RVAtoFOA(DWORD VAddress);
	int FOAtoRVA(DWORD FAddress);
	

// private variables
private:
	// file buffer
	byte* pImageBuffer;
	byte* pFileBuffer;

	wstring fileName;
	unsigned int fileSize;
	int bit;
	int inputType;

	// PE header pointers
	PIMAGE_DOS_HEADER				pDos;			//DOS header
	PIMAGE_FILE_HEADER				pPE;			//File header
	PIMAGE_OPTIONAL_HEADER32		pOptional32;	//Optional header 32-bit
	PIMAGE_OPTIONAL_HEADER64		pOptional64;	//Optional header 64-bit
	PIMAGE_SECTION_HEADER			pSection;		//section table
	PIMAGE_DATA_DIRECTORY			pDirectory;		//directory

	PIMAGE_EXPORT_DIRECTORY			pExport;		//export directory 
	PIMAGE_IMPORT_DESCRIPTOR		pImport;		//import directory
	PIMAGE_BASE_RELOCATION			pRelocation;	//relocation directory
	PIMAGE_BOUND_IMPORT_DESCRIPTOR	pBoundImport;	//bound import directory
	PIMAGE_RESOURCE_DIRECTORY		pResource;		//resource directory

	// error handling
	DWORD error_Flag; 
	
// private methods
private:
	bool assignHeader();

	//buffer convertion
	bool Convert_FileToImage();
	bool Convert_ImageToFile();

};
