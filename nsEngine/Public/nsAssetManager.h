#pragma once

#include "nsAssetTypes.h"



class NS_ENGINE_API nsAssetManager
{
	enum AssetFlags : uint8
	{
		AssetFlag_None				= (0),
		AssetFlag_Loading			= (1 << 0),
		AssetFlag_Loaded			= (1 << 1),
		AssetFlag_PendingUnload		= (1 << 2),
		AssetFlag_Unloaded			= (1 << 3),
	};


	struct TextureAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<nsTextureID> Handles;
	};


	struct MaterialParameterTable
	{
		nsTMap<nsName, nsSharedTextureAsset> TextureParameterValues;
		nsTMap<nsName, float> ScalarParameterValues;
		nsTMap<nsName, nsVector4> VectorParameterValues;
	};
	
	struct MaterialAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<MaterialParameterTable> ParameterTables;
		nsTArray<nsMaterialID> Handles;
	};


	struct ModelAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<nsAssetModelMeshes> Meshes;
	};


private:
	bool bInitialized;

	nsString EngineAssetsPath;
	nsString GameAssetsPath;

	nsTArray<nsName> TextureAssetNames;
	nsTArray<nsString> TextureAssetPaths;
	nsTArray<uint8> TextureAssetFlags;
	nsTArray<int> TextureAssetRefs;
	nsTArray<nsTextureID> TextureAssets;

	nsTArray<nsName> MaterialAssetNames;
	nsTArray<nsString> MaterialAssetPaths;
	nsTArray<uint8> MaterialAssetFlags;
	nsTArray<int> MaterialAssetRefs;
	nsTArray<nsMaterialID> MaterialAssets;

	nsTArray<nsName> ModelAssetNames;
	nsTArray<nsString> ModelAssetPaths;
	nsTArray<uint8> ModelAssetFlags;
	nsTArray<int> ModelAssetRefs;
	nsTArray<nsAssetModelMeshes> ModelAssetMeshes;


public:
	void Initialize();

private:
	void RegisterAsset(const nsString& assetFile);

public:
	bool IsValidAssetFile(const nsString& assetFile, nsAssetFileHeader& outHeader) const;
	bool GetAssetInfoFromFile(const nsString& assetFile, nsAssetInfo& outAssetInfo) const;
	void GetAssetInfosFromPath(const nsString& path, nsTArray<nsAssetInfo>& outAssetInfos) const;
	void Update();


	NS_NODISCARD_INLINE const nsString& GetEngineAssetsPath() const
	{
		return EngineAssetsPath;
	}


	NS_NODISCARD_INLINE const nsString& GetGameAssetsPath() const
	{
		return GameAssetsPath;
	}



// ================================================================================================ //
// TEXTURE
// ================================================================================================ //
public:
	void SaveTextureAsset(nsName name, nsTextureID texture, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedTextureAsset LoadTextureAsset(const nsName& name);
	void Internal_AddTextureAssetReference(int index, nsTextureID texture);
	void Internal_RemoveTextureAssetReference(int index, nsTextureID texture);

private:
	void UpdateTextureAssets();



// ================================================================================================ //
// MATERIAL
// ================================================================================================ //
public:
	void SaveMaterialAsset(nsName name, nsMaterialID material, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedMaterialAsset LoadMaterialAsset(const nsName& name);
	void Internal_AddMaterialAssetReference(int index, nsMaterialID material);
	void Internal_RemoveMaterialAssetReference(int index, nsMaterialID material);

private:
	void UpdateMaterialAssets();



// ================================================================================================ //
// MODEL
// ================================================================================================ //
public:
	void SaveModelAsset(nsName name, const nsAssetModelMeshes& meshes, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedModelAsset LoadModelAsset(const nsName& name);
	void Internal_AddModelAssetReference(int index);
	void Internal_RemoveModelAssetReference(int index);

private:
	void UpdateModelAssets();


	NS_DECLARE_SINGLETON(nsAssetManager)

};
