#include "nsCommandLines.h"



nsCommandLines::nsCommandLines() noexcept
	: bInitialized(false)
{

}


void nsCommandLines::Initialize(int argc, wchar_t* argv[]) noexcept
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
		argument.RemoveCharInPlace(TEXT('-'));
		argument.TrimInPlace();
		argument.ToLowerInPlace();
		commandAndValue = argument.Splits(TEXT('='));

		if (commandAndValue.GetCount() == 1)
		{
			commandAndValue.Add(TEXT("1"));
		}

		CommandTable.Add(*commandAndValue[0], *commandAndValue[1]);
		argument.Clear();
		commandAndValue.Clear();
	}

	bInitialized = true;
}


void nsCommandLines::Initialize(const wchar_t* commandLine) noexcept
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
		if (commandLine[i] == TEXT('-'))
		{
			int len = 1;

			for (int j = i + 1; j < strLen; ++j)
			{
				wchar_t c = commandLine[j];

				if (c == TEXT('\0') || c == TEXT(' '))
				{
					break;
				}

				len++;
			}

			nsString& arg = commandArgs.Add();
			arg.Resize(len);
			nsPlatform::Memory_Copy(*arg, &commandLine[i], sizeof(wchar_t) * len);
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
		commandValuePairs = arg.Splits(TEXT('='));

		if (commandValuePairs.GetCount() == 1)
		{
			commandValuePairs.Add(TEXT("1"));
		}

		CommandTable.Add(*commandValuePairs[0], *commandValuePairs[1]);
		arg.Clear();
		commandValuePairs.Clear();
	}

	bInitialized = true;
}


bool nsCommandLines::HasCommand(const nsString& command) const noexcept
{
	return CommandTable.Exists(command);
}


nsString nsCommandLines::GetValue(const nsString& command) const noexcept
{
	if (!CommandTable.Exists(command))
	{
		return TEXT("");
	}

	return CommandTable[command];
}
