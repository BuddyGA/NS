#include "nsFileSystem.h"
#include <commdlg.h>


#define NS_ValidatePathLength(path) const int len = path.GetLength(); NS_Validate(len <= NS_PLATFORM_MAX_PATH)



bool nsFileSystem::FolderExists(const nsString& folderPath) noexcept
{
	if (folderPath.IsEmpty())
	{
		return false;
	}

	NS_ValidatePathLength(folderPath);

	WIN32_FIND_DATA fileData;
	HANDLE fileHandle = FindFirstFile(*folderPath, &fileData);
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

	const int error = CreateDirectory(*folderPath, NULL);
	return error != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}


bool nsFileSystem::FolderDelete(const nsString& folderPath) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to delete folder. Folder [%s] does not exists!"), *folderPath);
		return false;
	}

	const nsString search = folderPath + TEXT("/*");
	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFile(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == TEXT(".") || fileName == TEXT(".."))
			{
				continue;
			}

			const nsString file = folderPath + "/" + fileName;

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FolderDelete(file);
				RemoveDirectory(*file);
			}
			else
			{
				DeleteFile(*file);
			}

		}
		while (FindNextFile(fileHandle, &fileData));

		RemoveDirectory(*folderPath);
	}

	FindClose(fileHandle);

	return true;
}


void nsFileSystem::FolderIterate(nsTArray<nsString>& outFolders, const nsString& folderPath, bool bIncludeSubfolders) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to iterate folder. Folder [%s] does not exists!"), *folderPath);
		return;
	}

	const nsString search = folderPath + TEXT("/*");
	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFile(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == TEXT(".") || fileName == TEXT(".."))
			{
				continue;
			}

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				const nsString subDir = folderPath + TEXT("/") + fileName;
				outFolders.Add(subDir);

				if (bIncludeSubfolders)
				{
					FolderIterate(outFolders, subDir, bIncludeSubfolders);
				}
			}

		}
		while (FindNextFile(fileHandle, &fileData));
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
	nsPlatformFileHandle fileHandle = nsPlatform::File_Open(*filePath, nsEPlatformFileOpenMode::WRITE_OVERWRITE_EXISTING);

	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to create file [%s]!"), *filePath);
		return false;
	}

	nsPlatform::File_Close(fileHandle);

	return true;
}


bool nsFileSystem::FileDelete(const nsString& filePath) noexcept
{
	if (!FileExists(filePath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to delete file. File [%s] does not exists!"), *filePath);
		return false;
	}

	return (bool)DeleteFile(*filePath);
}


bool nsFileSystem::FileCopy(const nsString& srcFilePath, const nsString& dstFilePath) noexcept
{
	if (!FileExists(srcFilePath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to copy file. Source file [%s] does not exists!"), *srcFilePath);
		return false;
	}

	if (dstFilePath.IsEmpty())
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to copy file. Destination file path is empty!"));
		return false;
	}

	NS_ValidatePathLength(dstFilePath);
	
	return (bool)CopyFile(*srcFilePath, *dstFilePath, false);
}


bool nsFileSystem::FileMove(const nsString& srcFilePath, const nsString& dstFilePath) noexcept
{
	if (!FileExists(srcFilePath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to move file. Source file [%s] does not exists!"), *srcFilePath);
		return false;
	}

	if (dstFilePath.IsEmpty())
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to move file. Destination file path is empty!"));
		return false;
	}

	NS_ValidatePathLength(dstFilePath);

	return (bool)MoveFile(*srcFilePath, *dstFilePath);
}


void nsFileSystem::FileIterate(nsTArray<nsString>& outFiles, const nsString& folderPath, bool bIncludeSubfolders, const nsString& optExtension) noexcept
{
	if (!FolderExists(folderPath))
	{
		NS_LogWarning(nsSystemLog, TEXT("Fail to iterate file in folder. Folder [%s] does not exists!"), *folderPath);
		return;
	}

	const nsString search = folderPath + TEXT("/*");
	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFile(*search, &fileData);
	nsString fileName;

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileName = fileData.cFileName;

			if (fileName == TEXT(".") || fileName == TEXT(".."))
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
		while (FindNextFile(fileHandle, &fileData));
	}

	FindClose(fileHandle);
}


nsString nsFileSystem::OpenFileDialog_ImportAsset() noexcept
{
	wchar_t filePath[1024];
	nsPlatform::Memory_Zero(filePath, sizeof(filePath));

	OPENFILENAME ofn{};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrInitialDir = nsPlatform::GetDirectoryPath();
	ofn.lpstrFilter = TEXT("Model Files (GLB FBX)\0*.glb;*.fbx\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);

	GetOpenFileName(&ofn);

	return filePath;
}
