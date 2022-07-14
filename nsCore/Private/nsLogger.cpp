#include "nsLogger.h"



nsLogCategory nsTempLog(TEXT("nsTempLog"), nsELogVerbosity::LV_DEBUG);
nsLogCategory nsSystemLog(TEXT("nsSystemLog"), nsELogVerbosity::LV_INFO);



nsLogger::nsLogger() noexcept
	: bInitialized(false)
	, OutputFileHandle(nullptr)
	, GlobalVerbosity(nsELogVerbosity::LV_INFO)
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
	NS_AssertV(bInitialized, TEXT("Must call Initialize()!"));

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
		outputColorMasks = nsEPlatformConsoleOutputColor::Red | nsEPlatformConsoleOutputColor::Green;
	}
	else if (verbosity == nsELogVerbosity::LV_ERROR)
	{
		outputColorMasks = nsEPlatformConsoleOutputColor::Red;
	}

	const nsString outputMessage = nsString::Format(TEXT("%s\n"), *message);
	nsPlatform::ConsoleOutput(*outputMessage, outputColorMasks);

	if (OutputFileHandle)
	{
		OutputFileCriticalSection.Enter();
		nsPlatform::File_Write(OutputFileHandle, *outputMessage, outputMessage.GetAllocatedSizeBytes());
		OutputFileCriticalSection.Leave();
	}
}
