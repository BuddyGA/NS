#pragma once

#include "nsCoreTypes.h"


#ifdef _WIN64
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#define NS_PLATFORM_WINDOWS
typedef HWND		nsWindowHandle;
typedef HMODULE		nsModuleHandle;
typedef HANDLE		nsFileHandle;

enum class nsEFileSeekMode : uint8
{
	BEGIN	= FILE_BEGIN,
	CURRENT	= FILE_CURRENT,
	END		= FILE_END
};

namespace nsEPlatformConsoleTextColor
{
	enum Mask
	{
		Blue	= FOREGROUND_BLUE,
		Green	= FOREGROUND_GREEN,
		Red		= FOREGROUND_RED,
	};
};

#define NS_Abort() abort()
#endif // _WIN64


typedef uint8 nsPlatformConsoleTextColorMasks;


enum class nsEPlatformFileOpenMode : uint8
{
	READ = 0,
	WRITE_OVERWRITE_EXISTING,
	WRITE_APPEND
};


#define NS_PLATFORM_OUTPUT_BUFFER_SIZE		(2048)
#define NS_PLATFORM_MAX_PATH				(255)



class NS_CORE_API nsCriticalSection
{
	NS_DECLARE_NOCOPY_NOMOVE(nsCriticalSection)

private:
#ifdef NS_PLATFORM_WINDOWS
	CRITICAL_SECTION CS;
#endif // NS_PLATFORM_WINDOWS


public:
	nsCriticalSection() noexcept;
	~nsCriticalSection() noexcept;
	void Enter() noexcept;
	bool TryEnter() noexcept;
	void Leave() noexcept;

};



class NS_CORE_API nsAtomic
{
private:
	volatile int Value;

public:
	nsAtomic() noexcept;
	int Add(int add) noexcept;
	int Set(int newValue) noexcept;
	int Increment() noexcept;
	int Decrement() noexcept;
	int Get() const noexcept;

};



namespace nsEInputKey 
{
	enum
	{
		NONE					= 0,

#ifdef NS_PLATFORM_WINDOWS

		// Mouse
		MOUSE_LEFT				= VK_LBUTTON,
		MOUSE_MIDDLE			= VK_MBUTTON,
		MOUSE_RIGHT				= VK_RBUTTON,

		// Keyboard
		KEYBOARD_BACKSPACE		= VK_BACK,
		KEYBOARD_TAB			= VK_TAB,
		KEYBOARD_ENTER			= VK_RETURN,
		KEYBOARD_PAUSE			= VK_PAUSE,
		KEYBOARD_CAPS_LOCK		= VK_CAPITAL,
		KEYBOARD_ESCAPE			= VK_ESCAPE,
		KEYBOARD_SPACEBAR		= VK_SPACE,
		KEYBOARD_PAGE_UP		= VK_PRIOR,
		KEYBOARD_PAGE_DOWN		= VK_NEXT,
		KEYBOARD_END			= VK_END,
		KEYBOARD_HOME			= VK_HOME,
		KEYBOARD_ARROW_LEFT		= VK_LEFT,
		KEYBOARD_ARROW_UP		= VK_UP,
		KEYBOARD_ARROW_RIGHT	= VK_RIGHT,
		KEYBOARD_ARROW_DOWN		= VK_DOWN,
		KEYBOARD_PRINT_SCREEN	= VK_PRINT,
		KEYBOARD_INSERT			= VK_INSERT,
		KEYBOARD_DELETE			= VK_DELETE,

		KEYBOARD_0				= 0x30,
		KEYBOARD_1				= 0x31,
		KEYBOARD_2				= 0x32,
		KEYBOARD_3				= 0x33,
		KEYBOARD_4				= 0x34,
		KEYBOARD_5				= 0x35,
		KEYBOARD_6				= 0x36,
		KEYBOARD_7				= 0x37,
		KEYBOARD_8				= 0x38,
		KEYBOARD_9				= 0x39,

