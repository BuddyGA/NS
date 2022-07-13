#include "nsPlatform.h"



static wchar_t PlatformDirectoryPath[NS_PLATFORM_MAX_PATH];



// ============================================================================================================================================ //
// CRITICAL SECTION
// ============================================================================================================================================ //
nsCriticalSection::nsCriticalSection() noexcept
{
	InitializeCriticalSection(&CS);
}


nsCriticalSection::~nsCriticalSection() noexcept
{
	DeleteCriticalSection(&CS);
}


void nsCriticalSection::Enter() noexcept
{
	EnterCriticalSection(&CS);
}


bool nsCriticalSection::TryEnter() noexcept
{
	return TryEnterCriticalSection(&CS);
}


void nsCriticalSection::Leave() noexcept
{
	LeaveCriticalSection(&CS);
}




// ============================================================================================================================================ //
// ATOMIC
// ============================================================================================================================================ //
nsAtomic::nsAtomic() noexcept
	: Value(0)
{
}


int nsAtomic::Add(int add) noexcept
{
	return static_cast<int>(InterlockedAdd(reinterpret_cast<volatile LONG*>(&Value), static_cast<LONG>(add)));
}


int nsAtomic::Set(int newValue) noexcept
{
	return static_cast<int>(InterlockedExchange(reinterpret_cast<volatile LONG*>(&Value), static_cast<LONG>(newValue)));
}


int nsAtomic::Increment() noexcept
{
	return static_cast<int>(InterlockedIncrement(reinterpret_cast<volatile LONG*>(&Value)));
}

int nsAtomic::Decrement() noexcept
{
	return static_cast<int>(InterlockedDecrement(reinterpret_cast<volatile LONG*>(&Value)));
}


int nsAtomic::Get() const noexcept
{
	return Value;
}




// ============================================================================================================================================ //
// PLATFORM
// ============================================================================================================================================ //
void nsPlatform::Initialize() noexcept
{
	GetModuleFileName(NULL, PlatformDirectoryPath, NS_PLATFORM_MAX_PATH);

	// Get directory path
	{
		//String_Copy(PlatformDirectoryPath, argv[0]);
		int len = String_Length(PlatformDirectoryPath);
		int tokenIndex = -1;

		for (int i = len - 1; i >= 0; --i)
		{
			if (PlatformDirectoryPath[i] == '\\' || PlatformDirectoryPath[i] == '/')
			{
				tokenIndex = i;
				break;
			}
		}

		PlatformDirectoryPath[tokenIndex] = '\0';

		len = String_Length(PlatformDirectoryPath);

		for (int i = 0; i < len; ++i)
		{
			if (PlatformDirectoryPath[i] == '\\')
			{
				PlatformDirectoryPath[i] = '/';
			}
		}
	}

	ConsoleOutput(TEXT("Initialize platform [Windows]\n"));
}


void nsPlatform::Shutdown() noexcept
{
	
}


const wchar_t* nsPlatform::GetDirectoryPath() noexcept
{
	return PlatformDirectoryPath;
}


void nsPlatform::ConsoleOutput(const wchar_t* message, nsPlatformConsoleTextColorMasks colorMasks) noexcept
{
	const int len = String_Length(message);

	if (len == 0)
	{
		return;
	}

	OutputDebugString(message);

	HANDLE consoleOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD defaultColorAttribute = 0;

	if (consoleOutputHandle && consoleOutputHandle != INVALID_HANDLE_VALUE)
	{
		CONSOLE_SCREEN_BUFFER_INFO info{};
		GetConsoleScreenBufferInfo(consoleOutputHandle, &info);
		defaultColorAttribute = info.wAttributes;

		if (colorMasks != 0)
		{
			WORD colorWord = 0;

			if (colorMasks & nsEPlatformConsoleOutputColor::Red)
			{
				colorWord |= FOREGROUND_RED;
			}

			if (colorMasks & nsEPlatformConsoleOutputColor::Green)
			{
				colorWord |= FOREGROUND_GREEN;
			}

			if (colorMasks & nsEPlatformConsoleOutputColor::Blue)
			{
				colorWord |= FOREGROUND_BLUE;
			}

			SetConsoleTextAttribute(consoleOutputHandle, colorWord);
		}

		DWORD writtenCount = 0;
		WriteConsole(consoleOutputHandle, message, len, &writtenCount, NULL);

		/*
		if (colorMasks != 0)
		{
			SetConsoleTextAttribute(consoleOutputHandle, defaultColorAttribute);
		}
		*/
	}
}


