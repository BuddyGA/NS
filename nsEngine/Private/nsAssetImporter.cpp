#include "nsAssetImporter.h"
#include "nsAssetManager.h"
#include "nsFileSystem.h"
#include "nsConsole.h"
#include "nsMesh.h"



nsAssetImporter::nsAssetImporter() noexcept
	: bInitialized(false)
{

}


void nsAssetImporter::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	/*
	nsAssetImportOption_Image option;
	option.SourceFile = "../../../Assets/Textures/tex_debug_1024.png";
	option.Format = nsETextureFormat::COMPRESSED_BC3_RGBA;
	option.bGenerateMipMaps = true;
	ImportAssetFromImageFile(option, "Textures");
	*/

	nsAssetImportOption_Model importOptionModel{};

	//importOptionModel.SourceFile = "../../../Assets/Models/mesh_wall.glb";
	//importOptionModel.MeshScaleMultiplier = 100.0f;

	//importOptionModel.SourceFile = "../../../Assets/Models/mesh_boombox.glb";
	//importOptionModel.MeshScaleMultiplier = 1000.0f;

	//importOptionModel.SourceFile = "../../../Assets/Models/mesh_cube_two_mats.glb";
	//importOptionModel.MeshScaleMultiplier = 100.0f;


	// Test import mesh, skeleton, animation
	//importOptionModel.SourceFile = "../../../Assets/Models/LowPolyChar.glb";
	//importOptionModel.SourceFile = "../../../Assets/Models/LowPolyChar_rigged_animations.glb";
	//importOptionModel.MeshScaleMultiplier = 1.0f;
	//importOptionModel.bImportMesh = true;
	//importOptionModel.bImportSkeleton = true;
	//importOptionModel.bImportAnimation = true;
	//ImportAssetFromModelFile(importOptionModel, "Models");

	bInitialized = true;
}




// =============================================================================================================================================================== //
// IMPORT FROM IMAGE FILE
// =============================================================================================================================================================== //
#include "Compressonator.h"



static NS_NODISCARD_INLINE CMP_FORMAT ns_ConvertTextureFormatToCMP(nsETextureFormat format)
{
	switch (format)
	{
		case nsETextureFormat::UNCOMPRESSED_RGBA:		return CMP_FORMAT_RGBA_8888;
		case nsETextureFormat::UNCOMPRESSED_BGRA:		return CMP_FORMAT_BGRA_8888;
		case nsETextureFormat::UNCOMPRESSED_R:			return CMP_FORMAT_R_8;
		case nsETextureFormat::COMPRESSED_BC3_RGBA:		return CMP_FORMAT_BC3;
		case nsETextureFormat::COMPRESSED_BC4_R:		return CMP_FORMAT_BC4;
		case nsETextureFormat::COMPRESSED_BC5_RG:		return CMP_FORMAT_BC5;
		case nsETextureFormat::COMPRESSED_BC6_H:		return CMP_FORMAT_BC6H;
		case nsETextureFormat::COMPRESSED_BC7_RGBA:		return CMP_FORMAT_BC7;
		default: break;
	}

	NS_ValidateV(0, "Unsupported texture format!");
	return CMP_FORMAT_Unknown;
}


