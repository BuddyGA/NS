#include "nsAssetManager.h"
#include "nsFileSystem.h"
#include "nsConsole.h"
#include "nsMesh.h"
#include "nsTexture.h"
#include "nsMaterial.h"



nsAssetManager::nsAssetManager() noexcept
	: bInitialized(false)
{
	EngineAssetsPath = "EngineAssets";
	GameAssetsPath = "GameAssets";
}


void nsAssetManager::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	nsFileSystem::FolderCreate(EngineAssetsPath);
	nsFileSystem::FolderCreate(GameAssetsPath);

	// Initialize engine texture assets
	{
		nsTextureManager& textureManager = nsTextureManager::Get();

		const nsTextureID whiteTexture = textureManager.GetDefaultTexture2D_White();
		SaveTextureAsset(textureManager.GetTextureName(whiteTexture), whiteTexture, "Textures", true);

		const nsTextureID blackTexture = textureManager.GetDefaultTexture2D_Black();
		SaveTextureAsset(textureManager.GetTextureName(blackTexture), blackTexture, "Textures", true);
	}


	// Initialize engine model assets
	{
		nsMeshManager& meshManager = nsMeshManager::Get();

		const nsMeshID floorMesh = meshManager.GetDefaultMesh_Floor();
		nsAssetModelMeshes floorMeshes;
		floorMeshes.Add(floorMesh);
		SaveModelAsset("mdl_default_floor", floorMeshes, "Models", true);

		const nsMeshID wallMesh = meshManager.GetDefaultMesh_Wall();
		nsAssetModelMeshes wallMeshes;
		wallMeshes.Add(wallMesh);
		SaveModelAsset("mdl_default_wall", wallMeshes, "Models", true);

		const nsMeshID boxMesh = meshManager.GetDefaultMesh_Box();
		nsAssetModelMeshes boxMeshes;
		boxMeshes.Add(boxMesh);
		SaveModelAsset("mdl_default_box", boxMeshes, "Models", true);

		const nsMeshID platformMesh = meshManager.GetDefaultMesh_Platform();
		nsAssetModelMeshes platformMeshes;
		platformMeshes.Add(platformMesh);
		SaveModelAsset("mdl_default_platform", platformMeshes, "Models", true);
	}


	nsTArray<nsString> assetFiles;
	assetFiles.Reserve(64);
	nsFileSystem::FileIterate(assetFiles, GameAssetsPath, true, NS_ENGINE_ASSET_FILE_EXTENSION);

	for (int i = 0; i < assetFiles.GetCount(); ++i)
	{
		RegisterAsset(assetFiles[i]);
	}

	bInitialized = true;
}


void nsAssetManager::RegisterAsset(const nsString& assetFile)
{
	nsAssetFileHeader header{};

	if (!IsValidAssetFile(assetFile, header))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to register asset from file [%s]. Invalid asset file!", *assetFile);
		return;
	}

	const nsName name = nsFileSystem::FileGetName(assetFile);
	const nsString path = nsFileSystem::FileGetPath(assetFile);
	const nsEAssetType assetType = static_cast<nsEAssetType>(header.Type);

	switch (assetType)
	{
		case nsEAssetType::TEXTURE:
		{
			NS_AssertV(TextureAsset.Names.Find(name) == NS_ARRAY_INDEX_INVALID, "Texture asset with name [%s] already exists!", *name);
			NS_CONSOLE_Log(AssetLog, "Register texture asset [%s]", *name);

			TextureAsset.Names.Add(name);
			TextureAsset.Paths.Add(path);
			TextureAsset.Flags.Add(AssetFlag_Unloaded);
			TextureAsset.RefCounts.Add(0);
			TextureAsset.Handles.Add(nsTextureID::INVALID);

			break;
		}

		case nsEAssetType::MATERIAL:
		{
			NS_ValidateV(0, "Not implemented yet!");
			break;
		}

		case nsEAssetType::MODEL:
		{
			NS_AssertV(ModelAsset.Names.Find(name) == NS_ARRAY_INDEX_INVALID, "Mesh asset with name [%s] already exists!", *name);
			NS_CONSOLE_Log(AssetLog, "Register mesh asset [%s]", *name);

			ModelAsset.Names.Add(name);
			ModelAsset.Paths.Add(path);
			ModelAsset.Flags.Add(AssetFlag_Unloaded);
			ModelAsset.RefCounts.Add(0);
			ModelAsset.Meshes.Add();

			break;
		}

		default:
			NS_ValidateV(0, "Not implemented yet!");
			break;
	}
}


