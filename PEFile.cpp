#include "PEFile.h"


PEFile::PEFile()
{
	pDos = NULL;
	pPE = NULL;
	pOptional32 = NULL;
	pOptional64 = NULL;
	pSection = NULL;
	pDirectory = NULL;

	pExport = NULL;
	pImport = NULL;
	pRelocation = NULL;
	pBoundImport = NULL;
	pResource = NULL;

	error_Flag = 0;
	bit = 0;
	inputType = UNDEFINED;
	fileSize = 0;

	// file and image buffer
	pImageBuffer = NULL;
	pFileBuffer = NULL;
	
}

PEFile::~PEFile()
{
	reset();
}

bool PEFile::readFile(wstring fName)
// open file and copy to the buffer
// then convert it from file buffer to image buffer
{
	ifstream inputFile;

	byte* current;
	byte temp;

	inputFile.open(fName, ios::binary);
	if (!inputFile.is_open())
	{
		error_Flag |= ERROR_FILEOPEN;
		return false;
	}

	reset();
	fileName = fName;

	inputFile.seekg(0, ios::end);
	fileSize = (int)inputFile.tellg();	// get the file size
	inputFile.seekg(0, ios::beg);

	if (pFileBuffer != NULL)			// if file buffer is reused
		delete[] pFileBuffer;

	pFileBuffer = new byte[fileSize];	//allocate file buffer 

	current = pFileBuffer;
	temp = inputFile.get();
	while (inputFile.good())			// get the contents of the file
	{
		*current = temp;
		current++;
		temp = inputFile.get();
	}

	// convert file to image 
	if (Convert_FileToImage() == false)
	{
		error_Flag |= ERROR_FILETOIMAGE;
		reset();
		inputFile.close();
		return false;
	}
	// assign headers
	if (assignHeader() == false)
	{
		error_Flag |= ERROR_ASSIGNHEADER;
		reset();
		inputFile.close();
		return false;
	}
	inputType = FILE;
	inputFile.close();

	return true;
}


bool PEFile::saveFile()
{
	if (inputType != FILE)
		return false;

	ofstream outputFile;

	// open the output file
	outputFile.open(fileName, ofstream::out | ofstream::binary);
	if (outputFile.is_open() == false)
	{
		error_Flag |= ERROR_FILEOPEN;
		return false;
	}
	// convert image to file
	if (Convert_ImageToFile() == false)
	{
		error_Flag |= ERROR_IMAGETOFILE;
		outputFile.close();
		return false;
	}
	// writing to the file		
	outputFile.write((char*)pFileBuffer, fileSize);
	outputFile.flush();
	outputFile.close();

	return true;
}

bool PEFile::openProcess(DWORD PID, DWORD64 imageBase, DWORD imageSize)
{
	if (PID == 0 || imageBase == 0 || imageSize == 0)
		return false;

	HANDLE hPro = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, PID);

	if (hPro == NULL)
		return false;

	// reset internal buffer
	reset();
	pImageBuffer = new byte[imageSize];
	memset(pImageBuffer, 0, imageSize);

	// read memory
	if (ReadProcessMemory(hPro, (LPCVOID)imageBase, pImageBuffer, imageSize, NULL) == false)
		return false;
	if (assignHeader() == false)
		return false;
	inputType = PROCESS;

	return true;
}

