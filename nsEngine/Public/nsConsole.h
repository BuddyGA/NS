#pragma once

#include "nsEngineTypes.h"



NS_DELEGATE_ThreeParams(nsConsoleCallbackDelegate, const nsString&, command, const nsString*, params, int, paramCount)



struct nsConsoleLogEntry
{
	int CharIndex;
	nsColor Color;
};


class NS_ENGINE_API nsConsoleManager
{
	NS_DECLARE_SINGLETON(nsConsoleManager)

private:
	bool bInitialized;

	nsTArray<wchar_t> LogChars;
	nsTArray<nsConsoleLogEntry> LogEntries;

	nsTMap<nsString, nsConsoleCallbackDelegate> CommandTable;


public:
	void Initialize() noexcept;
	void AddLogEntry(const nsString& log, nsColor color);
	void ExecuteCommand(const nsString& textCommand);

	template<typename...TVarArgs>
	NS_INLINE void AddLogEntryWithCategory(const nsLogCategory& category, nsELogVerbosity verbosity, const wchar_t* format, TVarArgs... args)
	{
		if (verbosity < category.Verbosity || category.Verbosity < nsLogger::Get().GlobalVerbosity)
		{
			return;
		}

		nsString output = nsLogger::Get().OutputLogCategory(category, verbosity, format, args...);
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


	NS_NODISCARD_INLINE const nsTArray<wchar_t>& GetLogChars() const noexcept
	{
		return LogChars;
	}


	NS_NODISCARD_INLINE const nsTArray<nsConsoleLogEntry>& GetLogEntries() const noexcept
	{
		return LogEntries;
	}

};



#define NS_CONSOLE_Debug(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_DEBUG, message, __VA_ARGS__)
#define NS_CONSOLE_Log(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_INFO, message, __VA_ARGS__)
#define NS_CONSOLE_Warning(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_WARNING, message, __VA_ARGS__)
#define NS_CONSOLE_Error(category, message, ...) nsConsoleManager::Get().AddLogEntryWithCategory(category, nsELogVerbosity::LV_ERROR, message, __VA_ARGS__)


#define NS_CONSOLE_RegisterCommand(command) nsConsoleManager::Get().RegisterCommand(command, this)
