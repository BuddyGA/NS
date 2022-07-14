#pragma once

#include "nsContainer.h"


#define NS_STRING_MAX_LENGTH	NS_ARRAY_INDEX_LAST


class nsString
{
private:
	nsTArray<wchar_t> CharArray;


public:
	nsString() noexcept = default;
	~nsString() noexcept = default;


	nsString(const nsString& other) noexcept
		: CharArray(other.CharArray)
	{
	}


	nsString(nsString&& other) noexcept
		: CharArray(std::move(other.CharArray))
	{
	}


	nsString(const wchar_t* wstr) noexcept
	{
		CopyWideChars(wstr);
	}


	nsString(const char* cstr) noexcept
	{
		CopyChars(cstr);
	}


private:
	NS_INLINE void CopyWideChars(const wchar_t* src) noexcept
	{
		CharArray.Clear();
		const int len = nsPlatform::String_Length(src);

		if (len == 0)
		{
			return;
		}

		Resize(len);
		nsPlatform::String_Copy(CharArray.GetData(), src);
	}


	NS_INLINE void CopyChars(const char* src) noexcept
	{
		CharArray.Clear();
		const int len = nsPlatform::String_Length(src);

		if (len == 0)
		{
			return;
		}

		Resize(len);
		nsPlatform::String_ConvertToWide(CharArray.GetData(), src, len);
	}


public:
	NS_INLINE void Reserve(int newCapacity) noexcept
	{
		CharArray.Reserve(newCapacity);
	}


	NS_INLINE void Resize(int newLength) noexcept
	{
		CharArray.Resize(newLength + 1);
	}


	NS_NODISCARD_INLINE int GetLength() const noexcept
	{
		const int count = CharArray.GetCount();
		return count > 0 ? count - 1 : 0;
	}


	NS_NODISCARD_INLINE int Find(wchar_t c) const noexcept
	{
		return CharArray.Find(c);
	}


	NS_NODISCARD_INLINE int FindLast(wchar_t c) const noexcept
	{
		return CharArray.FindLast(c);
	}


	NS_INLINE void Add(wchar_t c) noexcept
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


	NS_INLINE int InsertAt(const wchar_t* cstr, int index = NS_STRING_MAX_LENGTH) noexcept
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
			nsPlatform::Memory_Move(CharArray.GetData() + (index + len), CharArray.GetData() + index, sizeof(wchar_t) * (currentLength - index));
			nsPlatform::Memory_Copy(CharArray.GetData() + index, cstr, sizeof(wchar_t) * len);
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


	NS_INLINE void RemoveCharInPlace(wchar_t c) noexcept
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


	NS_NODISCARD_INLINE nsString RemoveChar(wchar_t c) const noexcept
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


	NS_INLINE void ReplaceInPlace(wchar_t c, wchar_t with) noexcept
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


	NS_NODISCARD_INLINE nsString Replace(wchar_t c, wchar_t with) const noexcept
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
		nsPlatform::Memory_Copy(*temp, &CharArray[index], sizeof(wchar_t) * length);

		return temp;
	}


	NS_NODISCARD_INLINE nsTArray<nsString> Splits(wchar_t separator) const noexcept
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


	// Get total memory size in bytes including the null terminator
	NS_NODISCARD_INLINE int GetTotalSizeBytes() const noexcept
	{
		return sizeof(wchar_t) * CharArray.GetCount();
	}


	// Get allocated memory size in bytes NOT including the null terminator
	NS_NODISCARD_INLINE int GetAllocatedSizeBytes() const noexcept
	{
		return sizeof(wchar_t) * GetLength();
	}