		KEYBOARD_A				= 0x41,
		KEYBOARD_B				= 0x42,
		KEYBOARD_C				= 0x43,
		KEYBOARD_D				= 0x44,
		KEYBOARD_E				= 0x45,
		KEYBOARD_F				= 0x46,
		KEYBOARD_G				= 0x47,
		KEYBOARD_H				= 0x48,
		KEYBOARD_I				= 0x49,
		KEYBOARD_J				= 0x4A,
		KEYBOARD_K				= 0x4B,
		KEYBOARD_L				= 0x4C,
		KEYBOARD_M				= 0x4D,
		KEYBOARD_N				= 0x4E,
		KEYBOARD_O				= 0x4F,
		KEYBOARD_P				= 0x50,
		KEYBOARD_Q				= 0x51,
		KEYBOARD_R				= 0x52,
		KEYBOARD_S				= 0x53,
		KEYBOARD_T				= 0x54,
		KEYBOARD_U				= 0x55,
		KEYBOARD_V				= 0x56,
		KEYBOARD_W				= 0x57,
		KEYBOARD_X				= 0x58,
		KEYBOARD_Y				= 0x59,
		KEYBOARD_Z				= 0x5A,

		KEYBOARD_NUMPAD_0		= VK_NUMPAD0,
		KEYBOARD_NUMPAD_1		= VK_NUMPAD1,
		KEYBOARD_NUMPAD_2		= VK_NUMPAD2,
		KEYBOARD_NUMPAD_3		= VK_NUMPAD3,
		KEYBOARD_NUMPAD_4		= VK_NUMPAD4,
		KEYBOARD_NUMPAD_5		= VK_NUMPAD5,
		KEYBOARD_NUMPAD_6		= VK_NUMPAD6,
		KEYBOARD_NUMPAD_7		= VK_NUMPAD7,
		KEYBOARD_NUMPAD_8		= VK_NUMPAD8,
		KEYBOARD_NUMPAD_9		= VK_NUMPAD9,
		KEYBOARD_NUMPAD_MUL		= VK_MULTIPLY,
		KEYBOARD_NUMPAD_ADD		= VK_ADD,
		KEYBOARD_NUMPAD_SUB		= VK_SUBTRACT,
		KEYBOARD_NUMPAD_DOT		= VK_DECIMAL,
		KEYBOARD_NUMPAD_DIV		= VK_DIVIDE,

		KEYBOARD_F1				= VK_F1,
		KEYBOARD_F2				= VK_F2,
		KEYBOARD_F3				= VK_F3,
		KEYBOARD_F4				= VK_F4,
		KEYBOARD_F5				= VK_F5,
		KEYBOARD_F6				= VK_F6,
		KEYBOARD_F7				= VK_F7,
		KEYBOARD_F8				= VK_F8,
		KEYBOARD_F9				= VK_F9,
		KEYBOARD_F10			= VK_F10,
		KEYBOARD_F11			= VK_F11,
		KEYBOARD_F12			= VK_F12,

		KEYBOARD_NUM_LOCK		= VK_NUMLOCK,
		KEYBOARD_SCROLL_LOCK	= VK_SCROLL,

		KEYBOARD_SHIFT_LEFT		= VK_LSHIFT,
		KEYBOARD_SHIFT_RIGHT	= VK_RSHIFT,
		KEYBOARD_CTRL_LEFT		= VK_LCONTROL,
		KEYBOARD_CTRL_RIGHT		= VK_RCONTROL,
		KEYBOARD_ALT_LEFT		= VK_LMENU,
		KEYBOARD_ALT_RIGHT		= VK_RMENU,
		
		KEYBOARD_SEMICOLON		= VK_OEM_1,
		KEYBOARD_PLUS			= VK_OEM_PLUS,
		KEYBOARD_COMMA			= VK_OEM_COMMA,
		KEYBOARD_MINUS			= VK_OEM_MINUS,
		KEYBOARD_PERIOD			= VK_OEM_PERIOD,
		KEYBOARD_SLASH			= VK_OEM_2,
		KEYBOARD_TILDE			= VK_OEM_3,
		KEYBOARD_BRACKET_LEFT	= VK_OEM_4,
		KEYBOARD_SEPARATOR		= VK_OEM_5,
		KEYBOARD_BRACKET_RIGHT	= VK_OEM_6,
		KEYBOARD_QUOTE			= VK_OEM_7,

#endif // NS_PLATFORM_WINDOWS