bool PEFile::assignHeader()
{
	byte* pCurrent = pImageBuffer;

	pDos = (PIMAGE_DOS_HEADER)pCurrent;			// assign Dos pointer 
	pCurrent += pDos->e_lfanew;					// move to NT header

	pCurrent += 4;								// move to PE pointer
	pPE = (PIMAGE_FILE_HEADER)pCurrent;			// assign PE pointer 

	pCurrent += sizeof(IMAGE_FILE_HEADER);		// move to PE optional header
	
	if (*(PWORD)pCurrent == 0x10B)				// determine bit 
		bit = 32;
	else if (*(PWORD)pCurrent == 0x20B)
		bit = 64;
	else	
		return false;
	
	if (bit == 64)								// assign the PE optional header and the directory pointer 
	{
		pOptional64 = (PIMAGE_OPTIONAL_HEADER64)pCurrent;
		pDirectory = pOptional64->DataDirectory;
	}	
	else if (bit == 32)
	{
		pOptional32 = (PIMAGE_OPTIONAL_HEADER32)pCurrent;
		pDirectory = pOptional32->DataDirectory;
	}
				
	pCurrent += pPE->SizeOfOptionalHeader;			// move to section header
	pSection = (PIMAGE_SECTION_HEADER)pCurrent;		// assgin Section pointer 

	// assign directory tables
	if (pDirectory[0].VirtualAddress)				 
		pExport = (PIMAGE_EXPORT_DIRECTORY)(pImageBuffer + pDirectory[0].VirtualAddress);	// assign export directory pointer 
	
	if (pDirectory[1].VirtualAddress)
		pImport = (PIMAGE_IMPORT_DESCRIPTOR)(pImageBuffer + pDirectory[1].VirtualAddress);	// assign import directory pointer 
	
	if (pDirectory[5].VirtualAddress)
		pRelocation = (PIMAGE_BASE_RELOCATION)(pImageBuffer + pDirectory[5].VirtualAddress);	// assign relocation directory pointer 
	
	if (pDirectory[11].VirtualAddress)
		pBoundImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)(pImageBuffer + pDirectory[11].VirtualAddress);	// assign bound import directory pointer 
	
	if (pDirectory[2].VirtualAddress)
		pResource = (PIMAGE_RESOURCE_DIRECTORY)(pImageBuffer + pDirectory[2].VirtualAddress);	// assign resource directory pointer 
	
	return true;
}


int PEFile::RVAtoFOA(DWORD vAddress)
{
	if (isImage() == false)
	{
		error_Flag |= EMPTY_IMAGEBUFFER;
		return false;
	}

	if ((bit == 32 && vAddress < pOptional32->SizeOfHeaders) || (bit == 64 && vAddress < pOptional64->SizeOfHeaders))					// when vAddress is in the range of header
		return vAddress;				// RVA = FOA in this case

	for (int i = 0; i < pPE->NumberOfSections; i++)
	{
		if (pSection[i].SizeOfRawData == 0)		// the section does not take disk space
			continue;
		else if (vAddress < pSection[i].VirtualAddress)		
			return OUTRANGE;
		else if (vAddress >= pSection[i].VirtualAddress && vAddress < pSection[i].VirtualAddress + pSection[i].SizeOfRawData)	
			return vAddress - pSection[i].VirtualAddress + pSection[i].PointerToRawData;
	}
	return OUTRANGE;
}

int PEFile::FOAtoRVA(DWORD FileOffset)
{
	if (isImage() == false)
	{
		error_Flag |= EMPTY_IMAGEBUFFER;
		return false;
	}

	if ((bit == 32 && FileOffset < pOptional32->SizeOfHeaders) || (bit == 64 && FileOffset < pOptional64->SizeOfHeaders))				// when file address is in the range of header
		return FileOffset;				// RVA = FOA in this case

	for (int i = 0; i < pPE->NumberOfSections; i++)			
	{
		if (pSection[i].SizeOfRawData == 0)		// the section does not take disk space
			continue;
		else if (FileOffset < pSection[i].PointerToRawData)
			return OUTRANGE;
		else if (FileOffset >= pSection[i].PointerToRawData && FileOffset < pSection[i].PointerToRawData + pSection[i].SizeOfRawData)
			return FileOffset - pSection[i].PointerToRawData + pSection[i].VirtualAddress;
	}
	return OUTRANGE;
}


