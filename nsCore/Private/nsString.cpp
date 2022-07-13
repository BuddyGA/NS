#include "nsString.h"


/*
nsString nsString::Format(const char* format, ...) noexcept
{
	constexpr int BUFFER_SIZE = 2048;

	char buffer[BUFFER_SIZE];
	va_list args;
	va_start(args, format);
	int n = vsnprintf(buffer, BUFFER_SIZE, format, args);

	nsString temp;

	if (n > BUFFER_SIZE - 1)
	{
		temp.Resize(n);
		n = vsnprintf(*temp, temp.GetLength(), format, args);
	}
	else
	{
		temp = buffer;
	}

	NS_Assert(temp.GetLength() == n);

	va_end(args);

	return temp;
}


nsString nsString::FromBool(bool bValue) noexcept
{
	char buffer[8];
	nsPlatform::String_Format(buffer, 8, "%s", bValue ? "True" : "False");

	return buffer;
}


nsString nsString::FromInt(int value) noexcept
{
	char buffer[16];
	nsPlatform::String_Format(buffer, 16, "%i", value);

	return buffer;
}


nsString nsString::FromFloat(float value, int precision) noexcept
{
	char buffer[32];
	nsPlatform::String_Format(buffer, 32, "%.*f", precision, value);

	return buffer;
}



nsName nsName::Format(const char* format, ...) noexcept
{
	nsName name;

	va_list args;
	va_start(args, format);
	vsnprintf(name.Chars, nsName::N - 1, format, args);
	va_end(args);

	return name;
}
*/


nsName nsName::NONE;
