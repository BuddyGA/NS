#pragma once

#include "nsGUIFramework.h"



NS_DELEGATE_ThreeParams(nsConsoleCallbackDelegate, const nsString&, command, const nsString*, params, int, paramCount)



struct nsConsoleLogEntry
{
	int CharIndex;
	nsColor Color;
};


class NS_ENGINE_API nsConsoleManager
{
private:
	bool bInitialized;

	nsTArray<char> LogChars;
	nsTArray<nsConsoleLogEntry> LogEntries;

	nsTMap<nsString, nsConsoleCallbackDelegate> CommandTable;


public:
	void Initialize() noexcept;
	void AddLogEntry(const nsString& log, nsColor color);
	void AddLogEntryWithCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const nsString& log);
	void ExecuteCommand(const nsString& textCommand);

private:
	NS_NODISCARD_INLINE nsConsoleCallbackDelegate& AddCommand(const nsString& command) noexcept
	{
		return CommandTable.Add(command.ToLower());
	}

public:
	template<typename T>
	NS_INLINE void RegisterCommand(nsString command, T* object)
	{
		AddCommand(command).Bind(object, &T::HandleConsoleCommand);
	}


	NS_NODISCARD_INLINE const nsTArray<char>& GetLogChars() const noexcept
	{
		return LogChars;
	}


	NS_NODISCARD_INLINE const nsTArray<nsConsoleLogEntry>& GetLogEntries() const noexcept
	{
		return LogEntries;
	}

	NS_DECLARE_SINGLETON(nsConsoleManager)

};



#define NS_CONSOLE_Debug(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_DEBUG, nsString::Format(message, __VA_ARGS__))
#define NS_CONSOLE_Log(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_INFO, nsString::Format(message, __VA_ARGS__))
#define NS_CONSOLE_Warning(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_WARNING, nsString::Format(message, __VA_ARGS__))
#define NS_CONSOLE_Error(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_ERROR, nsString::Format(message, __VA_ARGS__))


#define NS_CONSOLE_RegisterCommand(command) nsConsoleManager::Get().RegisterCommand(command, this)
