#pragma once

#include "nsContainer.h"


#define NS_STRING_MAX_LENGTH	NS_ARRAY_INDEX_LAST


class nsString
{
private:
	nsTArray<char> CharArray;


public:
	nsString() noexcept = default;
	~nsString() noexcept = default;


	nsString(const nsString& other) noexcept
		: CharArray(other.CharArray)
	{
		//nsPlatform::Output("nsString copy constructor\n");
	}


	nsString(nsString&& other) noexcept
		: CharArray(std::move(other.CharArray))
	{
		//nsPlatform::Output("nsString move constructor\n");
	}


	nsString(const char* cstr) noexcept
	{
		//nsPlatform::Output("nsString const char* constructor\n");
		CopyChars(cstr);
	}


private:
	NS_INLINE void CopyChars(const char* cstr) noexcept
	{
		const int len = nsPlatform::String_Length(cstr);

		if (len == 0)
		{
			return;
		}

		Resize(len);
		nsPlatform::String_Copy(CharArray.GetData(), cstr);
	}


public:
	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		CharArray.Reserve(newCapacity);
	}


	NS_INLINE void Resize(int newLength) noexcept
	{
		CharArray.Resize(newLength + 1);
		CharArray[newLength] = '\0';
	}


	NS_NODISCARD_INLINE int GetLength() const noexcept
	{
		const int count = CharArray.GetCount();
		return count > 0 ? count - 1 : 0;
	}


	NS_NODISCARD_INLINE int Find(char c) const noexcept
	{
		return CharArray.Find(c);
	}


	NS_NODISCARD_INLINE int FindLast(char c) const noexcept
	{
		return CharArray.FindLast(c);
	}


	NS_INLINE void Add(char c) noexcept
	{
		CharArray.Add(c);
	}


	NS_INLINE void AppendInPlace(const nsString& other) noexcept
	{
		const int otherLength = other.GetLength();

		if (otherLength == 0)
		{
			return;
		}

		const int length = GetLength();
		const int insertIndex = length;
		Resize(length + otherLength);
		nsPlatform::String_Copy(CharArray.GetData() + insertIndex, other.CharArray.GetData());
	}


	NS_NODISCARD_INLINE nsString Append(const nsString& other) const noexcept
	{
		nsString temp = *this;
		temp.AppendInPlace(other);

		return temp;
	}


	NS_INLINE int InsertAt(const char* cstr, int index = NS_STRING_MAX_LENGTH) noexcept
	{
		const int len = nsPlatform::String_Length(cstr);

		if (len == 0)
		{
			return 0;
		}

		const int currentLength = GetLength();

		// Append
		if (currentLength == 0 || index == currentLength || index == NS_STRING_MAX_LENGTH)
		{
			const int insertIndex = currentLength;
			Resize(currentLength + len);
			nsPlatform::String_Copy(CharArray.GetData() + insertIndex, cstr);
		}
		else
		{
			NS_Assert(currentLength > index);
			Resize(currentLength + len);
			nsPlatform::Memory_Move(CharArray.GetData() + (index + len), CharArray.GetData() + index, currentLength - index);
			nsPlatform::Memory_Copy(CharArray.GetData() + index, cstr, len);
		}

		return len;
	}


	NS_INLINE bool RemoveAt(int index) noexcept
	{
		return CharArray.RemoveAt(index);
	}


	NS_INLINE bool RemoveAtRange(int index, int count = NS_STRING_MAX_LENGTH) noexcept
	{
		if (count == NS_STRING_MAX_LENGTH)
		{
			count = GetLength() - index;
		}

		return CharArray.RemoveAtRange(index, count);
	}


	NS_INLINE void RemoveCharInPlace(char c) noexcept
	{
		if (GetLength() == 0)
		{
			return;
		}

		int i = 0;

		while (CharArray[i] != '\0')
		{
			if (CharArray[i] == c)
			{
				CharArray.RemoveAt(i);
				CharArray[GetLength()] = '\0';
			}
			else
			{
				++i;
			}
		}
	}


	NS_NODISCARD_INLINE nsString RemoveChar(char c) const noexcept
	{
		nsString temp;
		temp.RemoveCharInPlace(c);

		return temp;
	}


	NS_INLINE void Clear() noexcept
	{
		CharArray.Clear();
	}


	NS_INLINE void ToLowerInPlace() noexcept
	{
		nsPlatform::String_ToLower(CharArray.GetData());
	}


	NS_NODISCARD_INLINE nsString ToLower() const noexcept
	{
		nsString temp = *this;
		temp.ToLowerInPlace();

		return temp;
	}


	NS_INLINE void ToUpperInPlace() noexcept
	{
		nsPlatform::String_ToUpper(CharArray.GetData());
	}


	NS_NODISCARD_INLINE nsString ToUpper() const noexcept
	{
		nsString temp = *this;
		temp.ToUpperInPlace();

		return temp;
	}


	NS_INLINE void ReplaceInPlace(char c, char with) noexcept
	{
		const int length = GetLength();

		for (int i = 0; i < length; ++i)
		{
			if (CharArray[i] == c)
			{
				CharArray[i] = with;
			}
		}
	}


	NS_NODISCARD_INLINE nsString Replace(char c, char with) const noexcept
	{
		nsString temp = *this;
		temp.ReplaceInPlace(c, with);

		return temp;
	}


	NS_INLINE void TrimInPlace() noexcept
	{
		if (GetLength() == 0)
		{
			return;
		}

		int i = 0;

		while (CharArray[i] != '\0')
		{
			if (CharArray[i] == '\n' || CharArray[i] == '\r' || CharArray[i] == '\t' || CharArray[i] == ' ')
			{
				RemoveAt(i);
				CharArray[GetLength()] = '\0';
			}
			else
			{
				i++;
			}
		}
	}


	NS_NODISCARD_INLINE nsString Trim() const noexcept
	{
		nsString temp = *this;
		temp.TrimInPlace();

		return temp;
	}


	NS_NODISCARD_INLINE nsString Substring(int index, int length = NS_STRING_MAX_LENGTH) const noexcept
	{
		const int thisLength = GetLength();

		if (thisLength == 0)
		{
			return *this;
		}

		NS_Assert(index >= 0 && index < thisLength);
		NS_Assert(length > 0 && length <= NS_STRING_MAX_LENGTH);

		if (length >= thisLength)
		{
			length = thisLength;
		}

		nsString temp;
		temp.Resize(length);
		nsPlatform::Memory_Copy(*temp, &CharArray[index], length);

		return temp;
	}


	NS_NODISCARD_INLINE nsTArray<nsString> Splits(char separator) const noexcept
	{
		nsTArray<nsString> result;
		const int length = GetLength();

		if (length == 0)
		{
			return result;
		}

		int index = 0;
		int charLength = 0;

		for (int i = 0; i < length; ++i)
		{
			if (CharArray[i] == separator)
			{
				if (charLength > 0)
				{
					result.Add(Substring(index, charLength));
					charLength = 0;
				}

				index = i + 1;
			}
			else
			{
				++charLength;
			}
		}

		if (result.GetCount() == 0)
		{
			result.Add(*this);
		}
		else if (charLength > 0)
		{
			result.Add(Substring(index, charLength));
		}

		return result;
	}


	NS_NODISCARD_INLINE int ToInt() const noexcept
	{
		return nsPlatform::String_ToInt(CharArray.GetData());
	}


	NS_NODISCARD_INLINE float ToFloat() const noexcept
	{
		return nsPlatform::String_ToFloat(CharArray.GetData());
	}


	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return GetLength() == 0;
	}


