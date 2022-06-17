#include "nsAssetManager.h"
#include "nsFileSystem.h"
#include "nsConsole.h"



nsAssetManager::nsAssetManager() noexcept
	: bInitialized(false)
{
	EngineAssetsPath = "EngineAssets";
	GameAssetsPath = "GameAssets";

	TextureAssetNames.Reserve(64);
	TextureAssetPaths.Reserve(64);
	TextureAssetFlags.Reserve(64);
	TextureAssetRefs.Reserve(64);
	TextureAssets.Reserve(64);

	ModelAssetNames.Reserve(64);
	ModelAssetPaths.Reserve(64);
	ModelAssetFlags.Reserve(64);
	ModelAssetRefs.Reserve(64);
	ModelAssetMeshes.Reserve(64);
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
			NS_AssertV(TextureAssetNames.Find(name) == NS_ARRAY_INDEX_INVALID, "Texture asset with name [%s] already exists!", *name);
			NS_CONSOLE_Log(AssetLog, "Register texture asset [%s]", *name);

			TextureAssetNames.Add(name);
			TextureAssetPaths.Add(path);
			TextureAssetFlags.Add(AssetFlag_Unloaded);
			TextureAssetRefs.Add(0);
			TextureAssets.Add(nsTextureID::INVALID);

			break;
		}

		case nsEAssetType::MATERIAL:
		{
			NS_ValidateV(0, "Not implemented yet!");
			break;
		}

		case nsEAssetType::MODEL:
		{
			NS_AssertV(ModelAssetNames.Find(name) == NS_ARRAY_INDEX_INVALID, "Mesh asset with name [%s] already exists!", *name);
			NS_CONSOLE_Log(AssetLog, "Register mesh asset [%s]", *name);

			ModelAssetNames.Add(name);
			ModelAssetPaths.Add(path);
			ModelAssetFlags.Add(AssetFlag_Unloaded);
			ModelAssetRefs.Add(0);
			ModelAssetMeshes.Add();

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

	if (TextureAssetNames.Find(assetName) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save texture asset. Texture asset with name [%s] already exists!", *assetName);
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

	TextureAssetNames.Add(assetName);
	TextureAssetPaths.Add(assetPath);
	TextureAssetFlags.Add(AssetFlag_Loaded);
	TextureAssetRefs.Add(0);
	TextureAssets.Add(texture);
}


nsSharedTextureAsset nsAssetManager::LoadTextureAsset(const nsName& name)
{
	if (name.GetLength() == 0)
	{
		return nsSharedTextureAsset();
	}

	const int index = TextureAssetNames.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load texture asset. Texture asset with name [%s] not found!", *name);
		return nsSharedTextureAsset();
	}

	if (TextureAssets[index] != nsTextureID::INVALID)
	{
		TextureAssetFlags[index] = AssetFlag_Loaded;
		return nsSharedTextureAsset(index, name, TextureAssets[index]);
	}

	const nsString assetFile = nsString::Format("%s/%s%s", *TextureAssetPaths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

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
		TextureAssets[index] = textureManager.CreateTexture2D_Empty(name);

		nsTextureData& data = textureManager.GetTextureData(TextureAssets[index]);
		reader | data;
	}
	

	TextureAssetFlags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded texture asset [%s]", *name);

	return nsSharedTextureAsset(index, name, TextureAssets[index]);
}


void nsAssetManager::Internal_AddTextureAssetReference(int index, nsTextureID texture)
{
	NS_Assert(index >= 0 && index < TextureAssets.GetCount());
	NS_Assert(TextureAssets[index] == texture);

	TextureAssetRefs[index]++;
}


void nsAssetManager::Internal_RemoveTextureAssetReference(int index, nsTextureID texture)
{
	NS_Assert(index >= 0 && index < TextureAssets.GetCount());
	NS_Assert(TextureAssets[index] == texture);

	if (--TextureAssetRefs[index] <= 0)
	{
		TextureAssetRefs[index] = 0;
		TextureAssetFlags[index] |= AssetFlag_PendingUnload;
		NS_CONSOLE_Debug(AssetLog, "Mark texture asset [%s] pending unload (RefCount = 0)", *TextureAssetNames[index]);
	}
}


