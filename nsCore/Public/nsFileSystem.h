#pragma once

#include "nsString.h"



namespace nsFileSystem
{
	NS_NODISCARD extern NS_CORE_API bool FolderExists(const nsString& folderPath) noexcept;

	extern NS_CORE_API bool FolderCreate(const nsString& folderPath) noexcept;

	extern NS_CORE_API bool FolderDelete(const nsString& folderPath) noexcept;

	extern NS_CORE_API void FolderIterate(nsTArray<nsString>& outFolders, const nsString& folderPath, bool bIncludeSubfolders) noexcept;

	NS_NODISCARD extern NS_CORE_API bool FileExists(const nsString& filePath) noexcept;

	extern NS_CORE_API bool FileCreate(const nsString& filePath) noexcept;

	extern NS_CORE_API bool FileDelete(const nsString& filePath) noexcept;

	extern NS_CORE_API bool FileCopy(const nsString& srcFilePath, const nsString& dstFilePath) noexcept;

	extern NS_CORE_API bool FileMove(const nsString& srcFilePath, const nsString& dstFilePath) noexcept;

	extern NS_CORE_API void FileIterate(nsTArray<nsString>& outFiles, const nsString& folderPath, bool bIncludeSubfolders, const nsString& optExtension = "") noexcept;

	extern NS_CORE_API bool FileReadBinary(const nsString& filePath, nsTArray<uint8>& outResult) noexcept;

	extern NS_CORE_API bool FileReadText(const nsString& filePath, nsString& outResult) noexcept;

	extern NS_CORE_API bool FileWriteBinary(const nsString& filePath, const uint8* data, int dataSize) noexcept;
	
	NS_INLINE bool FileWriteBinary(const nsString& filePath, const nsTArray<uint8>& bytes) noexcept
	{
		return FileWriteBinary(filePath, bytes.GetData(), bytes.GetCount());
	}

	extern NS_CORE_API bool FileWriteText(const nsString& filePath, const char* text) noexcept;
	
	NS_INLINE bool FileWriteText(const nsString& filePath, const nsString& text) noexcept
	{
		return FileWriteText(filePath, *text);
	}

	// Get folder path of the file
	NS_NODISCARD extern NS_CORE_API nsString FileGetPath(const nsString& file) noexcept;

	// Get file name without extension
	NS_NODISCARD extern NS_CORE_API nsName FileGetName(const nsString& file) noexcept;

	// Get file extension with dot
	NS_NODISCARD extern NS_CORE_API nsName FileGetExtension(const nsString& file) noexcept;
	
};