public:
	NS_NODISCARD static NS_CORE_API nsString Format(const char* format, ...) noexcept;
	NS_NODISCARD static NS_CORE_API nsString FromBool(bool bValue) noexcept;
	NS_NODISCARD static NS_CORE_API nsString FromInt(int value) noexcept;
	NS_NODISCARD static NS_CORE_API nsString FromFloat(float value, int precision = 3) noexcept;


public:
	NS_INLINE nsString& operator=(const nsString& rhs) noexcept
	{
		if (this != &rhs)
		{
			//nsPlatform::Output("nsString copy assignment\n");
			CopyChars(*rhs);
		}

		return *this;
	}


	NS_INLINE nsString& operator=(nsString&& rhs) noexcept
	{
		if (this != &rhs)
		{
			//nsPlatform::Output("nsString move assignment\n");
			CharArray = std::move(rhs.CharArray);
		}

		return *this;
	}


	NS_INLINE nsString& operator=(const char* rhs) noexcept
	{
		if (CharArray.GetData() != rhs)
		{
			//nsPlatform::Output("nsString const char* assignment\n");
			CopyChars(rhs);
		}

		return *this;
	}


	NS_INLINE char* operator*() noexcept
	{
		return CharArray.GetData();
	}


	NS_INLINE const char* operator*() const noexcept
	{
		return CharArray.GetData();
	}


	NS_INLINE char& operator[](int index) noexcept
	{
		return CharArray[index];
	}


	NS_INLINE const char& operator[](int index) const noexcept
	{
		return CharArray[index];
	}


	NS_INLINE bool operator==(const nsString& rhs) const noexcept
	{
		//nsPlatform::Output("nsString operator==(nsString)\n");
		return nsPlatform::String_Compare(CharArray.GetData(), *rhs);
	}


	NS_INLINE bool operator==(const char* rhs) const noexcept
	{
		//nsPlatform::Output("nsString operator==(const char*)\n");
		return nsPlatform::String_Compare(CharArray.GetData(), rhs);
	}


	NS_INLINE bool operator!=(const nsString& rhs) const noexcept
	{
		return !nsPlatform::String_Compare(CharArray.GetData(), *rhs);
	}


	NS_INLINE bool operator!=(const char* rhs) const noexcept
	{
		return !nsPlatform::String_Compare(CharArray.GetData(), rhs);
	}


	NS_INLINE nsString operator+(const nsString& rhs) const noexcept
	{
		return Append(rhs);
	}


	NS_INLINE nsString& operator+=(const nsString& rhs) noexcept
	{
		AppendInPlace(rhs);
		return *this;
	}

};




