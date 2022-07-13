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
	const wchar_t* Name;
	nsELogVerbosity Verbosity;


public:
	nsLogCategory() noexcept
		: Name(nullptr)
		, Verbosity(nsELogVerbosity::LV_INFO)
	{
	}


	nsLogCategory(const wchar_t* name, nsELogVerbosity defaultVerbosity) noexcept
		: Name(name)
		, Verbosity(defaultVerbosity)
	{
	}

};



class NS_CORE_API nsLogger
{
	NS_DECLARE_SINGLETON(nsLogger)

private:
	bool bInitialized;
	nsPlatformFileHandle OutputFileHandle;
	nsCriticalSection OutputFileCriticalSection;

public:
	nsELogVerbosity GlobalVerbosity;


public:
	void Initialize(nsELogVerbosity globalVerbosity, nsString outputFile) noexcept;
	void OutputLog(const nsString& message, nsELogVerbosity verbosity) noexcept;

	template<typename...TVarArgs>
	NS_INLINE nsString OutputLogCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const wchar_t* format, TVarArgs... args) noexcept
	{
		NS_AssertV(bInitialized, TEXT("Must call Initialize()!"));

		if (verbosity < category.Verbosity || verbosity < GlobalVerbosity)
		{
			return TEXT("");
		}

		nsString message = nsString::Format(format, args...);
		nsString output;

		switch (verbosity)
		{
			case nsELogVerbosity::LV_INFO: output = nsString::Format(TEXT("[INF] %s: %s"), category.Name, *message); break;
			case nsELogVerbosity::LV_DEBUG: output = nsString::Format(TEXT("[DBG] %s: %s"), category.Name, *message); break;
			case nsELogVerbosity::LV_WARNING: output = nsString::Format(TEXT("[WRN] %s: %s"), category.Name, *message); break;
			case nsELogVerbosity::LV_ERROR: output = nsString::Format(TEXT("[ERR] %s: %s"), category.Name, *message); break;
			default: break;
		}

		OutputLog(output, verbosity);

		return output;
	}

};



extern NS_CORE_API nsLogCategory nsTempLog;
extern NS_CORE_API nsLogCategory nsSystemLog;



#define NS_LogDebug(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_DEBUG, format, __VA_ARGS__)
#define NS_LogInfo(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_INFO, format, __VA_ARGS__)
#define NS_LogWarning(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_WARNING, format, __VA_ARGS__)
#define NS_LogError(category, format, ...) nsLogger::Get().OutputLogCategory(category, nsELogVerbosity::LV_ERROR, format, __VA_ARGS__)