void nsPlatform::ConsoleOutputFormat(nsPlatformConsoleTextColorMasks colorMasks, const wchar_t* format, ...) noexcept
{
	if (format == nullptr)
	{
		return;
	}

	wchar_t outputBuffer[NS_PLATFORM_OUTPUT_BUFFER_SIZE];

	va_list args;
	va_start(args, format);
	int charCount = vswprintf(outputBuffer, NS_PLATFORM_OUTPUT_BUFFER_SIZE, format, args);
	va_end(args);

	NS_Assert(charCount < NS_PLATFORM_OUTPUT_BUFFER_SIZE - 2);
	outputBuffer[charCount++] = '\n';
	outputBuffer[charCount++] = '\0';
	NS_Assert(charCount <= NS_PLATFORM_OUTPUT_BUFFER_SIZE);

	ConsoleOutput(outputBuffer, colorMasks);
}


void nsPlatform::Memory_Zero(void* dst, uint64 size) noexcept
{
	NS_Assert(dst);
	NS_Assert(size > 0);

	memset(dst, 0, size);
}


void nsPlatform::Memory_Set(void* dst, int value, uint64 size) noexcept
{
	NS_Assert(dst);
	NS_Assert(size > 0);

	memset(dst, value, size);
}


void nsPlatform::Memory_Copy(void* dst, const void* src, uint64 size) noexcept
{
	NS_Assert(dst);
	NS_Assert(src);
	NS_Assert(size > 0);

	memcpy(dst, src, size);
}


void nsPlatform::Memory_Move(void* dst, const void* src, uint64 size) noexcept
{
	NS_Assert(dst);
	NS_Assert(src);
	NS_Assert(size > 0);

	memmove(dst, src, size);
}


void* nsPlatform::Memory_Alloc(uint64 size) noexcept
{
	NS_Assert(size > 0);

	return malloc(size);
}


void* nsPlatform::Memory_Realloc(void* oldData, uint64 size) noexcept
{
	NS_Assert(size > 0);
	
	return realloc(oldData, size);
}


void nsPlatform::Memory_Free(void* data) noexcept
{
	NS_Assert(data);

	free(data);
}


int nsPlatform::String_Length(const char* cstr) noexcept
{
	if (cstr == nullptr)
	{
		return 0;
	}

	return static_cast<int>(strlen(cstr));
}


int nsPlatform::String_Length(const wchar_t* wstr) noexcept
{
	if (wstr == nullptr)
	{
		return 0;
	}

	return static_cast<int>(wcslen(wstr));
}


void nsPlatform::String_Copy(char* dst, const char* src) noexcept
{
	strcpy(dst, src);
}


void nsPlatform::String_Copy(wchar_t* dst, const wchar_t* src) noexcept
{
	wcscpy(dst, src);
}


int nsPlatform::String_Format(char* buffer, int bufferCount, const char* format, ...) noexcept
{
	if (buffer == nullptr || bufferCount <= 0 || format == nullptr)
	{
		return 0;
	}

	va_list args;
	va_start(args, format);
	const int n = vsnprintf(buffer, bufferCount, format, args);
	va_end(args);

	return n;
}


int nsPlatform::String_Format(wchar_t* buffer, int bufferCount, const wchar_t* format, ...) noexcept
{
	if (buffer == nullptr || bufferCount <= 0 || format == nullptr)
	{
		return 0;
	}

	va_list args;
	va_start(args, format);
	const int n = vswprintf(buffer, bufferCount, format, args);
	va_end(args);

	return n;
}


