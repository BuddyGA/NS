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


void nsCommandLines::Initialize(const char* commandLine) noexcept
{
	if (bInitialized)
	{
		return;
	}

	const int strLen = nsPlatform::String_Length(commandLine);

	if (strLen == 0)
	{
		bInitialized = true;
		return;
	}

	nsTArray<nsString> commandArgs;
	commandArgs.Reserve(8);

	for (int i = 0; i < strLen; ++i)
	{
		if (commandLine[i] == '-')
		{
			int len = 1;

			for (int j = i + 1; j < strLen; ++j)
			{
				char c = commandLine[j];

				if (c == '\0' || c == ' ')
				{
					break;
				}

				len++;
			}

			nsString& arg = commandArgs.Add();
			arg.Resize(len);
			nsPlatform::Memory_Copy(*arg, &commandLine[i], len);
		}
	}

	const int cmdCount = commandArgs.GetCount();
	nsTArray<nsString> commandValuePairs;
	commandValuePairs.Reserve(cmdCount);
	nsString arg;

	for (int i = 0; i < cmdCount; ++i)
	{
		arg = commandArgs[i];
		arg.RemoveCharInPlace('-');
		arg.TrimInPlace();
		arg.ToLowerInPlace();
		commandValuePairs = arg.Splits('=');

		if (commandValuePairs.GetCount() == 1)
		{
			commandValuePairs.Add("1");
		}

		CommandTable.Add(*commandValuePairs[0], *commandValuePairs[1]);
		arg.Clear();
		commandValuePairs.Clear();
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