bool PEFile::Convert_FileToImage()
{
	if (pFileBuffer == NULL)
	{
		error_Flag = EMPTY_FILEBUFFER;
		return false;
	}

	byte* pRead = NULL;
	byte* pWrite = NULL;
	PIMAGE_FILE_HEADER pTempPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER32 pTempOptional = NULL;
	PIMAGE_SECTION_HEADER pTempSection = NULL;
	
	pRead = pFileBuffer;

	if (*((PWORD)pRead) != IMAGE_DOS_SIGNATURE)		// if the file doesn't have DOS signature
	{
		error_Flag = ERROR_DOS_SIGNATURE;
		return false;
	}
	pRead = pRead + ((PIMAGE_DOS_HEADER)pRead)->e_lfanew;				//move to NT header

	if (*((PDWORD)pRead) != IMAGE_NT_SIGNATURE)		// if the file does not have PE header
	{
		error_Flag = ERROR_PE_SIGNATURE;
		return false;
	}

	pRead += 4;											// move to PE header
	pTempPEHeader = (PIMAGE_FILE_HEADER)pRead;			// assign temporary PE header
	pRead += sizeof(IMAGE_FILE_HEADER);					// move to PE optional header
	pTempOptional = (PIMAGE_OPTIONAL_HEADER32)pRead;	// assign temporary optional header
	pRead += pTempPEHeader->SizeOfOptionalHeader;		// move to section header
	pTempSection = (PIMAGE_SECTION_HEADER)pRead;		// assign temporary section header

	pImageBuffer = new byte[pTempOptional->SizeOfImage];	// allocate memory for image buffer
	memset(pImageBuffer, 0, pTempOptional->SizeOfImage);	// initialize to 0

	// convert process
	pWrite = pImageBuffer;
	pRead = pFileBuffer;
	memcpy(pWrite, pRead, pTempOptional->SizeOfHeaders);	// copy the headers
	pRead = pRead + pTempOptional->SizeOfHeaders;			// move to the first section

	for (int i = 0; i < pTempPEHeader->NumberOfSections; i++)	// copy all sections
	{
		pRead = pFileBuffer + pTempSection[i].PointerToRawData;
		pWrite = pImageBuffer + pTempSection[i].VirtualAddress;

		if (pTempSection[i].VirtualAddress >= pTempOptional->SizeOfImage) // bound check
			break;
		if (pTempSection[i].VirtualAddress + pTempSection[i].SizeOfRawData > pTempOptional->SizeOfImage)	
			memcpy(pWrite, pRead, pTempOptional->SizeOfImage - pTempSection[i].VirtualAddress);		// copy as much as it can
		else
			memcpy(pWrite, pRead, pTempSection[i].SizeOfRawData);
	}
	return true;
}

bool PEFile::Convert_ImageToFile()
{
	byte* read = NULL;
	byte* write = NULL;

	if (pImageBuffer == NULL)
	{
		error_Flag |= EMPTY_IMAGEBUFFER;
		return false;
	}

	read = pImageBuffer;	// assign read pointer
	fileSize = pSection[pPE->NumberOfSections - 1].PointerToRawData + pSection[pPE->NumberOfSections - 1].SizeOfRawData;	// re-calculate file size

	if (pFileBuffer != NULL)		// reuse the file buffer
		delete[] pFileBuffer;
	pFileBuffer = new byte[fileSize];
	memset(pFileBuffer, 0, fileSize);		
	write = pFileBuffer;	// assign write pointer

	// converting process starts
	int sizeOfHeader;
	if (bit == 64)
		sizeOfHeader = pOptional64->SizeOfHeaders;
	else if (bit == 32)
		sizeOfHeader = pOptional32->SizeOfHeaders;
	else
		throw logic_error("Not 32-bit nor 64-bit in :Convert_ImageToFile()");

	memcpy(write, read, sizeOfHeader);	// copy the header

	for (int i = 0; i < pPE->NumberOfSections; i++)	// copy all sections
	{
		if (pSection[i].PointerToRawData >= fileSize)	// check bound
			break;

		read = (PBYTE)pImageBuffer + pSection[i].VirtualAddress;
		write = (PBYTE)pFileBuffer + pSection[i].PointerToRawData;
		
		if (pSection[i].PointerToRawData + pSection[i].SizeOfRawData > fileSize)
			memcpy(write, read, fileSize - pSection[i].SizeOfRawData);	// copy as much as it can
		else
			memcpy(write, read, pSection[i].SizeOfRawData);
	}

	return true;
}