public:
	template<typename...TVarArgs>
	NS_NODISCARD static NS_INLINE nsString Format(const wchar_t* format, TVarArgs&&... args) noexcept
	{
		wchar_t buffer[2048];
		const int n = nsPlatform::String_Format(buffer, 2048, format, args...);
		NS_Assert(n >= 0);

		return buffer;
	}


	NS_NODISCARD static NS_INLINE nsString FromBool(bool value) noexcept
	{
		wchar_t buffer[8];
		nsPlatform::String_Format(buffer, 8, TEXT("%s"), value ? TEXT("True") : TEXT("False"));
		return buffer;
	}


	NS_NODISCARD static NS_INLINE nsString FromInt(int value) noexcept
	{
		wchar_t buffer[16];
		nsPlatform::String_Format(buffer, 16, TEXT("%i"), value);
		return buffer;
	}


	NS_NODISCARD static NS_INLINE nsString FromFloat(float value, int precision = 3) noexcept
	{
		wchar_t buffer[32];
		nsPlatform::String_Format(buffer, 32, TEXT("%.*f"), precision, value);
		return buffer;
	}


public:
	NS_INLINE nsString& operator=(const nsString& rhs) noexcept
	{
		if (this != &rhs)
		{
			CopyWideChars(*rhs);
		}

		return *this;
	}


	NS_INLINE nsString& operator=(nsString&& rhs) noexcept
	{
		if (this != &rhs)
		{
			CharArray = std::move(rhs.CharArray);
		}

		return *this;
	}


	NS_INLINE nsString& operator=(const wchar_t* rhs) noexcept
	{
		if (CharArray.GetData() != rhs)
		{
			CopyWideChars(rhs);
		}

		return *this;
	}


	NS_INLINE nsString& operator=(const char* rhs) noexcept
	{
		CopyChars(rhs);
		return *this;
	}


	NS_INLINE wchar_t* operator*() noexcept
	{
		return CharArray.GetData();
	}


	NS_INLINE const wchar_t* operator*() const noexcept
	{
		return CharArray.GetData();
	}


	NS_INLINE wchar_t& operator[](int index) noexcept
	{
		return CharArray[index];
	}


	NS_INLINE const wchar_t& operator[](int index) const noexcept
	{
		return CharArray[index];
	}


	NS_INLINE bool operator==(const nsString& rhs) const noexcept
	{
		return nsPlatform::String_Compare(CharArray.GetData(), *rhs, false);
	}


	NS_INLINE bool operator==(const wchar_t* rhs) const noexcept
	{
		return nsPlatform::String_Compare(CharArray.GetData(), rhs, false);
	}


	NS_INLINE bool operator!=(const nsString& rhs) const noexcept
	{
		return !nsPlatform::String_Compare(CharArray.GetData(), *rhs, false);
	}


	NS_INLINE bool operator!=(const wchar_t* rhs) const noexcept
	{
		return !nsPlatform::String_Compare(CharArray.GetData(), rhs, false);
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


	nsName(const wchar_t* wstr) noexcept
	{
		CopyWideChars(wstr);
	}


private:
	NS_INLINE void CopyChars(const char* cstr) noexcept
	{
		nsPlatform::Memory_Zero(Chars, N);
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


	NS_INLINE void CopyWideChars(const wchar_t* wstr) noexcept
	{
		nsPlatform::Memory_Zero(Chars, N);
		int len = nsPlatform::String_Length(wstr);

		if (len == 0)
		{
			return;
		}

		if (len > N - 1)
		{
			len = N - 1;
		}

		nsPlatform::String_ConvertToChar(Chars, wstr, len);
	}


public:
	template<typename...TVarArgs>
	NS_NODISCARD static NS_INLINE nsName Format(const char* format, TVarArgs... args) noexcept
	{
		nsName Name;
		nsPlatform::String_Format(Name.Chars, N, format, args...);
		return Name;
	}


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


	NS_NODISCARD_INLINE nsString ToString() const noexcept
	{
		wchar_t buffer[64] = {};
		nsPlatform::String_ConvertToWide(buffer, Chars, N);

		return buffer;
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


	NS_INLINE nsName& operator=(const wchar_t* rhs) noexcept
	{
		CopyWideChars(rhs);
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
