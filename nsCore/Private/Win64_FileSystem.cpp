#include "nsFileSystem.h"


#define NS_ValidatePathLength(path) const int len = path.GetLength(); NS_Validate(len <= NS_PLATFORM_MAX_PATH)



bool nsFileSystem::FolderExists(const nsString& folderPath) noexcept
{
	if (folderPath.IsEmpty())
	{
		return false;
	}

	NS_ValidatePathLength(folderPath);

	WIN32_FIND_DATAA fileData;
	HANDLE fileHandle = FindFirstFileA(*folderPath, &fileData);
	return (fileHandle != INVALID_HANDLE_VALUE && (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}


bool nsFileSystem::FolderCreate(const nsString& folderPath) noexcept
{
	if (folderPath.IsEmpty())
	{
		return false;
	}

	if (FolderExists(folderPath))
	{
		return true;
	}

	NS_ValidatePathLength(folderPath);

	const int error = CreateDirectoryA(*folderPath, NULL);
	return error != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}


bool nsFileSystem::FolderDelete(const nsString& folderPath) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, "Fail to delete folder. Folder [%s] does not exists!", *folderPath);
		return false;
	}

	const nsString search = folderPath + "/*";
	WIN32_FIND_DATAA fileData{};
	HANDLE fileHandle = FindFirstFileA(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == "." || fileName == "..")
			{
				continue;
			}

			const nsString file = folderPath + "/" + fileName;

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FolderDelete(file);
				RemoveDirectoryA(*file);
			}
			else
			{
				DeleteFileA(*file);
			}

		}
		while (FindNextFileA(fileHandle, &fileData));

		RemoveDirectoryA(*folderPath);
	}

	FindClose(fileHandle);

	return true;
}


void nsFileSystem::FolderIterate(nsTArray<nsString>& outFolders, const nsString& folderPath, bool bIncludeSubfolders) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, "Fail to iterate folder. Folder [%s] does not exists!", *folderPath);
		return;
	}

	const nsString search = folderPath + "/*";
	WIN32_FIND_DATAA fileData{};
	HANDLE fileHandle = FindFirstFileA(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == "." || fileName == "..")
			{
				continue;
			}

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				const nsString subDir = folderPath + "/" + fileName;
				outFolders.Add(subDir);

				if (bIncludeSubfolders)
				{
					FolderIterate(outFolders, subDir, bIncludeSubfolders);
				}
			}

		}
		while (FindNextFileA(fileHandle, &fileData));
	}

	FindClose(fileHandle);
}



bool nsFileSystem::FileExists(const nsString& filePath) noexcept
{
	if (filePath.IsEmpty())
	{
		return false;
	}

	NS_ValidatePathLength(filePath);

	return nsPlatform::File_Exists(*filePath);
}


bool nsFileSystem::FileCreate(const nsString& filePath) noexcept
{
	nsFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING);

	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		NS_LogWarning(nsSystemLog, "Fail to create file [%s]!", *filePath);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileDelete(const nsString& filePath) noexcept
{
	if (!FileExists(filePath))
	{
		NS_LogWarning(nsSystemLog, "Fail to delete file. File [%s] does not exists!", *filePath);
		return false;
	}

	return (bool)DeleteFileA(*filePath);
}


bool nsFileSystem::FileCopy(const nsString& srcFilePath, const nsString& dstFilePath) noexcept
{
	if (!FileExists(srcFilePath))
	{
		NS_LogWarning(nsSystemLog, "Fail to copy file. Source file [%s] does not exists!", *srcFilePath);
		return false;
	}

	if (dstFilePath.IsEmpty())
	{
		NS_LogWarning(nsSystemLog, "Fail to copy file. Destination file path is empty!");
		return false;
	}

	NS_ValidatePathLength(dstFilePath);
	
	return (bool)CopyFileA(*srcFilePath, *dstFilePath, false);
}


bool nsFileSystem::FileMove(const nsString& srcFilePath, const nsString& dstFilePath) noexcept
{
	if (!FileExists(srcFilePath))
	{
		NS_LogWarning(nsSystemLog, "Fail to move file. Source file [%s] does not exists!", *srcFilePath);
		return false;
	}

	if (dstFilePath.IsEmpty())
	{
		NS_LogWarning(nsSystemLog, "Fail to move file. Destination file path is empty!");
		return false;
	}

	NS_ValidatePathLength(dstFilePath);

	return (bool)MoveFileA(*srcFilePath, *dstFilePath);
}


void nsFileSystem::FileIterate(nsTArray<nsString>& outFiles, const nsString& folderPath, bool bIncludeSubfolders, const nsString& optExtension) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, "Fail to iterate file in folder. Folder [%s] does not exists!", *folderPath);
		return;
	}

	const nsString search = folderPath + "/*";
	WIN32_FIND_DATAA fileData{};
	HANDLE fileHandle = FindFirstFileA(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == "." || fileName == "..")
			{
				continue;
			}

			const nsString file = folderPath + "/" + fileName;

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (bIncludeSubfolders)
				{
					FileIterate(outFiles, file, bIncludeSubfolders, optExtension);
				}
			}
			else if (!optExtension.IsEmpty())
			{
				const nsString ext = *FileGetExtension(file);

				if (ext == optExtension)
				{
					outFiles.Add(file);
				}
			}
			else
			{
				outFiles.Add(file);
			}

		}
		while (FindNextFileA(fileHandle, &fileData));
	}

	FindClose(fileHandle);
}