bool nsAssetManager::IsValidAssetFile(const nsString& assetFile, nsAssetFileHeader& outHeader) const
{
	if (assetFile.IsEmpty())
	{
		return false;
	}

	const nsName ext = nsFileSystem::FileGetExtension(assetFile);

	if (ext != NS_ENGINE_ASSET_FILE_EXTENSION)
	{
		return false;
	}

	nsFileStreamReader reader(assetFile);
	reader | outHeader;

	// TODO: Other validations

	return outHeader.Signature == NS_ENGINE_ASSET_FILE_SIGNATURE;
}


bool nsAssetManager::GetAssetInfoFromFile(const nsString& assetFile, nsAssetInfo& outAssetInfo) const
{
	bool bValid = false;
	nsAssetFileHeader header{};

	if (IsValidAssetFile(assetFile, header))
	{
		outAssetInfo.Name = nsFileSystem::FileGetName(assetFile);
		outAssetInfo.Path = nsFileSystem::FileGetPath(assetFile);
		outAssetInfo.Type = static_cast<nsEAssetType>(header.Type);
		bValid = true;
	}

	return bValid;
}


void nsAssetManager::GetAssetInfosFromPath(const nsString& path, nsTArray<nsAssetInfo>& outAssetInfos) const
{
	if (!nsFileSystem::FolderExists(path))
	{
		return;
	}

	nsTArray<nsString> assetFiles;
	nsFileSystem::FileIterate(assetFiles, path, false, NS_ENGINE_ASSET_FILE_EXTENSION);

	const int count = assetFiles.GetCount();
	outAssetInfos.Reserve(outAssetInfos.GetCount() + count);

	nsAssetInfo assetInfo{};

	for (int i = 0; i < count; ++i)
	{
		if (GetAssetInfoFromFile(assetFiles[i], assetInfo))
		{
			outAssetInfos.Add(assetInfo);
		}
	}
}


void nsAssetManager::Update()
{
	UpdateTextureAssets();
	UpdateModelAssets();
}




// ====================================================================================================================================================================== //
// TEXTURE
// ====================================================================================================================================================================== //
void nsAssetManager::SaveTextureAsset(nsName name, nsTextureID texture, const nsString& folderPath, bool bIsEngineAsset)
{
	nsTextureManager& textureManager = nsTextureManager::Get();
	const nsName assetName = textureManager.GetTextureName(texture);
	const nsString assetPath = nsString::Format("%s/%s", bIsEngineAsset ? *EngineAssetsPath : *GameAssetsPath, *folderPath);

	if (TextureAsset.Names.Find(assetName) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save texture asset. Texture asset with name [%s] already exists!", *assetName);
		return;
	}

	if (!nsFileSystem::FolderCreate(assetPath))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save texture asset. Create folder [%s] failed!", *assetPath);
		return;
	}
	
	// Write data
	nsBinaryStreamWriter writer;
	{
		nsAssetFileHeader header;
		header.Signature = NS_ENGINE_ASSET_FILE_SIGNATURE;
		header.Version = NS_ENGINE_ASSET_FILE_VERSION;
		header.Type = static_cast<int>(nsEAssetType::TEXTURE);
		header.Compression = 0;

		writer | header;

		nsTextureData& data = textureManager.GetTextureData(texture);
		writer | data;
	}
	

	const nsString assetFile = nsString::Format("%s/%s%s", *assetPath, *assetName, NS_ENGINE_ASSET_FILE_EXTENSION);

	if (!nsFileSystem::FileWriteBinary(assetFile, writer.GetBuffer()))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save texture [%s] to asset file [%s]", *assetName, *assetFile);
		return;
	}

	NS_CONSOLE_Log(AssetLog, "Texture [%s] saved to asset file [%s]", *assetName, *assetFile);

	TextureAsset.Names.Add(assetName);
	TextureAsset.Paths.Add(assetPath);
	TextureAsset.Flags.Add(AssetFlag_Loaded);
	TextureAsset.RefCounts.Add(0);
	TextureAsset.Handles.Add(texture);
}