bool nsPlatform::String_Compare(const char* cstrA, const char* cstrB, bool bIgnoreCase) noexcept
{
	if (cstrA == cstrB)
	{
		return true;
	}

	const int lenA = String_Length(cstrA);
	const int lenB = String_Length(cstrB);
	
	if (lenA == 0 && lenB == 0)
	{
		return true;
	}

	if (lenA != lenB)
	{
		return false;
	}

	if (bIgnoreCase)
	{
		for (int i = 0; i < lenA; ++i)
		{
			if (tolower(cstrA[i]) != tolower(cstrB[i]))
			{
				return false;
			}
		}

		return true;
	}

	return strcmp(cstrA, cstrB) == 0;
}


bool nsPlatform::String_Compare(const wchar_t* wstrA, const wchar_t* wstrB, bool bIgnoreCase) noexcept
{
	if (wstrA == wstrB)
	{
		return true;
	}

	const int lenA = String_Length(wstrA);
	const int lenB = String_Length(wstrB);

	if (lenA == 0 && lenB == 0)
	{
		return true;
	}

	if (lenA != lenB)
	{
		return false;
	}

	bool bEquals = true;

	if (bIgnoreCase)
	{
		for (int i = 0; i < lenA; ++i)
		{
			if (towlower(wstrA[i]) != towlower(wstrB[i]))
			{
				bEquals = false;
				break;
			}
		}
	}
	else
	{
		bEquals = wcscmp(wstrA, wstrB) == 0;
	}

	return bEquals;
}


int nsPlatform::String_ConvertToWide(wchar_t* dst, const char* src, int length)
{
	return static_cast<int>(mbstowcs(dst, src, length));
}


int nsPlatform::String_ConvertToMultiByte(char* dst, const wchar_t* src, int length)
{
	return static_cast<int>(wcstombs(dst, src, length));
}


void nsPlatform::String_ToLower(char* cstr) noexcept
{
	const int len = String_Length(cstr);

	if (len == 0)
	{
		return;
	}

	for (int i = 0; i < len; ++i)
	{
		cstr[i] = tolower(cstr[i]);
	}
}


void nsPlatform::String_ToLower(wchar_t* wstr) noexcept
{
	const int len = String_Length(wstr);

	if (len == 0)
	{
		return;
	}

	for (int i = 0; i < len; ++i)
	{
		wstr[i] = towlower(wstr[i]);
	}
}


void nsPlatform::String_ToUpper(char* cstr) noexcept
{
	const int len = String_Length(cstr);

	if (len == 0)
	{
		return;
	}

	for (int i = 0; i < len; ++i)
	{
		cstr[i] = toupper(cstr[i]);
	}
}


void nsPlatform::String_ToUpper(wchar_t* wstr) noexcept
{
	const int len = String_Length(wstr);

	if (len == 0)
	{
		return;
	}

	for (int i = 0; i < len; ++i)
	{
		wstr[i] = towupper(wstr[i]);
	}
}


int nsPlatform::String_ToInt(const char* cstr) noexcept
{
	const int len = String_Length(cstr);

	if (len == 0)
	{
		return 0;
	}

	return atoi(cstr);
}


int nsPlatform::String_ToInt(const wchar_t* wstr) noexcept
{
	const int len = String_Length(wstr);

	if (len == 0)
	{
		return 0;
	}

	return wcstol(wstr, nullptr, 10);
}


