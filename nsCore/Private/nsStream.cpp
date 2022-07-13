#include "nsStream.h"



nsBinaryStreamWriter::nsBinaryStreamWriter() noexcept
{
	Buffer.Reserve(NS_MEMORY_SIZE_KiB(4));
}


void nsBinaryStreamWriter::SerializeData(void* data, int dataSize) noexcept
{
	NS_Assert(data);
	NS_Assert(dataSize > 0);

	const int index = Buffer.GetCount();
	Buffer.Resize(index + dataSize);
	nsPlatform::Memory_Copy(Buffer.GetData() + index, data, dataSize);
}




nsBinaryStreamReader::nsBinaryStreamReader(nsTArray<uint8> data) noexcept
	: Buffer(std::move(data))
	, Offset(0)
{
}


void nsBinaryStreamReader::SerializeData(void* data, int dataSize) noexcept
{
	if (Buffer.IsEmpty())
	{
		return;
	}

	NS_Assert(data);
	NS_Assert(dataSize > 0);
	NS_Assert(Offset + dataSize <= Buffer.GetCount());

	nsPlatform::Memory_Copy(data, Buffer.GetData() + Offset, dataSize);
	Offset += dataSize;
}



nsFileStreamReader::nsFileStreamReader() noexcept
	: Handle(NULL)
{
}


nsFileStreamReader::nsFileStreamReader(const nsString& file) noexcept
	: Handle(NULL)
{
	Open(file);
}


nsFileStreamReader::~nsFileStreamReader() noexcept
{
	Close();
}


void nsFileStreamReader::SerializeData(void* data, int dataSize) noexcept
{
	NS_Assert(data);
	NS_Assert(dataSize > 0);

	nsPlatform::File_Read(Handle, data, dataSize);
}


void nsFileStreamReader::Open(const nsString& file) noexcept
{
	Handle = nsPlatform::File_Open(*file, nsEPlatformFileOpenMode::READ);
	NS_ValidateV(Handle, TEXT("Fail to open file!"), *file);
}


void nsFileStreamReader::Close() noexcept
{
	if (Handle)
	{
		nsPlatform::File_Close(Handle);
		Handle = NULL;
	}
}