nsSharedTextureAsset nsAssetManager::LoadTextureAsset(const nsName& name)
{
	if (name.GetLength() == 0)
	{
		return nsSharedTextureAsset();
	}

	const int index = TextureAsset.Names.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load texture asset. Texture asset with name [%s] not found!", *name);
		return nsSharedTextureAsset();
	}

	if (TextureAsset.Handles[index] != nsTextureID::INVALID)
	{
		TextureAsset.Flags[index] = AssetFlag_Loaded;
		return nsSharedTextureAsset(index, name, TextureAsset.Handles[index]);
	}

	const nsString assetFile = nsString::Format("%s/%s%s", *TextureAsset.Paths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

	nsTArray<uint8> bytes;

	if (!nsFileSystem::FileReadBinary(assetFile, bytes))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load texture asset. Read data from asset file [%s] failed!", *assetFile);
		return nsSharedTextureAsset();
	}


	// Read data
	nsBinaryStreamReader reader(bytes);
	{
		nsAssetFileHeader header{};
		reader | header;

		NS_Validate(header.Signature == NS_ENGINE_ASSET_FILE_SIGNATURE && header.Type == static_cast<int>(nsEAssetType::TEXTURE));

		nsTextureManager& textureManager = nsTextureManager::Get();
		TextureAsset.Handles[index] = textureManager.CreateTexture2D_Empty(name);

		nsTextureData& data = textureManager.GetTextureData(TextureAsset.Handles[index]);
		reader | data;
	}
	

	TextureAsset.Flags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded texture asset [%s]", *name);

	return nsSharedTextureAsset(index, name, TextureAsset.Handles[index]);
}


void nsAssetManager::Internal_AddTextureAssetReference(int index, nsTextureID texture)
{
	NS_Assert(index >= 0 && index < TextureAsset.Handles.GetCount());
	NS_Assert(texture == TextureAsset.Handles[index]);

	TextureAsset.RefCounts[index]++;
}


void nsAssetManager::Internal_RemoveTextureAssetReference(int index, nsTextureID texture)
{
	NS_Assert(index >= 0 && index < TextureAsset.Handles.GetCount());
	NS_Assert(texture == TextureAsset.Handles[index]);

	if (--TextureAsset.RefCounts[index] <= 0)
	{
		TextureAsset.RefCounts[index] = 0;
		TextureAsset.Flags[index] |= AssetFlag_PendingUnload;
		NS_CONSOLE_Debug(AssetLog, "Mark texture asset [%s] pending unload (RefCount = 0)", *TextureAsset.Names[index]);
	}
}


void nsAssetManager::UpdateTextureAssets()
{
	for (int i = 0; i < TextureAsset.Flags.GetCount(); ++i)
	{
		uint8& flags = TextureAsset.Flags[i];

		if (flags & AssetFlag_PendingUnload)
		{
			NS_Assert(TextureAsset.RefCounts[i] == 0);
			NS_Assert(TextureAsset.Handles[i].IsValid());
			
			flags = AssetFlag_Unloaded;
			TextureAsset.RefCounts[i] = 0;
			NS_CONSOLE_Debug(AssetLog, "Unloaded texture asset [%s]", *TextureAsset.Names[i]);
			nsTextureManager::Get().DestroyTexture(TextureAsset.Handles[i]);
		}
	}
}




// ====================================================================================================================================================================== //
// MATERIAL
// ====================================================================================================================================================================== //
void nsAssetManager::SaveMaterialAsset(nsName name, nsMaterialID material, const nsString& folderPath, bool bIsEngineAsset)
{

}


