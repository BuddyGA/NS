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

private:
	bool bInitialized;
	nsString EngineAssetsPath;
	nsString GameAssetsPath;


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
private:
	struct TextureAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<nsTextureID> Handles;

		TextureAssetData()
		{
			Names.Reserve(64);
			Paths.Reserve(64);
			Flags.Reserve(64);
			RefCounts.Reserve(64);
			Handles.Reserve(64);
		}
	};

	TextureAssetData TextureAsset;


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
private:
	struct MaterialAssetData
	{
		struct ParameterTable
		{
			nsTMap<nsName, nsSharedTextureAsset> TextureParameterValues;
			nsTMap<nsName, float> ScalarParameterValues;
			nsTMap<nsName, nsVector4> VectorParameterValues;
		};

		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<ParameterTable> ParameterTables;
		nsTArray<nsMaterialID> Handles;

		MaterialAssetData()
		{
			Names.Reserve(64);
			Paths.Reserve(64);
			Flags.Reserve(64);
			RefCounts.Reserve(64);
			ParameterTables.Reserve(64);
			Handles.Reserve(64);
		}
	};

	MaterialAssetData MaterialAsset;


public:
	void SaveMaterialAsset(nsName name, nsMaterialID material, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedMaterialAsset LoadMaterialAsset(const nsName& name);
	void Internal_AddMaterialAssetReference(int index, nsMaterialID material);
	void Internal_RemoveMaterialAssetReference(int index, nsMaterialID material);

	/*
	void Internal_SetMaterialTextureParameterValue(int index, nsMaterialID material, nsName paramName, const nsSharedTextureAsset& sharedTextureAsset);
	nsSharedTextureAsset Internal_GetMaterialTextureParamaterValue(int index, nsMaterialID material, nsName paramName) const;
	void Internal_SetMaterialScalarParameterValue(int index, nsMaterialID material, nsName paramName, float scalar);
	float Internal_GetMaterialScalarParameterValue(int index, nsMaterialID material, nsName paramName) const;
	void Internal_SetMaterialVectorParameterValue(int index, nsMaterialID material, nsName paramName, nsVector4 vector);
	nsVector4 Internal_GetMaterialVectorParameterValue(int index, nsMaterialID material, nsName paramName) const;
	*/

private:
	void UpdateMaterialAssets();



// ================================================================================================ //
// MODEL
// ================================================================================================ //
private:
	struct ModelAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<nsAssetModelMeshes> Meshes;

		ModelAssetData()
		{
			Names.Reserve(64);
			Paths.Reserve(64);
			Flags.Reserve(64);
			RefCounts.Reserve(64);
			Meshes.Reserve(64);
		}
	};

	ModelAssetData ModelAsset;


public:
	void SaveModelAsset(nsName name, const nsAssetModelMeshes& meshes, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedModelAsset LoadModelAsset(const nsName& name);
	void Internal_AddModelAssetReference(int index);
	void Internal_RemoveModelAssetReference(int index);

private:
	void UpdateModelAssets();


	NS_DECLARE_SINGLETON(nsAssetManager)

};
