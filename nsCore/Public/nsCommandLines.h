#pragma once

#include "nsString.h"



class NS_CORE_API nsCommandLines
{
	NS_DECLARE_SINGLETON(nsCommandLines)

private:
	bool bInitialized;
	nsTMap<nsString, nsString> CommandTable;


public:
	void Initialize(int argc, wchar_t* argv[]) noexcept;
	void Initialize(const wchar_t* commandLine) noexcept;
	bool HasCommand(const nsString& command) const noexcept;
	nsString GetValue(const nsString& command) const noexcept;


	NS_NODISCARD_INLINE int GetValueAsInt(const nsString& command) const noexcept
	{
		return GetValue(command).ToInt();
	}
	

	NS_NODISCARD_INLINE float GetValuesAsFloat(const nsString& command) const noexcept
	{
		return GetValue(command).ToFloat();
	}

};