float nsPlatform::String_ToFloat(const char* cstr) noexcept
{
	const int len = String_Length(cstr);

	if (len == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(atof(cstr));
}


float nsPlatform::String_ToFloat(const wchar_t* wstr) noexcept
{
	const int len = String_Length(wstr);

	if (len == 0)
	{
		return 0.0f;
	}

	return wcstof(wstr, nullptr);
}


uint64 nsPlatform::String_Hash(const char* cstr) noexcept
{
	const int length = String_Length(cstr);

	if (length == 0)
	{
		return 0;
	}

	uint64 hash = 0;

	for (int i = 0; i < length; ++i)
	{
		hash += cstr[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}


uint64 nsPlatform::String_Hash(const wchar_t* wstr) noexcept
{
	const int length = String_Length(wstr);

	if (length == 0)
	{
		return 0;
	}

	uint64 hash = 0;

	for (int i = 0; i < length; ++i)
	{
		hash += wstr[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}


bool nsPlatform::File_Exists(const wchar_t* filePath) noexcept
{
	const int len = String_Length(filePath);

	if (len == 0)
	{
		return false;
	}

	NS_Validate(len <= NS_PLATFORM_MAX_PATH);

	const DWORD dwAttrib = GetFileAttributes(filePath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


nsPlatformFileHandle nsPlatform::File_Open(const wchar_t* filePath, nsEPlatformFileOpenMode mode) noexcept
{
	const int len = String_Length(filePath);

	if (len == 0)
	{
		return NULL;
	}

	NS_Validate(len <= NS_PLATFORM_MAX_PATH);
	DWORD desiredAccess = 0;
	DWORD createDisposition = 0;

	if (mode == nsEPlatformFileOpenMode::READ)
	{
		desiredAccess = GENERIC_READ;
		createDisposition = OPEN_ALWAYS;

	}
	else if (mode == nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING)
	{
		desiredAccess = GENERIC_WRITE;
		createDisposition = CREATE_ALWAYS;
	}
	else // mode == nsEPlatformFileOpenMode::WRITE_APPEND
	{
		desiredAccess = FILE_APPEND_DATA;
		createDisposition = OPEN_ALWAYS;
	}

	nsPlatformFileHandle handle = CreateFile(filePath, desiredAccess, FILE_SHARE_READ, NULL, createDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	const DWORD error = GetLastError();

	if (handle == INVALID_HANDLE_VALUE)
	{
		if (error == ERROR_SHARING_VIOLATION)
		{
			ConsoleOutputFormat(nsEPlatformConsoleOutputColor::Red, TEXT("\nOpen file [%s] failed. Error sharing violation!\n"), filePath);
		}
		else if (error == ERROR_PATH_NOT_FOUND)
		{
			ConsoleOutputFormat(nsEPlatformConsoleOutputColor::Red, TEXT("\nOpen file [%s] failed. Error path not found!\n"), filePath);
		}
	}

	NS_ValidateV(handle && handle != INVALID_HANDLE_VALUE, TEXT("Fail to open file!"));

	return handle;
}


bool nsPlatform::File_Seek(nsPlatformFileHandle fileHandle, int byteOffset, nsEPlatformFileSeekMode mode) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		ConsoleOutput(TEXT("Fail to set file pointer (seek). Invalid file handle!\n"), nsEPlatformConsoleOutputColor::Red);
		return false;
	}

	return SetFilePointer(fileHandle, byteOffset, NULL, static_cast<DWORD>(mode)) != INVALID_SET_FILE_POINTER;
}


bool nsPlatform::File_Read(nsPlatformFileHandle fileHandle, void* outResult, int byteSize) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return ReadFile(fileHandle, outResult, byteSize, NULL, NULL);
}


bool nsPlatform::File_Write(nsPlatformFileHandle fileHandle, const void* data, int dataSize) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		ConsoleOutput(TEXT("Fail to write file. Invalid file handle!\n"), nsEPlatformConsoleOutputColor::Red);
		return false;
	}

	if (data == nullptr)
	{
		return false;
	}

	NS_Assert(dataSize > 0);

	return WriteFile(fileHandle, data, static_cast<DWORD>(dataSize), NULL, NULL);
}


void nsPlatform::File_Close(nsPlatformFileHandle& fileHandle) noexcept
{
	if (fileHandle && fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fileHandle);
		fileHandle = NULL;
	}
}


bool nsPlatform::File_Copy(const wchar_t* srcFilePath, const wchar_t* dstFilePath) noexcept
{
	const int srcLen = String_Length(srcFilePath);
	const int dstLen = String_Length(dstFilePath);
	NS_Assert(srcFilePath && srcLen > 0 && srcLen <= NS_PLATFORM_MAX_PATH);
	NS_Assert(dstFilePath && dstLen > 0 && dstLen <= NS_PLATFORM_MAX_PATH);

	return CopyFile(srcFilePath, dstFilePath, FALSE) == TRUE;
}


bool nsPlatform::File_Delete(const wchar_t* filePath) noexcept
{
	const int len = String_Length(filePath);
	
	if (len == 0)
	{
		return false;
	}

	NS_Validate(len <= NS_PLATFORM_MAX_PATH);

	return DeleteFile(filePath);
}


int nsPlatform::File_GetSize(nsPlatformFileHandle fileHandle) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		ConsoleOutput(TEXT("Fail to get file size. Invalid file handle!"), nsEPlatformConsoleOutputColor::Red);
		return 0;
	}

	return static_cast<int>(GetFileSize(fileHandle, NULL));
}


