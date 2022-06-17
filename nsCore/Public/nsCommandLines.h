#pragma once

#include "nsString.h"



class NS_CORE_API nsCommandLines
{
	NS_DECLARE_SINGLETON(nsCommandLines)

private:
	bool bInitialized;
	nsTMap<nsName, nsName> CommandTable;


public:
	void Initialize(int argc, char* argv[]) noexcept;
	void Initialize(const char* commandLine) noexcept;
	bool HasCommand(nsName command) const noexcept;
	nsName GetValue(nsName command) const noexcept;


	NS_NODISCARD_INLINE int GetValueAsInt(nsName command) const noexcept
	{
		return GetValue(command).ToInt();
	}
	

	NS_NODISCARD_INLINE float GetValuesAsFloat(nsName command) const noexcept
	{
		return GetValue(command).ToFloat();
	}

};