nsSharedMaterialAsset nsAssetManager::LoadMaterialAsset(const nsName& name)
{
	if (name.GetLength() == 0)
	{
		return nsSharedMaterialAsset();
	}

	const int index = MaterialAsset.Names.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load material asset. Material asset with name [%s] not found!", *name);
		return nsSharedMaterialAsset();
	}

	if (MaterialAsset.Handles[index] != nsMaterialID::INVALID)
	{
		MaterialAsset.Flags[index] = AssetFlag_Loaded;
		return nsSharedMaterialAsset(index, name, MaterialAsset.Handles[index]);
	}


	const nsString assetFile = nsString::Format("%s/%s%s", *MaterialAsset.Paths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

	nsTArray<uint8> bytes;

	if (!nsFileSystem::FileReadBinary(assetFile, bytes))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load material asset. Read data from asset file [%s] failed!", *assetFile);
		return nsSharedMaterialAsset();
	}


	// Read data
	nsBinaryStreamReader reader(bytes);
	{
		nsAssetFileHeader header{};
		reader | header;

		NS_Validate(header.Signature == NS_ENGINE_ASSET_FILE_SIGNATURE && header.Type == static_cast<int>(nsEAssetType::MATERIAL));

		nsMaterialManager& materialManager = nsMaterialManager::Get();

		// TODO: Create material and load data
	}


	MaterialAsset.Flags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded material asset [%s]", *name);

	return nsSharedMaterialAsset(index, name, MaterialAsset.Handles[index]);
}


void nsAssetManager::Internal_AddMaterialAssetReference(int index, nsMaterialID material)
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);

	MaterialAsset.RefCounts[index]++;
}


void nsAssetManager::Internal_RemoveMaterialAssetReference(int index, nsMaterialID material)
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);

	if (--MaterialAsset.RefCounts[index] <= 0)
	{
		MaterialAsset.RefCounts[index] = 0;
		MaterialAsset.Flags[index] |= AssetFlag_PendingUnload;
		NS_CONSOLE_Debug(AssetLog, "Mark material asset [%s] pending unload (RefCount = 0)", *MaterialAsset.Names[index]);
	}
}

/*
void nsAssetManager::Internal_SetMaterialTextureParameterValue(int index, nsMaterialID material, nsName paramName, const nsSharedTextureAsset& sharedTextureAsset)
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);

	if (MaterialAsset.ParameterTables[index].TextureParameterValues.Exists(paramName))
	{

	}
}


nsSharedTextureAsset nsAssetManager::Internal_GetMaterialTextureParamaterValue(int index, nsMaterialID material, nsName paramName) const
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);

	nsSharedTextureAsset sharedTextureAsset;

	if (MaterialAsset.ParameterTables[index].TextureParameterValues.Exists(paramName))
	{

	}

	return sharedTextureAsset;
}


void nsAssetManager::Internal_SetMaterialScalarParameterValue(int index, nsMaterialID material, nsName paramName, float scalar)
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);
}


float nsAssetManager::Internal_GetMaterialScalarParameterValue(int index, nsMaterialID material, nsName paramName) const
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);
}


void nsAssetManager::Internal_SetMaterialVectorParameterValue(int index, nsMaterialID material, nsName paramName, nsVector4 vector)
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);
}


nsVector4 nsAssetManager::Internal_GetMaterialVectorParameterValue(int index, nsMaterialID material, nsName paramName) const
{
	NS_Assert(index >= 0 && index < MaterialAsset.Handles.GetCount());
	NS_Assert(material == MaterialAsset.Handles[index]);
}
*/

void nsAssetManager::UpdateMaterialAssets()
{

}




// ====================================================================================================================================================================== //
// MODEL
// ====================================================================================================================================================================== //
void nsAssetManager::SaveModelAsset(nsName name, const nsAssetModelMeshes& meshes, const nsString& folderPath, bool bIsEngineAsset)
{
	const nsString assetPath = nsString::Format("%s/%s", bIsEngineAsset ? *EngineAssetsPath : *GameAssetsPath, *folderPath);

	if (ModelAsset.Names.Find(name) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save model asset. Model asset with name [%s] already exists!", *name);
		return;
	}

	if (!nsFileSystem::FolderCreate(assetPath))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save model asset [%s]. Create folder [%s] failed!", *name, *assetPath);
		return;
	}


	// Write data
	nsBinaryStreamWriter writer;
	{
		nsAssetFileHeader header;
		header.Signature = NS_ENGINE_ASSET_FILE_SIGNATURE;
		header.Version = NS_ENGINE_ASSET_FILE_VERSION;
		header.Type = static_cast<int>(nsEAssetType::MODEL);
		header.Compression = 0;

		writer | header;

		int meshCount = meshes.GetCount();
		writer | meshCount;

		nsMeshManager& meshManager = nsMeshManager::Get();

		for (int i = 0; i < meshCount; ++i)
		{
			nsMeshLODGroup& lodGroup = meshManager.GetMeshLodGroup(meshes[i]);
			writer | lodGroup;
		}
	}
	

	const nsString assetFile = nsString::Format("%s/%s%s", *assetPath, *name, NS_ENGINE_ASSET_FILE_EXTENSION);

	if (!nsFileSystem::FileWriteBinary(assetFile, writer.GetBuffer()))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save model asset [%s] to file [%s]", *name, *assetFile);
		return;
	}

	NS_CONSOLE_Log(AssetLog, "Model asset [%s] saved to file [%s]", *name, *assetFile);

	ModelAsset.Names.Add(name);
	ModelAsset.Paths.Add(assetPath);
	ModelAsset.Flags.Add(AssetFlag_Loaded);
	ModelAsset.RefCounts.Add(0);
	ModelAsset.Meshes.Add(meshes);
}