nsPlatformModuleHandle nsPlatform::Module_Load(const wchar_t* moduleFile) noexcept
{
	NS_Assert(moduleFile);

	return LoadLibrary(moduleFile);
}


void nsPlatform::Module_Unload(nsPlatformModuleHandle& moduleHandle) noexcept
{
	if (moduleHandle && moduleHandle != INVALID_HANDLE_VALUE)
	{
		FreeLibrary(moduleHandle);
		moduleHandle = NULL;
	}
}


void* nsPlatform::Module_GetFunction(nsPlatformModuleHandle moduleHandle, const char* functionName) noexcept
{
	NS_ValidateV(moduleHandle, TEXT("Invalid module handle!"));

	return GetProcAddress(moduleHandle, functionName);
}


int64 nsPlatform::PerformanceQuery_Frequency() noexcept
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	return frequency.QuadPart;
}


int64 nsPlatform::PerformanceQuery_Counter() noexcept
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	return counter.QuadPart;
}


void nsPlatform::Mouse_ShowCursor(bool bShow) noexcept
{
	ShowCursor(bShow);
}


void nsPlatform::Mouse_SetCursorShape(nsEMouseCursorShape shape) noexcept
{
	switch (shape)
	{
		case nsEMouseCursorShape::ARROW:		SetCursor(LoadCursor(NULL, IDC_ARROW)); break;
		case nsEMouseCursorShape::HAND:			SetCursor(LoadCursor(NULL, IDC_HAND)); break;
		case nsEMouseCursorShape::BEAM:			SetCursor(LoadCursor(NULL, IDC_IBEAM)); break;
		case nsEMouseCursorShape::SIZE_NS:		SetCursor(LoadCursor(NULL, IDC_SIZENS)); break;
		case nsEMouseCursorShape::SIZE_WE:		SetCursor(LoadCursor(NULL, IDC_SIZEWE)); break;
		case nsEMouseCursorShape::SIZE_NWSE:	SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); break;
		default: break;
	}
}


void nsPlatform::Mouse_SetCapture(nsPlatformWindowHandle windowHandle, bool bCapture) noexcept
{
	NS_Assert(windowHandle);

	if (bCapture)
	{
		SetCapture(windowHandle);
	}
	else
	{
		ReleaseCapture();
	}
}


void nsPlatform::Mouse_SetCursorWindowPosition(nsPlatformWindowHandle windowHandle, const nsPointInt& position) noexcept
{
	NS_Assert(windowHandle);

	POINT windowPosition;
	windowPosition.x = position.X;
	windowPosition.y = position.Y;
	ClientToScreen(windowHandle, &windowPosition);

	SetCursorPos(windowPosition.x, windowPosition.y);
}


void nsPlatform::Mouse_ClipCursor(bool bClip, nsPlatformWindowHandle windowHandle, const nsRectInt& rect) noexcept
{
	if (bClip)
	{
		NS_Assert(windowHandle);

		POINT p;
		p.x = rect.X;
		p.y = rect.Y;
		ClientToScreen(windowHandle, &p);

		RECT clip;
		clip.left = p.x;
		clip.top = p.y;
		clip.right = p.x + rect.Width;
		clip.bottom = p.y + rect.Height;
		ClipCursor(&clip);
	}
	else
	{
		ClipCursor(NULL);
	}
}


bool nsPlatform::Mouse_IsCursorHidden() noexcept
{
	CURSORINFO info{};
	info.cbSize = sizeof(CURSORINFO);

	if (GetCursorInfo(&info))
	{
		return (info.flags == 0);
	}

	return false;
}


void nsPlatform::Thread_Sleep(int ms) noexcept
{
	NS_Assert(ms >= 0);

	Sleep(static_cast<DWORD>(ms));
}
