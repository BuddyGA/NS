#pragma once

#include "nsEngineTypes.h"



extern nsLogCategory AssetLog;



enum class nsEAssetType : uint8
{
	NONE = 0,
	TEXTURE,
	MATERIAL,
	MODEL,
	SKELETON,
	ANIMATION,
	AUDIO,
	LEVEL,
	FONT,
	ACTOR_TEMPLATE,
};



struct nsAssetFileHeader
{
	int Signature;
	int Version;
	int Type;
	int Compression;


public:
	nsAssetFileHeader() noexcept
		: Signature(0)
		, Version(0)
		, Type(0)
		, Compression(0)
	{
	}


	friend NS_INLINE void operator|(nsStream& stream, nsAssetFileHeader& assetFileHeader) noexcept
	{
		stream | assetFileHeader.Signature;
		stream | assetFileHeader.Version;
		stream | assetFileHeader.Type;
		stream | assetFileHeader.Compression;
	}

};



struct nsAssetInfo
{
	nsName Name;
	nsString Path;
	nsEAssetType Type;


public:
	nsAssetInfo() noexcept
	{
		Type = nsEAssetType::NONE;
	}


	NS_INLINE bool operator==(const nsAssetInfo& rhs) const noexcept
	{
		return Name == rhs.Name && Type == rhs.Type;
	}

};



class NS_ENGINE_API nsSharedTextureAsset
{
private:
	int AssetId;
	nsName Name;
	nsTextureID Texture;


public:
	nsSharedTextureAsset() noexcept;
	nsSharedTextureAsset(const nsSharedTextureAsset& other) noexcept;
	nsSharedTextureAsset(nsSharedTextureAsset&& other) noexcept;
	~nsSharedTextureAsset() noexcept;

private:
	nsSharedTextureAsset(int assetId, nsName name, nsTextureID texture) noexcept;
	void Copy(const nsSharedTextureAsset& other) noexcept;

public:
	void Release() noexcept;


	NS_NODISCARD_INLINE bool IsValid() const noexcept
	{
		return AssetId != -1 && Texture.IsValid();
	}


	NS_NODISCARD_INLINE nsName GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE nsTextureID GetTexture() const noexcept
	{
		return Texture;
	}


	NS_INLINE nsSharedTextureAsset& operator=(const nsSharedTextureAsset& rhs) noexcept
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}


	NS_INLINE nsSharedTextureAsset& operator=(nsSharedTextureAsset&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			AssetId = rhs.AssetId;
			Name = rhs.Name;
			Texture = rhs.Texture;
			rhs.AssetId = -1;
			rhs.Name = "";
			rhs.Texture = nsTextureID::INVALID;
		}

		return *this;
	}


	friend class nsAssetManager;

};



class NS_ENGINE_API nsSharedMaterialAsset
{
private:
	int AssetId;
	nsName Name;
	nsMaterialID Material;


public:
	nsSharedMaterialAsset() noexcept;
	nsSharedMaterialAsset(const nsSharedMaterialAsset& other) noexcept;
	nsSharedMaterialAsset(nsSharedMaterialAsset&& other) noexcept;
	~nsSharedMaterialAsset() noexcept;

private:
	nsSharedMaterialAsset(int assetId, nsName name, nsMaterialID material) noexcept;
	void Copy(const nsSharedMaterialAsset& other) noexcept;

public:
	void Release() noexcept;


	NS_NODISCARD_INLINE bool IsValid() const noexcept
	{
		return AssetId != -1 && Material.IsValid();
	}


	NS_NODISCARD_INLINE nsName GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE nsMaterialID GetMaterial() const noexcept
	{
		return Material;
	}


	NS_INLINE nsSharedMaterialAsset& operator=(const nsSharedMaterialAsset& rhs) noexcept
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}


	NS_INLINE nsSharedMaterialAsset& operator=(nsSharedMaterialAsset&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			AssetId = rhs.AssetId;
			Name = rhs.Name;
			Material = rhs.Material;
			rhs.AssetId = -1;
			rhs.Name = "";
			rhs.Material = nsMaterialID::INVALID;
		}

		return *this;
	}


	NS_INLINE bool operator==(const nsSharedMaterialAsset& rhs) const noexcept
	{
		return AssetId == rhs.AssetId && Material == rhs.Material;
	}


	NS_INLINE bool operator!=(const nsSharedMaterialAsset& rhs) const noexcept
	{
		return !(*this == rhs);
	}


	friend class nsAssetManager;
};



typedef nsTArrayInline<nsMeshID, NS_ENGINE_ASSET_MODEL_MAX_MESH> nsAssetModelMeshes;


class NS_ENGINE_API nsSharedModelAsset
{
private:
	int AssetId;
	nsName Name;
	nsAssetModelMeshes Meshes;


public:
	nsSharedModelAsset() noexcept;
	nsSharedModelAsset(const nsSharedModelAsset& other) noexcept;
	nsSharedModelAsset(nsSharedModelAsset&& other) noexcept;
	~nsSharedModelAsset() noexcept;

private:
	nsSharedModelAsset(int assetId, nsName name, nsAssetModelMeshes meshes) noexcept;
	void Copy(const nsSharedModelAsset& other) noexcept;

public:
	void Release() noexcept;