		// ...

		MAX_COUNT
	};
};

typedef uint16 nsInputKey;



enum class nsEButtonState : uint8
{
	NONE = 0,
	PRESSED,
	RELEASED,
	REPEAT,
};



enum class nsEMouseCursorShape : uint8
{
	ARROW = 0,
	HAND,
	BEAM,
	SIZE_NS,
	SIZE_WE,
	SIZE_NWSE
};



struct nsMouseMoveEventArgs
{
	nsPointInt Position;
	nsPointInt DeltaPosition;
};



struct nsMouseButtonEventArgs
{
	nsPointInt Position;
	nsInputKey Key;
	nsEButtonState ButtonState;
};



struct nsMouseWheelEventArgs
{
	nsPointInt Position;
	nsPointInt ScrollValue;
};



struct nsKeyboardButtonEventArgs
{
	nsInputKey Key;
	nsEButtonState ButtonState;
};



namespace nsPlatform
{
	extern NS_CORE_API void Initialize(int argc, char* argv[]) noexcept;
	extern NS_CORE_API void Shutdown() noexcept;
	NS_NODISCARD extern const char* GetDirectoryPath() noexcept;
	extern NS_CORE_API void Output(const char* message, nsPlatformConsoleTextColorMasks colorMasks = 0) noexcept;
	extern NS_CORE_API void OutputFormat(const char* format, ...) noexcept;
	extern NS_CORE_API void OutputFormatColored(nsPlatformConsoleTextColorMasks colorMasks, const char* format, ...) noexcept;

	extern NS_CORE_API void Memory_Zero(void* dst, uint64 size) noexcept;
	extern NS_CORE_API void Memory_Set(void* dst, int value, uint64 size) noexcept;
	extern NS_CORE_API void Memory_Copy(void* dst, const void* src, uint64 size) noexcept;
	extern NS_CORE_API void Memory_Move(void* dst, const void* src, uint64 size) noexcept;
	NS_NODISCARD extern NS_CORE_API void* Memory_Alloc(uint64 size) noexcept;
	NS_NODISCARD extern NS_CORE_API void* Memory_Realloc(void* oldData, uint64 size) noexcept;
	extern NS_CORE_API void Memory_Free(void* data) noexcept;

	extern NS_CORE_API int String_Length(const char* cstr) noexcept;
	extern NS_CORE_API void String_Copy(char* dst, const char* src) noexcept;
	extern NS_CORE_API int String_Format(char* buffer, int bufferCount, const char* format, ...) noexcept;
	extern NS_CORE_API bool String_Compare(const char* cstrA, const char* cstrB, bool bIgnoreCase = false) noexcept;
	extern NS_CORE_API void String_ToLower(char* cstr) noexcept;
	extern NS_CORE_API void String_ToUpper(char* cstr) noexcept;
	NS_NODISCARD extern NS_CORE_API int String_ToInt(const char* cstr) noexcept;
	NS_NODISCARD extern NS_CORE_API float String_ToFloat(const char* cstr) noexcept;
	NS_NODISCARD extern NS_CORE_API uint64 String_Hash(const char* cstr) noexcept;