bool PEFile::combineSection(IN int first, IN int second)
{
	
	// validation 
	if (pImageBuffer == NULL)
	{
		error_Flag |= EMPTY_IMAGEBUFFER;
		return false;
	}
	else if (first < 0 || first >= pPE->NumberOfSections || second < 0 || second >= pPE->NumberOfSections || abs(first - second) != 1)
	{
		error_Flag |= ERROR_COMSEC_INDEX;
		return false;
	}
	
	if (first > second)	// make sure first smaller than second
	{
		int temp = first;
		first = second;
		second = temp;
	}

	pSection[first].SizeOfRawData = pSection[second].PointerToRawData + pSection[second].SizeOfRawData - pSection[first].PointerToRawData;	
	pSection[first].Misc.VirtualSize = pSection[second].VirtualAddress + pSection[second].Misc.VirtualSize - pSection[first].VirtualAddress;
	pSection[first].Characteristics = pSection[first].Characteristics | pSection[second].Characteristics;

	// move all sections after second forward
	for (int i = 0; i < pPE->NumberOfSections - 1 - second; i++)
		pSection[second + i] = pSection[second + i + 1];

	pPE->NumberOfSections--;

	return true;
}


bool PEFile::addSection(IN IMAGE_SECTION_HEADER newSection)
{
	if (pImageBuffer == NULL)
	{
		error_Flag |= EMPTY_IMAGEBUFFER;
		return false;
	}
	if (bit != 32 && bit != 64)
	{
		error_Flag |= ERROR_BIT;
		return false;
	}

	// check for empty space
	if (bit == 32 && (DWORD)(pSection + pPE->NumberOfSections + 1) - (DWORD)pImageBuffer > pOptional32->SizeOfHeaders)
	{
		error_Flag |= ERROR_ADDSEC_FULL;
		return false;
	}
	else if (bit == 64 && (DWORD)(pSection + pPE->NumberOfSections + 1) - (DWORD)pImageBuffer > pOptional64->SizeOfHeaders)
	{
		error_Flag |= ERROR_ADDSEC_FULL;
		return false;
	}
	
	// check file alignment 
	if (bit == 32 && newSection.SizeOfRawData % pOptional32->FileAlignment != 0)
		newSection.SizeOfRawData = (newSection.SizeOfRawData / pOptional32->FileAlignment + 1) * pOptional32->FileAlignment;
	else if (bit == 64 && newSection.SizeOfRawData % pOptional64->FileAlignment != 0)
		newSection.SizeOfRawData = (newSection.SizeOfRawData / pOptional64->FileAlignment + 1) * pOptional64->FileAlignment;
	

	// assign virtual address & check size of image
	if (bit == 32)
	{
		if (pOptional32->SizeOfImage % pOptional32->SectionAlignment != 0)
			pOptional32->SizeOfImage = (pOptional32->SizeOfImage / pOptional32->SectionAlignment + 1) * pOptional32->SectionAlignment;
		newSection.VirtualAddress = pOptional32->SizeOfImage;
	}
	else if (bit == 64)
	{
		if (pOptional64->SizeOfImage % pOptional64->SectionAlignment != 0)
			pOptional64->SizeOfImage = (pOptional64->SizeOfImage / pOptional64->SectionAlignment + 1) * pOptional64->SectionAlignment;
		newSection.VirtualAddress = pOptional64->SizeOfImage;
	}
		
	// assign file address
	newSection.PointerToRawData = pSection[pPE->NumberOfSections - 1].PointerToRawData + pSection[pPE->NumberOfSections - 1].SizeOfRawData;		// the FOA + size of the last existing section

	// assign other values
	newSection.NumberOfLinenumbers = 0;
	newSection.PointerToLinenumbers = 0;
	
	// copy the section to the section header
	pSection[pPE->NumberOfSections] = newSection;

	// re-calculate size of image
	int addSize = newSection.SizeOfRawData;
	if (bit == 32)
	{
		if (addSize % pOptional32->SectionAlignment != 0)
			addSize = (addSize / pOptional32->SectionAlignment + 1) * pOptional32->SectionAlignment;
		pOptional32->SizeOfImage += addSize;
	}
	else if (bit == 64)
	{
		if (addSize % pOptional64->SectionAlignment != 0)
			addSize = (addSize / pOptional64->SectionAlignment + 1) * pOptional64->SectionAlignment;
		pOptional64->SizeOfImage += addSize;
	}
	
	// add 1 to number of sections
	pPE->NumberOfSections++;

	return true;
}

