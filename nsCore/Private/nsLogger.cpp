#include "nsLogger.h"



nsLogCategory nsTempLog("nsTempLog", nsELogVerbosity::LV_DEBUG);
nsLogCategory nsSystemLog("nsSystemLog", nsELogVerbosity::LV_DEBUG);



nsLogger::nsLogger() noexcept
	: bInitialized(false)
	, GlobalVerbosity(nsELogVerbosity::LV_INFO)
	, OutputFileHandle(nullptr)
{
}


void nsLogger::Initialize(nsELogVerbosity globalVerbosity, nsString outputFile) noexcept
{
	if (bInitialized)
	{
		return;
	}

	GlobalVerbosity = globalVerbosity;

	if (!outputFile.IsEmpty())
	{
		OutputFileHandle = nsPlatform::File_Open(*outputFile, nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING);
	}

	bInitialized = true;
}


void nsLogger::OutputLog(const nsString& message, nsELogVerbosity verbosity) noexcept
{
	NS_AssertV(bInitialized, "Must call Initialize()!");

	if (verbosity < GlobalVerbosity)
	{
		return;
	}

	const int length = message.GetLength();

	if (length == 0)
	{
		return;
	}

	nsPlatformConsoleTextColorMasks outputColorMasks = 0;

	if (verbosity == nsELogVerbosity::LV_WARNING)
	{
		outputColorMasks = nsEPlatformConsoleTextColor::Red | nsEPlatformConsoleTextColor::Green;
	}
	else if (verbosity == nsELogVerbosity::LV_ERROR)
	{
		outputColorMasks = nsEPlatformConsoleTextColor::Red;
	}

	nsString outputMessage = nsString::Format("%s\n", *message);
	nsPlatform::Output(*outputMessage, outputColorMasks);

	if (OutputFileHandle)
	{
		OutputFileCriticalSection.Enter();
		nsPlatform::File_Write(OutputFileHandle, *outputMessage, outputMessage.GetLength());
		OutputFileCriticalSection.Leave();
	}
}


nsString nsLogger::OutputLogCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const nsString& message) noexcept
{
	NS_AssertV(bInitialized, "Must call Initialize()!");

	if (verbosity < category.Verbosity || verbosity < GlobalVerbosity)
	{
		return "";
	}

	nsString output;

	switch (verbosity)
	{
		case nsELogVerbosity::LV_INFO: output = nsString::Format("[INF] %s: %s", category.Name, *message); break;
		case nsELogVerbosity::LV_DEBUG: output = nsString::Format("[DBG] %s: %s", category.Name, *message); break;
		case nsELogVerbosity::LV_WARNING: output = nsString::Format("[WRN] %s: %s", category.Name, *message); break;
		case nsELogVerbosity::LV_ERROR: output = nsString::Format("[ERR] %s: %s", category.Name, *message); break;
		default: break;
	}

	OutputLog(output, verbosity);

	return output;
}
