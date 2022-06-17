#include "nsAssetTypes.h"
#include "nsAssetManager.h"



nsLogCategory AssetLog("nsAssetLog", nsELogVerbosity::LV_DEBUG);



// ====================================================================================================================================================================== //
// SHARED ASSET - TEXTURE
// ====================================================================================================================================================================== //
nsSharedTextureAsset::nsSharedTextureAsset() noexcept
	: AssetId(-1)
	, Name("")
	, Texture(nsTextureID::INVALID)
{
}


nsSharedTextureAsset::nsSharedTextureAsset(const nsSharedTextureAsset& other) noexcept
	: AssetId(-1)
	, Name("")
	, Texture(nsTextureID::INVALID)
{
	Copy(other);
}


nsSharedTextureAsset::nsSharedTextureAsset(nsSharedTextureAsset&& other) noexcept
	: AssetId(other.AssetId)
	, Name(other.Name)
	, Texture(other.Texture)
{
	other.AssetId = -1;
	other.Name = "";
	other.Texture = nsTextureID::INVALID;
}


nsSharedTextureAsset::~nsSharedTextureAsset() noexcept
{
	Release();
}


nsSharedTextureAsset::nsSharedTextureAsset(int assetId, nsName name, nsTextureID texture) noexcept
	: AssetId(assetId)
	, Name(name)
	, Texture(texture)
{
	if (AssetId != -1)
	{
		NS_Assert(Texture.IsValid());
		nsAssetManager::Get().Internal_AddTextureAssetReference(AssetId, Texture);
	}
}


void nsSharedTextureAsset::Copy(const nsSharedTextureAsset& other) noexcept
{
	Release();

	AssetId = other.AssetId;
	Name = other.Name;
	Texture = other.Texture;

	if (AssetId != -1)
	{
		NS_Assert(Texture.IsValid());
		nsAssetManager::Get().Internal_AddTextureAssetReference(AssetId, Texture);
	}
}


void nsSharedTextureAsset::Release() noexcept
{
	if (AssetId != -1)
	{
		NS_Assert(Texture.IsValid());
		nsAssetManager::Get().Internal_RemoveTextureAssetReference(AssetId, Texture);
	}

	AssetId = -1;
	Name = "";
	Texture = nsTextureID::INVALID;
}




// ====================================================================================================================================================================== //
// SHARED ASSET - MATERIAL
// ====================================================================================================================================================================== //
nsSharedMaterialAsset::nsSharedMaterialAsset() noexcept
	: AssetId(-1)
	, Name("")
	, Material(nsMaterialID::INVALID)
{
}


nsSharedMaterialAsset::nsSharedMaterialAsset(const nsSharedMaterialAsset& other) noexcept
	: AssetId(-1)
	, Name("")
	, Material(nsMaterialID::INVALID)
{
	Copy(other);
}


nsSharedMaterialAsset::nsSharedMaterialAsset(nsSharedMaterialAsset&& other) noexcept
	: AssetId(other.AssetId)
	, Name(other.Name)
	, Material(other.Material)
{
	other.AssetId = -1;
	other.Name = "";
	other.Material = nsMaterialID::INVALID;
}


nsSharedMaterialAsset::~nsSharedMaterialAsset() noexcept
{
	Release();
}


nsSharedMaterialAsset::nsSharedMaterialAsset(int assetId, nsName name, nsMaterialID material) noexcept
	: AssetId(assetId)
	, Name(name)
	, Material(material)
{
	if (AssetId != -1)
	{
		NS_Assert(Material.IsValid());
		nsAssetManager::Get().Internal_AddMaterialAssetReference(AssetId, Material);
	}
}


void nsSharedMaterialAsset::Copy(const nsSharedMaterialAsset& other) noexcept
{
	Release();

	AssetId = other.AssetId;
	Name = other.Name;
	Material = other.Material;

	if (AssetId != -1)
	{
		NS_Assert(Material.IsValid());
		nsAssetManager::Get().Internal_AddMaterialAssetReference(AssetId, Material);
	}
}


void nsSharedMaterialAsset::Release() noexcept
{
	if (AssetId != -1)
	{
		NS_Assert(Material.IsValid());
		nsAssetManager::Get().Internal_RemoveMaterialAssetReference(AssetId, Material);
	}

	AssetId = -1;
	Name = "";
	Material = nsMaterialID::INVALID;
}


void nsSharedMaterialAsset::SetTextureParameterValue(nsName paramName, nsSharedTextureAsset texture) noexcept
{
	if (!IsValid())
	{
		return;
	}
}


nsSharedTextureAsset nsSharedMaterialAsset::GetTextureParamaterValue(nsName paramName) const noexcept
{
	nsSharedTextureAsset texture;

	if (!IsValid())
	{
		return texture;
	}

	texture;
}


void nsSharedMaterialAsset::SetScalarParameterValue(nsName paramName, float value) noexcept
{
	if (!IsValid())
	{
		return;
	}
}


float nsSharedMaterialAsset::GetScalarParameterValue(nsName paramName) const noexcept
{
	float scalar = 0.0f;

	if (!IsValid())
	{
		return scalar;
	}
}


void nsSharedMaterialAsset::SetVectorParameterValue(nsName paramName, nsVector4 value) noexcept
{
	if (!IsValid())
	{
		return;
	}
}


nsVector4 nsSharedMaterialAsset::GetVectorParameterValue(nsName paramName) const noexcept
{
	nsVector4 vector;

	if (!IsValid())
	{
		return vector;
	}
}




// ====================================================================================================================================================================== //
// SHARED ASSET - MODEL
// ====================================================================================================================================================================== //
nsSharedModelAsset::nsSharedModelAsset() noexcept
	: AssetId(-1)
	, Name("")
	, Meshes()
{

}


nsSharedModelAsset::nsSharedModelAsset(const nsSharedModelAsset& other) noexcept
	: AssetId(-1)
	, Name("")
	, Meshes()
{
	Copy(other);
}


nsSharedModelAsset::nsSharedModelAsset(nsSharedModelAsset&& other) noexcept
	: AssetId(other.AssetId)
	, Name(other.Name)
	, Meshes(other.Meshes)
{
	other.AssetId = -1;
	other.Name = "";
	other.Meshes.Clear();
}


nsSharedModelAsset::~nsSharedModelAsset() noexcept
{
	Release();
}


nsSharedModelAsset::nsSharedModelAsset(int assetId, nsName name, nsAssetModelMeshes meshes) noexcept
	: AssetId(assetId)
	, Name(name)
	, Meshes(meshes)
{
	if (AssetId != -1)
	{
		NS_Assert(Meshes.GetCount() > 0);
		nsAssetManager::Get().Internal_AddModelAssetReference(AssetId);
	}
}


void nsSharedModelAsset::Copy(const nsSharedModelAsset& other) noexcept
{
	Release();

	AssetId = other.AssetId;
	Name = other.Name;
	Meshes = other.Meshes;

	if (AssetId != -1)
	{
		NS_Assert(Meshes.GetCount() > 0);
		nsAssetManager::Get().Internal_AddModelAssetReference(AssetId);
	}
}


void nsSharedModelAsset::Release() noexcept
{
	if (AssetId != -1)
	{
		NS_Assert(Meshes.GetCount() > 0);
		nsAssetManager::Get().Internal_RemoveModelAssetReference(AssetId);
	}

	AssetId = -1;
	Name = "";
	Meshes.Clear();
}