void nsAssetImporter::ImportAssetFromImageFile(const nsAssetImportOption_Image& option, const nsString& dstFolderPath) noexcept
{
	if (!nsFileSystem::FileExists(option.SourceFile))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import texture. Source file [%s] does not exists!", *option.SourceFile);
		return;
	}

	if (!nsFileSystem::FolderCreate(dstFolderPath))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import texture from file [%s]. Fail to create destination folder [%s]", *option.SourceFile, *dstFolderPath);
		return;
	}

	const nsName fileName = nsFileSystem::FileGetName(option.SourceFile);
	const nsName fileExt = nsFileSystem::FileGetExtension(option.SourceFile);

	if (!(fileExt == ".bmp" || fileExt == ".png" || fileExt == ".tga"))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import texture from file [%s]. File format not supported!", *option.SourceFile);
		return;
	}

	CMP_MipSet srcMipSet = {};

	if (CMP_LoadTexture(*option.SourceFile, &srcMipSet) != CMP_OK)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import texture from file [%s]. Compressonator: Load texture data failed!", *option.SourceFile);
		return;
	}

	if (srcMipSet.m_nMipLevels <= 1 && option.bGenerateMipMaps)
	{
		const int mipMinSize = CMP_CalcMinMipSize(srcMipSet.dwHeight, srcMipSet.dwWidth, NS_ENGINE_TEXTURE_MAX_MIP);
		CMP_GenerateMIPLevels(&srcMipSet, mipMinSize);
	}

	CMP_MipSet dstMipSet = {};
	const bool bCompressed = !(option.Format == nsETextureFormat::UNCOMPRESSED_RGBA || option.Format == nsETextureFormat::UNCOMPRESSED_BGRA || option.Format == nsETextureFormat::UNCOMPRESSED_R);

	if (bCompressed)
	{
		KernelOptions options = {};
		options.format = ns_ConvertTextureFormatToCMP(option.Format);
		options.fquality = 0.1f;
		options.threads = 0;

		if (CMP_ProcessTexture(&srcMipSet, &dstMipSet, options, nullptr) != CMP_OK)
		{
			NS_CONSOLE_Warning(AssetLog, "Fail to import texture from file [%s]. Compressonator: Fail to process texture compression!", *option.SourceFile);
			CMP_FreeMipSet(&srcMipSet);
			CMP_FreeMipSet(&dstMipSet);
			return;
		}

		NS_Assert(dstMipSet.m_nMipLevels >= 1);
	}

	const CMP_MipSet* useMipSet = bCompressed ? &dstMipSet : &srcMipSet;

	nsTextureManager& textureManager = nsTextureManager::Get();
	const nsTextureID texture = textureManager.CreateTexture2D(fileName, option.Format, useMipSet->m_nWidth, useMipSet->m_nHeight);

	const CMP_MipLevelTable& mipLevelTable0 = useMipSet->m_pMipLevelTable[0];
	textureManager.UpdateTextureMipData(texture, 0, mipLevelTable0->m_pbData, mipLevelTable0->m_dwLinearSize);

	for (int m = 1; m < useMipSet->m_nMipLevels; ++m)
	{
		const CMP_MipLevelTable& mipLevelTable = useMipSet->m_pMipLevelTable[m];
		const int mipIndex = textureManager.AddTextureMipLevel(texture, mipLevelTable->m_nWidth, mipLevelTable->m_nHeight);
		NS_Assert(mipIndex == m);
		textureManager.UpdateTextureMipData(texture, m, mipLevelTable->m_pbData, mipLevelTable->m_dwLinearSize);
	}

	CMP_FreeMipSet(&srcMipSet);
	CMP_FreeMipSet(&dstMipSet);

	NS_CONSOLE_Log(AssetLog, "Imported texture from source file: %s", *option.SourceFile);

	nsAssetManager::Get().SaveTextureAsset(fileName, texture, dstFolderPath, false);
}






// =============================================================================================================================================================== //
// IMPORT FROM MODEL FILE
// =============================================================================================================================================================== //
#include "nsAssetImporter_GLB.cpp"


void nsAssetImporter::ImportAssetFromModelFile(const nsAssetImportOption_Model& option, const nsString& dstFolderPath) noexcept
{
	if (!nsFileSystem::FileExists(option.SourceFile))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import model. Source file [%s] does not exists!", *option.SourceFile);
		return;
	}

	const nsName fileName = nsFileSystem::FileGetName(option.SourceFile);
	const nsName fileExt = nsFileSystem::FileGetExtension(option.SourceFile);

	if (!(fileExt == ".glb" || fileExt == ".gltf" || fileExt == ".fbx"))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import model from file [%s]. File format not supported!", *option.SourceFile);
		return;
	}

	if (fileExt == ".glb")
	{
		ImportAssetFromModelFile_GLB(option, dstFolderPath);
	}
}
