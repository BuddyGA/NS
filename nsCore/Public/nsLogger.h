#pragma once

#include "nsString.h"



enum class nsELogVerbosity : uint8
{
	LV_DEBUG = 0,
	LV_INFO,
	LV_WARNING,
	LV_ERROR,
};



class nsLogCategory
{
public:
	const char* Name;
	nsELogVerbosity Verbosity;


public:
	nsLogCategory() noexcept
		: Name(nullptr)
		, Verbosity(nsELogVerbosity::LV_INFO)
	{
	}


	nsLogCategory(const char* name, nsELogVerbosity defaultVerbosity) noexcept
		: Name(name)
		, Verbosity(defaultVerbosity)
	{
	}

};



class NS_CORE_API nsLogger
{
private:
	bool bInitialized;
	nsELogVerbosity GlobalVerbosity;
	nsFileHandle OutputFileHandle;
	nsCriticalSection OutputFileCriticalSection;


public:
	void Initialize(nsELogVerbosity globalVerbosity, nsString outputFile) noexcept;
	void OutputLog(const nsString& message, nsELogVerbosity verbosity) noexcept;
	nsString OutputLogCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const nsString& message) noexcept;


	NS_NODISCARD_INLINE nsELogVerbosity GetGlobalVerbosity() const noexcept
	{
		return GlobalVerbosity;
	}


	NS_DECLARE_SINGLETON(nsLogger)

};



extern NS_CORE_API nsLogCategory nsTempLog;
extern NS_CORE_API nsLogCategory nsSystemLog;



#define NS_LogDebug(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_DEBUG, nsString::Format(format, __VA_ARGS__))
#define NS_LogInfo(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_INFO, nsString::Format(format, __VA_ARGS__))
#define NS_LogWarning(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_WARNING, nsString::Format(format, __VA_ARGS__))
#define NS_LogError(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_ERROR, nsString::Format(format, __VA_ARGS__))