void nsAssetManager::UpdateTextureAssets()
{
	for (int i = 0; i < TextureAssetFlags.GetCount(); ++i)
	{
		uint8& flags = TextureAssetFlags[i];

		if (flags & AssetFlag_PendingUnload)
		{
			NS_Assert(TextureAssetRefs[i] == 0);
			NS_Assert(TextureAssets[i].IsValid());
			
			flags = AssetFlag_Unloaded;
			TextureAssetRefs[i] = 0;
			NS_CONSOLE_Debug(AssetLog, "Unloaded texture asset [%s]", *TextureAssetNames[i]);
			nsTextureManager::Get().DestroyTexture(TextureAssets[i]);
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

	const int index = MaterialAssetNames.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load material asset. Material asset with name [%s] not found!", *name);
		return nsSharedMaterialAsset();
	}

	if (MaterialAssets[index] != nsMaterialID::INVALID)
	{
		MaterialAssetFlags[index] = AssetFlag_Loaded;
		return nsSharedMaterialAsset(index, name, MaterialAssets[index]);
	}


	const nsString assetFile = nsString::Format("%s/%s%s", *MaterialAssetPaths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

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


	MaterialAssetFlags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded mesh asset [%s]", *name);

	return nsSharedMaterialAsset(index, name, MaterialAssets[index]);
}


void nsAssetManager::Internal_AddMaterialAssetReference(int index, nsMaterialID material)
{

}


void nsAssetManager::Internal_RemoveMaterialAssetReference(int index, nsMaterialID material)
{

}


void nsAssetManager::UpdateMaterialAssets()
{

}




// ====================================================================================================================================================================== //
// MODEL
// ====================================================================================================================================================================== //
void nsAssetManager::SaveModelAsset(nsName name, const nsAssetModelMeshes& meshes, const nsString& folderPath, bool bIsEngineAsset)
{
	const nsString assetPath = nsString::Format("%s/%s", bIsEngineAsset ? *EngineAssetsPath : *GameAssetsPath, *folderPath);

	if (ModelAssetNames.Find(name) != NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save model asset. Model asset with name [%s] already exists!", *name);
		return;
	}

	if (!nsFileSystem::FolderCreate(assetPath))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to save model asset [%s]. Create folder failed [%s] failed!", *name, *assetPath);
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

	ModelAssetNames.Add(name);
	ModelAssetPaths.Add(assetPath);
	ModelAssetFlags.Add(AssetFlag_Loaded);
	ModelAssetRefs.Add(0);
	ModelAssetMeshes.Add(meshes);
}


nsSharedModelAsset nsAssetManager::LoadModelAsset(const nsName& name)
{
	if (name.GetLength() == 0)
	{
		return nsSharedModelAsset();
	}

	const int index = ModelAssetNames.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to load model asset. Model asset with name [%s] not found!", *name);
		return nsSharedModelAsset();
	}

	if (ModelAssetMeshes[index].GetCount() > 0)
	{
		ModelAssetFlags[index] = AssetFlag_Loaded;
		return nsSharedModelAsset(index, name, ModelAssetMeshes[index]);
	}

	const nsString assetFile = nsString::Format("%s/%s%s", *ModelAssetPaths[index], *name, NS_ENGINE_ASSET_FILE_EXTENSION);

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

			ModelAssetMeshes[index].Add(mesh);
		}
	}


	ModelAssetFlags[index] = AssetFlag_Loaded;

	NS_CONSOLE_Debug(AssetLog, "Loaded model asset [%s]", *name);

	return nsSharedModelAsset(index, name, ModelAssetMeshes[index]);
}


void nsAssetManager::Internal_AddModelAssetReference(int index)
{
	NS_Assert(index >= 0 && index < ModelAssetMeshes.GetCount());

	ModelAssetRefs[index]++;
}


void nsAssetManager::Internal_RemoveModelAssetReference(int index)
{
	NS_Assert(index >= 0 && index < ModelAssetMeshes.GetCount());

	if (--ModelAssetRefs[index] <= 0)
	{
		ModelAssetRefs[index] = 0;
		ModelAssetFlags[index] |= AssetFlag_PendingUnload;
		NS_CONSOLE_Debug(AssetLog, "Mark model asset [%s] pending unload (RefCount = 0)", *ModelAssetNames[index]);
	}
}


void nsAssetManager::UpdateModelAssets()
{
	for (int i = 0; i < ModelAssetFlags.GetCount(); ++i)
	{
		uint8& flags = ModelAssetFlags[i];

		if (flags & AssetFlag_PendingUnload)
		{
			NS_Assert(ModelAssetRefs[i] == 0);
			NS_CONSOLE_Debug(AssetLog, "Unloaded model asset [%s]", *ModelAssetNames[i]);
			flags = AssetFlag_Unloaded;
			ModelAssetRefs[i] = 0;

			const int meshCount = ModelAssetMeshes[i].GetCount();
			NS_Assert(meshCount > 0);

			for (int m = 0; m < meshCount; ++m)
			{
				nsMeshManager::Get().DestroyMesh(ModelAssetMeshes[i][m]);
			}

			ModelAssetMeshes[i].Clear();
		}
	}
}