nsSharedModelAsset nsAssetManager::LoadModelAsset(const nsName& name)
{
	if (name.GetLength() == 0)
	{
		return nsSharedModelAsset();
	}

	const int index = ModelAsset.Names.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load model asset. Model asset with name [%s] not found!", *name);
		return nsSharedModelAsset();
	}

	if (ModelAsset.Meshes[index].GetCount() > 0)
	{
		ModelAsset.Flags[index] = AssetFlag_Loaded;
		return nsSharedModelAsset(index, name, ModelAsset.Meshes[index]);
	}

	const nsString assetFile = nsString::Format("%s/%s%s", *ModelAsset.Paths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

	nsTArray<uint8> bytes;

	if (!nsFileSystem::FileReadBinary(assetFile, bytes))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load model asset. Read data from asset file [%s] failed!", *assetFile);
		return nsSharedModelAsset();
	}


	// Read data
	nsBinaryStreamReader reader(bytes);
	{
		nsAssetFileHeader header{};
		reader | header;

		NS_Validate(header.Signature == NS_ENGINE_ASSET_FILE_SIGNATURE && header.Type == static_cast<int>(nsEAssetType::MODEL));

		int meshCount = 0;
		reader | meshCount;

		nsMeshManager& meshManager = nsMeshManager::Get();

		for (int i = 0; i < meshCount; ++i)
		{
			const nsMeshID mesh = meshManager.CreateMesh(name);
			nsMeshLODGroup& lodGroup = meshManager.GetMeshLodGroup(mesh);
			reader | lodGroup;

			ModelAsset.Meshes[index].Add(mesh);
		}
	}


	ModelAsset.Flags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded model asset [%s]", *name);

	return nsSharedModelAsset(index, name, ModelAsset.Meshes[index]);
}


void nsAssetManager::Internal_AddModelAssetReference(int index)
{
	NS_Assert(index >= 0 && index < ModelAsset.RefCounts.GetCount());

	ModelAsset.RefCounts[index]++;
}


void nsAssetManager::Internal_RemoveModelAssetReference(int index)
{
	NS_Assert(index >= 0 && index < ModelAsset.RefCounts.GetCount());

	if (--ModelAsset.RefCounts[index] <= 0)
	{
		ModelAsset.RefCounts[index] = 0;
		ModelAsset.Flags[index] |= AssetFlag_PendingUnload;
		NS_CONSOLE_Debug(AssetLog, "Mark model asset [%s] pending unload (RefCount = 0)", *ModelAsset.Names[index]);
	}
}


void nsAssetManager::UpdateModelAssets()
{
	for (int i = 0; i < ModelAsset.Flags.GetCount(); ++i)
	{
		uint8& flags = ModelAsset.Flags[i];

		if (flags & AssetFlag_PendingUnload)
		{
			NS_Assert(ModelAsset.RefCounts[i] == 0);
			NS_CONSOLE_Debug(AssetLog, "Unloaded model asset [%s]", *ModelAsset.Names[i]);
			flags = AssetFlag_Unloaded;
			ModelAsset.RefCounts[i] = 0;

			const int meshCount = ModelAsset.Meshes[i].GetCount();
			NS_Assert(meshCount > 0);

			for (int m = 0; m < meshCount; ++m)
			{
				nsMeshManager::Get().DestroyMesh(ModelAsset.Meshes[i][m]);
			}

			ModelAsset.Meshes[i].Clear();
		}
	}
}
