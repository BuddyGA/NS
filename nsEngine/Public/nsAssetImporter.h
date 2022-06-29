#pragma once

#include "nsTexture.h"



struct nsAssetImportOption_Image
{
	nsString SourceFile;
	nsETextureFormat Format;
	bool bGenerateMipMaps;
};



struct nsAssetImportOption_Model
{
	nsString SourceFile;
	float MeshScaleMultiplier;
	bool bImportMesh;
	bool bImportSkeleton;
	bool bImportAnimation;
	bool bImportTexture;
};



class NS_ENGINE_API nsAssetImporter
{
private:
	bool bInitialized;

public:
	void Initialize() noexcept;

	void ImportAssetFromImageFile(const nsAssetImportOption_Image& option, const nsString& dstFolderPath) noexcept;


private:
	void ImportAssetFromModelFile_GLB(const nsAssetImportOption_Model& option, const nsString& dstFolderPath);

public:
	void ImportAssetFromModelFile(const nsAssetImportOption_Model& option, const nsString& dstFolderPath) noexcept;

	NS_DECLARE_SINGLETON(nsAssetImporter)

};