	NS_NODISCARD_INLINE bool IsValid() const noexcept
	{
		return AssetId != -1 && Meshes.GetCount() > 0;
	}


	NS_NODISCARD_INLINE nsName GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE const nsAssetModelMeshes& GetMeshes() const noexcept
	{
		return Meshes;
	}


	NS_INLINE nsSharedModelAsset& operator=(const nsSharedModelAsset& rhs) noexcept
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}


	NS_INLINE nsSharedModelAsset& operator=(nsSharedModelAsset&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			AssetId = rhs.AssetId;
			Name = rhs.Name;
			Meshes = rhs.Meshes;
			rhs.AssetId = -1;
			rhs.Name = "";
			rhs.Meshes.Clear();
		}

		return *this;
	}


	NS_INLINE bool operator==(const nsSharedModelAsset& rhs) const noexcept
	{
		return AssetId == rhs.AssetId;
	}


	NS_INLINE bool operator!=(const nsSharedModelAsset& rhs) const noexcept
	{
		return !(*this == rhs);
	}


	friend class nsAssetManager;
};




class NS_ENGINE_API nsSharedSkeletonAsset
{
private:
	int AssetId;
	nsName Name;
	nsAnimationSkeletonID Skeleton;


public:
	nsSharedSkeletonAsset() noexcept;
	nsSharedSkeletonAsset(const nsSharedSkeletonAsset& other) noexcept;
	nsSharedSkeletonAsset(nsSharedSkeletonAsset&& other) noexcept;
	~nsSharedSkeletonAsset() noexcept;

private:
	nsSharedSkeletonAsset(int assetId, nsName name, nsAnimationSkeletonID skeleton) noexcept;
	void Copy(const nsSharedSkeletonAsset& other) noexcept;

public:
	void Release() noexcept;


	NS_NODISCARD_INLINE bool IsValid() const noexcept
	{
		return AssetId != -1 && Skeleton.IsValid();
	}


	NS_NODISCARD_INLINE nsName GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE nsAnimationSkeletonID GetSkeleton() const noexcept
	{
		return Skeleton;
	}


	NS_INLINE nsSharedSkeletonAsset& operator=(const nsSharedSkeletonAsset& rhs) noexcept
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}


	NS_INLINE nsSharedSkeletonAsset& operator=(nsSharedSkeletonAsset&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			AssetId = rhs.AssetId;
			Name = rhs.Name;
			Skeleton = rhs.Skeleton;
			rhs.AssetId = -1;
			rhs.Name = "";
			rhs.Skeleton = nsAnimationSkeletonID::INVALID;
		}

		return *this;
	}


	NS_INLINE bool operator==(const nsSharedSkeletonAsset& rhs) const noexcept
	{
		return AssetId == rhs.AssetId && Skeleton == rhs.Skeleton;
	}


	NS_INLINE bool operator!=(const nsSharedSkeletonAsset& rhs) const noexcept
	{
		return !(*this == rhs);
	}


	friend class nsAssetManager;

};




class NS_ENGINE_API nsSharedAnimationAsset
{
private:
	int AssetId;
	nsName Name;
	nsAnimationClipID Clip;


public:
	nsSharedAnimationAsset() noexcept;
	nsSharedAnimationAsset(const nsSharedAnimationAsset& other) noexcept;
	nsSharedAnimationAsset(nsSharedAnimationAsset&& other) noexcept;
	~nsSharedAnimationAsset() noexcept;

private:
	nsSharedAnimationAsset(int assetId, nsName name, nsAnimationClipID clip) noexcept;
	void Copy(const nsSharedAnimationAsset& other) noexcept;

public:
	void Release() noexcept;


	NS_NODISCARD_INLINE bool IsValid() const noexcept
	{
		return AssetId != -1 && Clip.IsValid();
	}


	NS_NODISCARD_INLINE nsName GetName() const noexcept
	{
		return Name;
	}


	NS_NODISCARD_INLINE nsAnimationClipID GetClip() const noexcept
	{
		return Clip;
	}


	NS_INLINE nsSharedAnimationAsset& operator=(const nsSharedAnimationAsset& rhs) noexcept
	{
		if (this != &rhs)
		{
			Copy(rhs);
		}

		return *this;
	}


	NS_INLINE nsSharedAnimationAsset& operator=(nsSharedAnimationAsset&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			AssetId = rhs.AssetId;
			Name = rhs.Name;
			Clip = rhs.Clip;
			rhs.AssetId = -1;
			rhs.Name = "";
			rhs.Clip = nsAnimationClipID::INVALID;
		}

		return *this;
	}


	NS_INLINE bool operator==(const nsSharedAnimationAsset& rhs) const noexcept
	{
		return AssetId == rhs.AssetId && Clip == rhs.Clip;
	}


	NS_INLINE bool operator!=(const nsSharedAnimationAsset& rhs) const noexcept
	{
		return !(*this == rhs);
	}


	friend class nsAssetManager;
};
