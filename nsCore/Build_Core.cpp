#include "nsPlatform.h"

#ifdef NS_PLATFORM_WINDOWS
#include "Private/Win64_Platform.cpp"
#include "Private/Win64_Threadpool.cpp"
#include "Private/Win64_Window.cpp"
#include "Private/Win64_FileSystem.cpp"
#endif // NS_PLATFORM_WINDOWS

#include "Private/nsReflection.cpp"
#include "Private/nsCommandLines.cpp"
#include "Private/nsFileSystem.cpp"
#include "Private/nsLogger.cpp"
#include "Private/nsMath.cpp"
#include "Private/nsMemory.cpp"
#include "Private/nsStream.cpp"
#include "Private/nsString.cpp"
#include "Private/nsObject.cpp"