DWORD PEFile::getErrorCode()
{
	return error_Flag;
}


bool PEFile::isExport()
{
	return (pExport) ? true : false;
}

bool PEFile::isImport()
{
	return (pImport) ? true : false;
}

bool PEFile::isImage()
{
	return pImageBuffer;
}

IMAGE_DOS_HEADER PEFile::getDosHeader()
{
	if (pDos == NULL)
		return IMAGE_DOS_HEADER{ 0 };
	else
		return *pDos;
}

IMAGE_FILE_HEADER PEFile::getFileHeader()
{
	if (pPE == NULL)
		return IMAGE_FILE_HEADER{ 0 };
	else
		return *pPE;
}

int PEFile::getOptionalHeader(OUT void** optionalHeader)
{
	int optHeaderSize = pPE->SizeOfOptionalHeader;
	
	*optionalHeader = new byte[optHeaderSize];
	if (bit == 64)
		memcpy(*optionalHeader, pOptional64, optHeaderSize);
	else if (bit == 32)
		memcpy(*optionalHeader, pOptional32, optHeaderSize);
	
	return 0;
}

IMAGE_SECTION_HEADER PEFile::getSecHeader(int n)
{
	if (pSection != NULL && n >= 0 && n < pPE->NumberOfSections)
	{
		return *(pSection + n);
	}
	else
		return IMAGE_SECTION_HEADER{ 0 };
}

IMAGE_DATA_DIRECTORY PEFile::getDirectory(int n)
{
	int numDir = 0;

	if (bit == 64)
		numDir = pOptional64->NumberOfRvaAndSizes;
	else if (bit == 32)
		numDir = pOptional32->NumberOfRvaAndSizes;

	if (pDirectory != NULL && n >= 0 && n < numDir)
		return *(pDirectory + n);
	else
		return IMAGE_DATA_DIRECTORY{ 0 };
}

IMAGE_EXPORT_DIRECTORY PEFile::getExport()
{
	if (pExport == NULL)
		return IMAGE_EXPORT_DIRECTORY{ 0 };
	else
		return *pExport;
}

int PEFile::getImport(OUT IMAGE_IMPORT_DESCRIPTOR** importDescriptor)
{
	PIMAGE_IMPORT_DESCRIPTOR current = pImport;
	int numOfImport = 0;

	if (current == NULL)
	{
		*importDescriptor = NULL;
		return 0;
	}
		

	while (current->Name)	// get how many import dlls
	{
		current++;
		numOfImport++;
	}
		
	
	*importDescriptor = new IMAGE_IMPORT_DESCRIPTOR[numOfImport];
	memset(*importDescriptor, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR) * numOfImport);
	memcpy(*importDescriptor, pImport, sizeof(IMAGE_IMPORT_DESCRIPTOR) * numOfImport);

	return numOfImport;
}

