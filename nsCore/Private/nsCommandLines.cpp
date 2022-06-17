#include "nsCommandLines.h"



nsCommandLines::nsCommandLines() noexcept
	: bInitialized(false)
{

}


void nsCommandLines::Initialize(int argc, char* argv[]) noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_Assert(argc > 0);
	NS_Assert(argv);

	nsTArray<nsString> commandAndValue;
	commandAndValue.Reserve(2);
	nsString argument;

	// Skip executable path, therefore, (i = 1)
	for (int i = 1; i < argc; ++i)
	{
		argument = argv[i];
		argument.RemoveCharInPlace('-');
		argument.TrimInPlace();
		argument.ToLowerInPlace();
		commandAndValue = argument.Splits('=');

		if (commandAndValue.GetCount() == 1)
		{
			commandAndValue.Add("1");
		}

		CommandTable.Add(*commandAndValue[0], *commandAndValue[1]);
		argument.Clear();
		commandAndValue.Clear();
	}

	bInitialized = true;
}


bool nsCommandLines::HasCommand(nsName command) const noexcept
{
	return CommandTable.Exists(command);
}


nsName nsCommandLines::GetValue(nsName command) const noexcept
{
	if (!CommandTable.Exists(command))
	{
		return "";
	}

	return CommandTable[command];
}
