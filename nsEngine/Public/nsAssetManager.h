#pragma once

#include "nsAssetTypes.h"



class NS_ENGINE_API nsAssetManager
{
	NS_DECLARE_SINGLETON(nsAssetManager)

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



	enum AssetFlags : uint8
	{
		AssetFlag_None				= (0),
		AssetFlag_Loading			= (1 << 0),
		AssetFlag_Loaded			= (1 << 1),
		AssetFlag_PendingUnload		= (1 << 2),
		AssetFlag_Unloaded			= (1 << 3),
	};


	template<typename THandle>
	struct TAssetData
	{
		nsTArray<nsName> Names;
		nsTArray<nsString> Paths;
		nsTArray<uint8> Flags;
		nsTArray<int> RefCounts;
		nsTArray<THandle> Handles;

	public:
		TAssetData()
		{
			Names.Reserve(64);
			Paths.Reserve(64);
			Flags.Reserve(64);
			RefCounts.Reserve(64);
			Handles.Reserve(64);
		}


		NS_INLINE void Add(nsName name, nsString path, uint8 flags, THandle handle)
		{
			Names.Add(name);
			Paths.Add(path);
			Flags.Add(flags);
			RefCounts.Add(0);
			Handles.Add(handle);
		}


		NS_INLINE int AddReference(int index, THandle handle)
		{
			NS_Assert(index >= 0 && index < Handles.GetCount());
			NS_Assert(handle == Handles[index]);

			return ++RefCounts[index];
		}


		NS_INLINE int RemoveReference(int index, THandle handle)
		{
			NS_Assert(index >= 0 && index < Handles.GetCount());
			NS_Assert(handle == Handles[index]);

			if (--RefCounts[index] <= 0)
			{
				RefCounts[index] = 0;
				Flags[index] |= AssetFlag_PendingUnload;
			}

			return RefCounts[index];
		}

	};



// ================================================================================================ //
// TEXTURE
// ================================================================================================ //
private:
	TAssetData<nsTextureID> TextureAsset;


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
	TAssetData<nsMaterialID> MaterialAsset;


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
private:
	TAssetData<nsAssetModelMeshes> ModelAsset;


public:
	void SaveModelAsset(nsName name, const nsAssetModelMeshes& meshes, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedModelAsset LoadModelAsset(const nsName& name);
	void Internal_AddModelAssetReference(int index);
	void Internal_RemoveModelAssetReference(int index);

private:
	void UpdateModelAssets();



// ================================================================================================ //
// SKELETON
// ================================================================================================ //
private:
	TAssetData<nsAnimationSkeletonID> SkeletonAsset;


public:
	void SaveSkeletonAsset(nsName name, nsAnimationSkeletonID skeleton, const nsString& folderPath, bool bIsEngineAsset);
	nsSharedSkeletonAsset LoadSkeletonAsset(const nsName& name);
	void Internal_AddSkeletonAssetReference(int index, nsAnimationSkeletonID skeleton);
	void Internal_RemoveSkeletonAssetReference(int index, nsAnimationSkeletonID skeleton);

private:
	void UpdateSkeletonAssets();

};