int PEFile::getExportFunctions(OUT ExportFunction** pExportFunc)
{
	if (!isExport())
		return 0;
		
	int exportFuncSize = pExport->NumberOfFunctions;
	*pExportFunc = new ExportFunction[exportFuncSize];

	PDWORD pRVA = (PDWORD)(pImageBuffer + pExport->AddressOfFunctions);
	PDWORD pNameAddr = (PDWORD)(pImageBuffer + pExport->AddressOfNames);
	PWORD pOrdinalAddr = (PWORD)(pImageBuffer + pExport->AddressOfNameOrdinals);
		
	char* name = NULL;
	int numOfOrdinalFunc = pExport->NumberOfFunctions - pExport->NumberOfNames;

	// populate all export functions into the structure
	for (int i = 0; i < exportFuncSize; i++)
	{
		// extract functions that don't have a name
		if (i < numOfOrdinalFunc)
		{
			(*pExportFunc)[i].address = pRVA[i];
			(*pExportFunc)[i].ordinal = pExport->Base + i;
			(*pExportFunc)[i].name = "-";
		}
		// extract functions that have a name
		else
		{
			(*pExportFunc)[i].address = pRVA[i - numOfOrdinalFunc];
			(*pExportFunc)[i].ordinal = pExport->Base + pOrdinalAddr[i - numOfOrdinalFunc];
			name = (char*)(pImageBuffer + pNameAddr[i - numOfOrdinalFunc]);

			(*pExportFunc)[i].name = string(name);				
		}
			
	}

	return exportFuncSize;
}

int PEFile::getHint(IN DWORD RVA)
{
	return *(short*)((char*)pImageBuffer + RVA);
}

int PEFile::getImportLookupThunk(OUT void** lookup, IN DWORD lookupRVA)
{
	if (lookup == NULL || lookupRVA == 0)
		return 0;

	if (bit == 32)
	{
		PDWORD current = (PDWORD)(pImageBuffer + lookupRVA);
		int size = 0;
		while (*current != 0)
		{
			size++;
			current++;
		}
			

		*lookup = new DWORD[size];
		memcpy(*lookup, pImageBuffer + lookupRVA, size * 4);

		return size;
	}
	else if (bit == 64)
	{
		PDWORD64 current = (PDWORD64)(pImageBuffer + lookupRVA);
		int size = 0;
		while (*current != 0)
		{
			size++;
			current++;
		}

		*lookup = new DWORD64[size];
		memcpy(*lookup, pImageBuffer + lookupRVA, size * 8);

		return size;
	}
	else
		return 0;
}

int PEFile::getBit()
{
	return bit;
}

int PEFile::getInputType()
{
	return inputType;
}

string PEFile::getString(DWORD RVA)
{
	return string((char*)pImageBuffer + RVA);
}

void PEFile::setDosHeader(PIMAGE_DOS_HEADER pNewDos)
{
	*pDos = *pNewDos;
	
}

void PEFile::setPEHeader(PIMAGE_FILE_HEADER pNewPE)
{
	*pPE = *pNewPE;
}

void PEFile::setOptionalHeader(void* pNewOptional)
{
	if (bit == 32)
		memcpy(pOptional32, pNewOptional, pPE->SizeOfOptionalHeader - pOptional32->NumberOfRvaAndSizes * 8);
	else if (bit == 64)
		memcpy(pOptional64, pNewOptional, pPE->SizeOfOptionalHeader - pOptional64->NumberOfRvaAndSizes * 8);
}

void PEFile::reset()
{
	
	pDos = NULL;
	pPE = NULL;
	pOptional32 = NULL;
	pOptional64 = NULL;
	pSection = NULL;
	pDirectory = NULL;

	pExport = NULL;
	pImport = NULL;
	pRelocation = NULL;
	pBoundImport = NULL;
	pResource = NULL;

	bit = 0;
	inputType = 0;
	fileSize = 0;
	error_Flag = 0;

	// deallocate image buffer
	if (pFileBuffer != NULL)
		delete[] pFileBuffer;
	pFileBuffer = NULL;

	// deallocate file buffer
	if (pImageBuffer != NULL)
		delete[] pImageBuffer;
	pImageBuffer = NULL;

}
