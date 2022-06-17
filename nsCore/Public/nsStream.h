#pragma once

#include "nsString.h"



class NS_CORE_API nsStream
{
	NS_DECLARE_NOCOPY(nsStream)

public:
	nsStream() noexcept {}
	virtual ~nsStream() noexcept {}

public:
	virtual void SerializeData(void* data, int dataSize) noexcept = 0;
	virtual bool IsLoading() const noexcept = 0;


	template<typename T>
	NS_INLINE nsStream& operator|(T& rhs) noexcept
	{
		SerializeData(&rhs, sizeof(T));
		return *this;
	}


	template<typename T>
	NS_INLINE nsStream& operator|(nsTArray<T>& rhs) noexcept
	{
		int count = rhs.GetCount();
		SerializeData(&count, sizeof(int));

		if (count > 0)
		{
			if (IsLoading())
			{
				rhs.Resize(count);
			}

			for (int i = 0; i < count; ++i)
			{
				*this | rhs[i];
			}
		}

		return *this;
	}


	template<typename T, int N = 2>
	NS_INLINE nsStream& operator|(nsTArrayInline<T, N>& rhs) noexcept
	{
		int count = rhs.GetCount();
		SerializeData(&count, sizeof(int));

		if (count > 0)
		{
			if (IsLoading())
			{
				rhs.Resize(count);
			}

			for (int i = 0; i < count; ++i)
			{
				*this | rhs[i];
			}
		}

		return *this;
	}


	NS_INLINE nsStream& operator|(nsString& rhs) noexcept
	{
		int length = rhs.GetLength();
		SerializeData(&length, sizeof(int));

		if (length > 0)
		{
			if (IsLoading())
			{
				rhs.Resize(length);
			}

			SerializeData(*rhs, length);
		}
		
		return *this;
	}


	NS_INLINE nsStream& operator|(nsName& rhs) noexcept
	{
		int length = rhs.GetLength();
		SerializeData(&length, sizeof(int));

		if (length > 0)
		{
			NS_Assert(length < nsName::N);
			SerializeData(*rhs, length);
		}

		return *this;
	}

};



class NS_CORE_API nsBinaryStreamWriter : public nsStream
{
private:
	nsTArray<uint8> Buffer;


public:
	nsBinaryStreamWriter() noexcept;
	virtual void SerializeData(void* data, int dataSize) noexcept override;
	virtual bool IsLoading() const noexcept override { return false; }


	NS_NODISCARD_INLINE const nsTArray<uint8>& GetBuffer() const noexcept
	{
		return Buffer;
	}


	NS_NODISCARD_INLINE const uint8* GetBufferData() const noexcept
	{
		return Buffer.GetData();
	}


	NS_NODISCARD_INLINE int GetBufferSize() const noexcept
	{
		return Buffer.GetCount();
	}

};



class NS_CORE_API nsBinaryStreamReader : public nsStream
{
private:
	nsTArray<uint8> Buffer;
	int Offset;


public:
	nsBinaryStreamReader(nsTArray<uint8> data) noexcept;
	virtual void SerializeData(void* data, int dataSize) noexcept override;
	virtual bool IsLoading() const noexcept override { return true; }


	NS_NODISCARD_INLINE const uint8* GetBufferData() const noexcept
	{
		return Buffer.GetData();
	}


	NS_NODISCARD_INLINE int GetBufferSize() const noexcept
	{
		return Buffer.GetCount();
	}


	NS_NODISCARD_INLINE int GetCurrentOffset() const noexcept
	{
		return Offset;
	}

};




class NS_CORE_API nsFileStreamReader : public nsStream
{
private:
	nsFileHandle Handle;


public:
	nsFileStreamReader() noexcept;
	nsFileStreamReader(const nsString& file) noexcept;
	~nsFileStreamReader() noexcept;
	virtual void SerializeData(void* data, int dataSize) noexcept override;
	virtual bool IsLoading() const noexcept override { return true; }

	void Open(const nsString& file) noexcept;
	void Close() noexcept;

};
