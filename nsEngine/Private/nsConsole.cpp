#include "nsConsole.h"



nsConsoleManager::nsConsoleManager() noexcept
	: bInitialized(false)
{
	LogChars.Reserve(16384);
	LogEntries.Reserve(2048);
}


void nsConsoleManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	/*
	NS_CONSOLE_Log(nsTempLog, "[00] log message...");
	NS_CONSOLE_Log(nsTempLog, "[01] log message...");
	NS_CONSOLE_Log(nsTempLog, "[02] log message...");
	NS_CONSOLE_Log(nsTempLog, "[03] log message...");
	NS_CONSOLE_Log(nsTempLog, "[04] log message...");
	NS_CONSOLE_Log(nsTempLog, "[05] log message...");
	NS_CONSOLE_Warning(nsTempLog, "[06] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[07] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[08] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[09] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[10] warning message...");
	NS_CONSOLE_Error(nsTempLog, "[11] Error message...");
	NS_CONSOLE_Error(nsTempLog, "[12] Error message...");
	NS_CONSOLE_Error(nsTempLog, "[13] Error message...");
	NS_CONSOLE_Error(nsTempLog, "[14] Error message...");
	NS_CONSOLE_Error(nsTempLog, "[15] Error message...");
	NS_CONSOLE_Log(nsTempLog, "[16] log message...");
	NS_CONSOLE_Log(nsTempLog, "[17] log message...");
	NS_CONSOLE_Log(nsTempLog, "[18] log message...");
	NS_CONSOLE_Warning(nsTempLog, "[19] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[20] warning message...");
	NS_CONSOLE_Log(nsTempLog, "[21] log message...");
	NS_CONSOLE_Log(nsTempLog, "[22] log message...");
	NS_CONSOLE_Log(nsTempLog, "[23] log message...");
	NS_CONSOLE_Error(nsTempLog, "[24] Error message...");
	NS_CONSOLE_Log(nsTempLog, "[25] log message...");
	NS_CONSOLE_Log(nsTempLog, "[26] log message...");
	NS_CONSOLE_Log(nsTempLog, "[27] log message...");
	NS_CONSOLE_Warning(nsTempLog, "[28] warning message...");
	NS_CONSOLE_Warning(nsTempLog, "[29] warning message...");
	NS_CONSOLE_Log(nsTempLog, "[30] log message...");
	NS_CONSOLE_Log(nsTempLog, "[31] log message...");
	NS_CONSOLE_Log(nsTempLog, "[32] log message...");
	NS_CONSOLE_Log(nsTempLog, "[33] log message...");
	NS_CONSOLE_Log(nsTempLog, "[34] log message...");
	NS_CONSOLE_Log(nsTempLog, "[35] log message...");
	NS_CONSOLE_Error(nsTempLog, "[36] Error message...");
	NS_CONSOLE_Log(nsTempLog, "[37] log message...");
	NS_CONSOLE_Log(nsTempLog, "[38] log message...");
	NS_CONSOLE_Warning(nsTempLog, "[39] warning message...");
	NS_CONSOLE_Log(nsTempLog, "[40] log message...");
	NS_CONSOLE_Log(nsTempLog, "[41] log message...");
	NS_CONSOLE_Log(nsTempLog, "[42] log message...");
	NS_CONSOLE_Log(nsTempLog, "[43] log message...");
	NS_CONSOLE_Error(nsTempLog, "[44] Error message... [END]");
	*/

	bInitialized = true;
}


void nsConsoleManager::AddLogEntry(const nsString& log, nsColor color)
{
	if (log.IsEmpty())
	{
		return;
	}

	nsConsoleLogEntry& entry = LogEntries.Add();
	entry.CharIndex = LogChars.GetCount();
	entry.Color = color;

	const int charLength = log.GetLength();
	LogChars.Reserve(charLength + 128);
	LogChars.InsertAt(*log, charLength + 1);
}


void nsConsoleManager::AddLogEntryWithCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const nsString& log)
{
	if (verbosity < category.Verbosity || category.Verbosity < nsLogger::Get().GetGlobalVerbosity())
	{
		return;
	}

	nsString output = nsLogger::Get().OutputLogCategory(category, verbosity, log);
	nsColor color = nsColor::WHITE;

	if (verbosity == nsELogVerbosity::LV_WARNING)
	{
		color = nsColor::YELLOW;
	}
	else if (verbosity == nsELogVerbosity::LV_ERROR)
	{
		color = nsColor::RED;
	}

	AddLogEntry(output, color);
}


void nsConsoleManager::ExecuteCommand(const nsString& textCommand)
{
	if (textCommand.GetLength() == 0)
	{
		return;
	}

	const nsTArray<nsString> inputs = textCommand.Splits(' ');
	const nsString& command = inputs[0];
	nsConsoleCallbackDelegate* commandCallback = CommandTable.GetValueByKey(command);

	if (commandCallback)
	{
		commandCallback->Broadcast(command, inputs.GetCount() > 1 ? &inputs[1] : nullptr, inputs.GetCount() - 1);
		AddLogEntry(nsString::Format("Command: %s", *textCommand), nsColor::GREEN);
	}
	else
	{
		AddLogEntry(nsString::Format("Unrecognized command: %s", *command), nsColor::YELLOW);
	}
}
