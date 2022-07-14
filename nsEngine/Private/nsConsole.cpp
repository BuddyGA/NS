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

	NS_CONSOLE_Log(nsTempLog, TEXT("[00] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[01] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[02] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[03] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[04] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[05] log message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[06] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[07] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[08] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[09] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[10] warning message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[11] Error message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[12] Error message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[13] Error message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[14] Error message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[15] Error message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[16] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[17] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[18] log message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[19] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[20] warning message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[21] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[22] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[23] log message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[24] Error message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[25] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[26] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[27] log message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[28] warning message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[29] warning message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[30] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[31] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[32] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[33] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[34] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[35] log message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[36] Error message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[37] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[38] log message..."));
	NS_CONSOLE_Warning(nsTempLog, TEXT("[39] warning message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[40] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[41] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[42] log message..."));
	NS_CONSOLE_Log(nsTempLog, TEXT("[43] log message..."));
	NS_CONSOLE_Error(nsTempLog, TEXT("[44] Error message... [END]"));

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


void nsConsoleManager::ExecuteCommand(const nsString& textCommand)
{
	if (textCommand.GetLength() == 0)
	{
		return;
	}

	const nsString lowerCaseTextCommand = textCommand.ToLower();
	const nsTArray<nsString> inputs = lowerCaseTextCommand.Splits(' ');
	const nsString& command = inputs[0];

	if (command == TEXT("clear"))
	{
		LogChars.Clear();
		LogEntries.Clear();
		return;
	}

	nsConsoleCallbackDelegate* commandCallback = CommandTable.GetValueByKey(command);

	if (commandCallback)
	{
		AddLogEntry(nsString::Format(TEXT("Command: %s"), *textCommand), nsColor::GREEN);
		commandCallback->Broadcast(command, inputs.GetCount() > 1 ? &inputs[1] : nullptr, inputs.GetCount() - 1);
	}
	else
	{
		AddLogEntry(nsString::Format(TEXT("Unrecognized command: %s"), *command), nsColor::YELLOW);
	}
}