	NS_NODISCARD extern NS_CORE_API bool File_Exists(const char* filePath) noexcept;
	NS_NODISCARD extern NS_CORE_API nsFileHandle File_Open(const char* filePath, nsEPlatformFileOpenMode mode) noexcept;
	extern NS_CORE_API bool File_Seek(nsFileHandle fileHandle, int byteOffset, nsEFileSeekMode mode = nsEFileSeekMode::BEGIN) noexcept;
	extern NS_CORE_API bool File_Read(nsFileHandle fileHandle, void* outResult, int byteSize) noexcept;
	extern NS_CORE_API bool File_Write(nsFileHandle fileHandle, const void* data, int dataSize) noexcept;
	extern NS_CORE_API void File_Close(nsFileHandle& fileHandle) noexcept;
	extern NS_CORE_API bool File_Copy(const char* srcFilePath, const char* dstFilePath) noexcept;
	extern NS_CORE_API bool File_Delete(const char* filePath) noexcept;
	NS_NODISCARD extern NS_CORE_API int File_GetSize(nsFileHandle fileHandle) noexcept;

	NS_NODISCARD extern NS_CORE_API nsModuleHandle Module_Load(const char* moduleFile) noexcept;
	NS_NODISCARD extern NS_CORE_API void Module_Unload(nsModuleHandle& moduleHandle) noexcept;
	NS_NODISCARD extern NS_CORE_API void* Module_GetFunction(nsModuleHandle moduleHandle, const char* functionName) noexcept;

	template<typename TFunction>
	NS_NODISCARD_INLINE TFunction Module_GetFunctionAs(nsModuleHandle moduleHandle, const char* functionName) noexcept
	{
		return static_cast<TFunction>(Module_GetFunction(moduleHandle, functionName));
	}


	NS_NODISCARD extern NS_CORE_API int64 PerformanceQuery_Frequency() noexcept;
	NS_NODISCARD extern NS_CORE_API int64 PerformanceQuery_Counter() noexcept;

	extern NS_CORE_API void Mouse_ShowCursor(bool bShow) noexcept;
	extern NS_CORE_API void Mouse_SetCursorShape(nsEMouseCursorShape shape) noexcept;
	extern NS_CORE_API void Mouse_SetCapture(nsWindowHandle windowHandle, bool bCapture) noexcept;
	extern NS_CORE_API void Mouse_SetCursorWindowPosition(nsWindowHandle windowHandle, const nsPointInt& position) noexcept;
	extern NS_CORE_API void Mouse_ClipCursor(bool bClip, nsWindowHandle windowHandle = NULL, const nsRectInt& rect = nsRectInt()) noexcept;
	NS_NODISCARD extern NS_CORE_API bool Mouse_IsCursorHidden() noexcept;

	extern NS_CORE_API void Thread_Sleep(int ms) noexcept;

};



#define NS_AssertOutputV(expr, message, ...) \
char messageBuffer[1024]; \
nsPlatform::String_Format(messageBuffer, 1024, message, __VA_ARGS__); \
nsPlatform::OutputFormatColored(nsEPlatformConsoleTextColor::Red, "\nAssertion failed! (%s)\nMessage: %s\nFile: %s\nLine: %i\n", #expr, messageBuffer, __FILE__, __LINE__) \

#define NS_AssertOutput(expr) nsPlatform::OutputFormatColored(nsEPlatformConsoleTextColor::Red, "\nAssertion failed! (%s)\nFile: %s\nLine: %i\n", #expr, __FILE__, __LINE__)


#ifdef _DEBUG

#ifdef NS_PLATFORM_WINDOWS
#include <intrin.h>

#define NS_AssertV(expr, message, ...)				\
if (!(expr))										\
{													\
	NS_AssertOutputV(expr, message, __VA_ARGS__);	\
	__debugbreak();									\
}

#define NS_Assert(expr)			\
if (!(expr))					\
{								\
	 NS_AssertOutput(expr);		\
	__debugbreak();				\
}

#endif // NS_PLATFORM_WINDOWS

#else
#define NS_AssertV(expr, message, ...)
#define NS_Assert(expr)

#endif // _DEBUG


#define NS_ValidateV(expr, message, ...)			\
if (!(expr))										\
{													\
	NS_AssertOutputV(expr, message, __VA_ARGS__);	\
	NS_Abort();										\
}

#define NS_Validate(expr)							\
if (!(expr))										\
{													\
	NS_AssertOutput(expr);							\
	NS_Abort();										\
}