class nsName
{
public:
	static constexpr int N = 32;
	static NS_CORE_API nsName NONE;

private:
	char Chars[N];


public:
	nsName(const nsName&) noexcept = default;
	nsName(nsName&&) noexcept = default;
	nsName& operator=(const nsName&) noexcept = default;
	nsName& operator=(nsName&&) noexcept = default;
	~nsName() noexcept = default;


	nsName() noexcept
		: Chars()
	{
	}


	nsName(const char* cstr) noexcept
		: Chars()
	{
		CopyChars(cstr);
	}


private:
	NS_INLINE void CopyChars(const char* cstr) noexcept
	{
		nsPlatform::Memory_Set(Chars, 0, N);
		int len = nsPlatform::String_Length(cstr);

		if (len == 0)
		{
			return;
		}

		if (len > N - 1)
		{
			len = N - 1;
		}

		nsPlatform::Memory_Copy(Chars, cstr, len);
	}


public:
	NS_NODISCARD static NS_CORE_API nsName Format(const char* format, ...) noexcept;


	NS_NODISCARD_INLINE int GetLength() const noexcept
	{
		return nsPlatform::String_Length(Chars);
	}


	NS_NODISCARD_INLINE int ToInt() const noexcept
	{
		return nsPlatform::String_ToInt(Chars);
	}


	NS_NODISCARD_INLINE float ToFloat() const noexcept
	{
		return nsPlatform::String_ToFloat(Chars);
	}


public:
	NS_INLINE nsName& operator=(const char* cstr) noexcept
	{
		if (Chars != cstr)
		{
			CopyChars(cstr);
		}

		return *this;
	}


	NS_INLINE char* operator*() noexcept
	{
		return Chars;
	}


	NS_INLINE const char* operator*() const noexcept
	{
		return Chars;
	}


	NS_INLINE bool operator==(const nsName& rhs) const noexcept
	{
		return nsPlatform::String_Compare(Chars, rhs.Chars, true);
	}


	NS_INLINE bool operator==(const char* rhs) const noexcept
	{
		return nsPlatform::String_Compare(Chars, rhs, true);
	}


	NS_INLINE bool operator!=(const char* rhs) const noexcept
	{
		return !nsPlatform::String_Compare(Chars, rhs, true);
	}


	NS_INLINE bool operator!=(const nsName& rhs) const noexcept
	{
		return !nsPlatform::String_Compare(Chars, rhs.Chars, true);
	}

};



NS_NODISCARD_INLINE uint64 ns_GetHash(const nsString& string) noexcept
{
	return nsPlatform::String_Hash(*string);
}


NS_NODISCARD_INLINE uint64 ns_GetHash(const nsName& name) noexcept
{
	return nsPlatform::String_Hash(*name);
}
