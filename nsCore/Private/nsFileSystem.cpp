#include "nsFileSystem.h"


static nsLogCategory FileSystemLog(TEXT("nsFileSystemLog"), nsELogVerbosity::LV_WARNING);



bool nsFileSystem::FileReadBinary(const nsString& filePath, nsTArray<uint8>& outResult) noexcept
{
	if (!nsPlatform::File_Exists(*filePath))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read binary file. File [%s] does not exists!"), *filePath);
		return false;
	}

	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::READ);
	const int fileSize = nsPlatform::File_GetSize(fileHandle);
	outResult.Resize(fileSize);
	
	if (!nsPlatform::File_Read(fileHandle, outResult.GetData(), fileSize))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read data from file [%s]!"), *filePath);
		nsPlatform::File_Close(fileHandle);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileReadText(const nsString& filePath, nsString& outResult) noexcept
{
	if (!nsPlatform::File_Exists(*filePath))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read text file. File [%s] does not exists!"), *filePath);
		return false;
	}

	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::READ);
	const int fileSize = nsPlatform::File_GetSize(fileHandle);
	outResult.Resize(fileSize);

	if (!nsPlatform::File_Read(fileHandle, *outResult, fileSize))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read data from file [%s]!"), *filePath);
		nsPlatform::File_Close(fileHandle);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileReadText(const nsString& filePath, nsTArray<char>& outResult) noexcept
{
	if (!nsPlatform::File_Exists(*filePath))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read text file. File [%s] does not exists!"), *filePath);
		return false;
	}

	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::READ);
	const int fileSize = nsPlatform::File_GetSize(fileHandle);
	outResult.Resize(fileSize);

	if (!nsPlatform::File_Read(fileHandle, outResult.GetData(), fileSize))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to read data from file [%s]!"), *filePath);
		nsPlatform::File_Close(fileHandle);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileWriteBinary(const nsString& filePath, const uint8* data, int dataSize) noexcept
{
	if (data == nullptr || dataSize == 0)
	{
		NS_LogWarning(FileSystemLog, TEXT("Fail to write binary to file [%s]. <data> is NULL or <dataSize> is 0!"), *filePath);
		return false;
	}

	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING);

	if (!nsPlatform::File_Write(fileHandle, data, dataSize))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to write data to file [%s]!"), *filePath);
		nsPlatform::File_Close(fileHandle);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileWriteText(const nsString& filePath, const char* text) noexcept
{
	const int length = nsPlatform::String_Length(text);

	if (length == 0)
	{
		NS_LogWarning(FileSystemLog, TEXT("Fail to write text to file [%s]. Char length is 0!"), *filePath);
		return false;
	}

	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING);

	if (!nsPlatform::File_Write(fileHandle, text, length))
	{
		NS_LogError(FileSystemLog, TEXT("Fail to write data to file [%s]!"), *filePath);
		nsPlatform::File_Close(fileHandle);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


nsString nsFileSystem::FileGetPath(const nsString& file) noexcept
{
	if (file.IsEmpty())
	{
		return "";
	}

	const int length = file.GetLength();
	int lastTokenIndex = file.FindLast('/');

	if (lastTokenIndex == NS_ARRAY_INDEX_INVALID)
	{
		lastTokenIndex = file.FindLast('\\');
	}

	if (lastTokenIndex == NS_ARRAY_INDEX_INVALID)
	{
		return "";
	}

	return file.Substring(0, lastTokenIndex);
}


nsString nsFileSystem::FileGetName(const nsString& file) noexcept
{
	if (!file.IsEmpty())
	{
		int count = file.GetLength();
		int offset = 0;
		int lastTokenIndex = file.FindLast('/');

		if (lastTokenIndex == NS_ARRAY_INDEX_INVALID)
		{
			lastTokenIndex = file.FindLast('\\');
		}

		if (lastTokenIndex != NS_ARRAY_INDEX_INVALID)
		{
			offset = lastTokenIndex + 1;
		}

		const int dotIndex = file.FindLast('.');

		if (dotIndex != NS_ARRAY_INDEX_INVALID)
		{
			count = dotIndex - offset;
		}
		else
		{
			count = file.GetLength() - offset;
		}

		return file.Substring(offset, count);
	}

	return "";
}


nsString nsFileSystem::FileGetExtension(const nsString& file) noexcept
{
	nsString extension = TEXT("");

	if (!file.IsEmpty())
	{
		const int lastDotIndex = file.FindLast('.');

		if (lastDotIndex != NS_ARRAY_INDEX_INVALID)
		{
			const int count = file.GetLength() - lastDotIndex;
			extension = file.Substring(lastDotIndex, count);
		}
	}

	return extension;
}
