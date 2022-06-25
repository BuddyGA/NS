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

	nsAssetImportOption_Model importOptionModel{};

	//importOptionModel.SourceFile = "../../../Assets/Models/mesh_wall.glb";
	//importOptionModel.MeshScaleMultiplier = 100.0f;

	importOptionModel.SourceFile = "../../../Assets/Models/mesh_boombox.glb";
	importOptionModel.MeshScaleMultiplier = 1000.0f;

	//importOptionModel.SourceFile = "../../../Assets/Models/mesh_cube_two_mats.glb";
	//importOptionModel.MeshScaleMultiplier = 100.0f;

	importOptionModel.bImportMesh = true;
	ImportAssetFromModelFile(importOptionModel, "Models");
	*/

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

	NS_CONSOLE_Log(AssetLog, "Imported texture from file [Source: %s]", *option.SourceFile);

	nsAssetManager::Get().SaveTextureAsset(fileName, texture, dstFolderPath, false);
}






// =============================================================================================================================================================== //
// IMPORT FROM MODEL FILE
// =============================================================================================================================================================== //
#include "ThirdParty/json.hpp"



struct nsGLTF_Mesh
{
	nsName Name;
	nsTArray<nsVector3> VertexPositions;
	nsTArray<nsVector3> VertexNormals;
	nsTArray<nsVector4> VertexTangents;
	nsTArray<nsVector2> VertexTexCoords_0;
	nsTArray<nsVector2> VertexTexCoords_1;
	nsTArray<nsVector4> VertexJoints;
	nsTArray<nsVector4> VertexWeights;
	nsTArray<uint32> VertexIndices;

public:
	nsGLTF_Mesh()
	{
		Name = "";
	}

};


struct nsGLTF_Material
{
	nsName Name;

};


struct nsGLTF_Animation
{
	nsName Name;

};


struct nsGLTF_Model
{
	nsName Name;
	nsTArrayInline<nsGLTF_Mesh, NS_ENGINE_ASSET_MODEL_MAX_MESH> Meshes;
};



template<typename T>
static bool ns_GLTF_GetVertexData(nsTArray<T>& outData, const char* attributeName, nlohmann::json& jsonVertexAttributes, const nlohmann::json& jsonAccessors, const nlohmann::json& jsonBufferViews, const uint8* binData, int expectedComponentType, const char* expectedComponentTypeName, bool bAllowNull)
{
	NS_Validate(attributeName);
	NS_Validate(binData);

	const nlohmann::json jsonVertex = jsonVertexAttributes[attributeName];

	if (bAllowNull && jsonVertex.is_null())
	{
		return false;
	}

	NS_Validate(!jsonVertex.is_null());

	const int accessorIndex = jsonVertex;
	nlohmann::json jsonAccessor = jsonAccessors[accessorIndex];

	const int componentType = jsonAccessor["componentType"];
	NS_Validate(componentType == expectedComponentType);

	const nsName typeName = jsonAccessor["type"].get<std::string>().c_str();
	NS_Validate(typeName == expectedComponentTypeName);

	const int count = jsonAccessor["count"];
	NS_Validate(count > 0);

	const int bufferViewIndex = jsonAccessor["bufferView"];
	nlohmann::json jsonBufferView = jsonBufferViews[bufferViewIndex];
	
	const int bufferIndex = jsonBufferView["buffer"];
	NS_Validate(bufferIndex == 0);

	const int bufferOffset = jsonBufferView["byteOffset"];

	outData.Resize(count);
	nsPlatform::Memory_Copy(outData.GetData(), binData + bufferOffset, sizeof(T) * count);

	return true;
}


static void ns_ImportModel_GLB(const nsAssetImportOption_Model& option, const nsString& dstFolderPath)
{
	static constexpr uint32 GLB_MAGIC			= (0x46546C67); // glTF
	static constexpr uint32 GLB_CHUNK_TYPE_JSON = (0x4E4F534A); // JSON
	static constexpr uint32 GLB_CHUNK_TYPE_BIN	= (0x004E4942); // BIN


	nsTArray<uint8> bytes;

	if (!nsFileSystem::FileReadBinary(option.SourceFile, bytes))
	{
		NS_CONSOLE_Warning(AssetLog, "Fail to import model from file [%s]. Fail to read data!", *option.SourceFile);
		return;
	}

	nsBinaryStreamReader reader(bytes);

	// Header
	{
		uint32 magic = 0;
		reader | magic;
		NS_ValidateV(magic == GLB_MAGIC, "Invalid GLB file!");

		uint32 version = 0;
		reader | version;
		NS_ValidateV(version == 2, "Invalid GLB version!");

		uint32 length = 0;
		reader | length;
		NS_ValidateV(length > 12, "Empty GLB data!");
	}
	

	// Chunk-0 (JSON)
	nlohmann::json jsonData;
	{
		uint32 chunkLength = 0;
		reader | chunkLength;
		NS_ValidateV(chunkLength > 0, "Empty chunk-0 data size!");

		uint32 chunkType = 0;
		reader | chunkType;
		NS_ValidateV(chunkType == GLB_CHUNK_TYPE_JSON, "Invalid chunk-0 type!");

		nsString jsonString;
		jsonString.Resize(chunkLength);
		reader.SerializeData(*jsonString, chunkLength);
		NS_LogDebug(AssetLog, "%s", *jsonString);

		jsonData = nlohmann::json::parse(*jsonString);
	}


	// Chunk-1 (BIN)
	const uint8* binData = nullptr;
	{
		uint32 chunkLength = 0;
		reader | chunkLength;
		NS_ValidateV(chunkLength > 0, "Empty chunk-1 data size!");

		uint32 chunkType = 0;
		reader | chunkType;
		NS_ValidateV(chunkType == GLB_CHUNK_TYPE_BIN, "Invalid chunk-1 type!");

		binData = reader.GetBufferData() + reader.GetCurrentOffset();
	}


	const nlohmann::json jsonAccessors = jsonData["accessors"];
	NS_Validate(jsonAccessors.size() > 0);
	
	const nlohmann::json jsonBufferViews = jsonData["bufferViews"];
	NS_Validate(jsonBufferViews.size() > 0);


	// Import meshes
	if (option.bImportMesh)
	{
		const nlohmann::json jsonModels = jsonData["meshes"];
		const int modelCount = static_cast<int>(jsonModels.size());
		NS_Validate(modelCount == 1);

		nsGLTF_Model gltfModel;
		const nlohmann::json jsonModel = jsonModels[0];
		gltfModel.Name = nsName::Format("mdl_%s", jsonModel["name"].get<std::string>().c_str());

		const nlohmann::json jsonMeshes = jsonModel["primitives"];
		const int meshCount = static_cast<int>(jsonMeshes.size());

		for (int i = 0; i < meshCount; ++i)
		{
			nsGLTF_Mesh& gltfMesh = gltfModel.Meshes.Add();
			gltfMesh.Name = nsName::Format("%s_%i", *gltfModel.Name, i);

			const nlohmann::json jsonMesh = jsonMeshes[i];
			nlohmann::json jsonVertexAttributes = jsonMesh["attributes"];

			// Vertex position
			ns_GLTF_GetVertexData(gltfMesh.VertexPositions, "POSITION", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5126, "VEC3", false);

			for (int i = 0; i < gltfMesh.VertexPositions.GetCount(); ++i)
			{
				gltfMesh.VertexPositions[i] *= option.MeshScaleMultiplier;
			}

			// Vertex normal
			ns_GLTF_GetVertexData(gltfMesh.VertexNormals, "NORMAL", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5126, "VEC3", false);
			NS_Validate(gltfMesh.VertexNormals.GetCount() == gltfMesh.VertexPositions.GetCount());

			// Vertex tangent
			ns_GLTF_GetVertexData(gltfMesh.VertexTangents, "TANGENT", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5126, "VEC4", false);
			NS_Validate(gltfMesh.VertexTangents.GetCount() == gltfMesh.VertexPositions.GetCount());

			// Vertex texCoord_0
			ns_GLTF_GetVertexData(gltfMesh.VertexTexCoords_0, "TEXCOORD_0", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5126, "VEC2", false);
			NS_Validate(gltfMesh.VertexTexCoords_0.GetCount() == gltfMesh.VertexPositions.GetCount());

			// Vertex joints
			if (ns_GLTF_GetVertexData(gltfMesh.VertexJoints, "JOINTS_0", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5123, "VEC4", !option.bImportSkeleton))
			{
				NS_Validate(gltfMesh.VertexJoints.GetCount() == gltfMesh.VertexPositions.GetCount());
			}

			// Vertex weights
			if (ns_GLTF_GetVertexData(gltfMesh.VertexWeights, "WEIGHTS_0", jsonVertexAttributes, jsonAccessors, jsonBufferViews, binData, 5126, "VEC4", !option.bImportSkeleton))
			{
				NS_Validate(gltfMesh.VertexWeights.GetCount() == gltfMesh.VertexPositions.GetCount());
			}

			// Vertex indices
			const nlohmann::json jsonVertexIndices = jsonMesh["indices"];
			{
				NS_Validate(!jsonVertexIndices.is_null());

				const int accessorIndex = jsonVertexIndices;
				nlohmann::json jsonAccessor = jsonAccessors[accessorIndex];

				const int componentType = jsonAccessor["componentType"];
				NS_Validate(componentType == 5123 || componentType == 5125);

				const nsName typeName = jsonAccessor["type"].get<std::string>().c_str();
				NS_Validate(typeName == "SCALAR");

				const int count = jsonAccessor["count"];
				NS_Validate(count > 0);

				const int bufferViewIndex = jsonAccessor["bufferView"];
				nlohmann::json jsonBufferView = jsonBufferViews[bufferViewIndex];

				const int bufferIndex = jsonBufferView["buffer"];
				NS_Validate(bufferIndex == 0);

				const int bufferOffset = jsonBufferView["byteOffset"];

				gltfMesh.VertexIndices.Resize(count);

				// UINT16
				if (componentType == 5123)
				{
					const uint16* indices = reinterpret_cast<const uint16*>(binData + bufferOffset);

					for (int i = 0; i < count; ++i)
					{
						gltfMesh.VertexIndices[i] = indices[i];
					}
				}
				// UINT32
				else
				{
					nsPlatform::Memory_Copy(gltfMesh.VertexIndices.GetData(), binData + bufferOffset, sizeof(uint32) * count);
				}
			}
		}
		

		nsMeshManager& meshManager = nsMeshManager::Get();
		nsAssetModelMeshes modelMeshes;

		for (int i = 0; i < meshCount; ++i)
		{
			const nsGLTF_Mesh& gltfMesh = gltfModel.Meshes[i];
			const int vertexCount = gltfMesh.VertexPositions.GetCount();

			const nsMeshID newMesh = meshManager.CreateMesh(gltfMesh.Name);
			nsMeshVertexData& vertexData = meshManager.GetMeshVertexData(newMesh, 0);
			vertexData.Positions = gltfMesh.VertexPositions;

			vertexData.Attributes.Resize(vertexCount);

			const nsTArray<nsVector3>& vertexNormals = gltfMesh.VertexNormals;
			const nsTArray<nsVector4>& vertexTangents = gltfMesh.VertexTangents;
			const nsTArray<nsVector2>& vertexTexCoords = gltfMesh.VertexTexCoords_0;

			for (int v = 0; v < vertexCount; ++v)
			{
				nsVertexMeshAttribute& attribute = vertexData.Attributes[v];
				attribute.Normal = vertexNormals[v];
				attribute.Tangent = vertexTangents[v].ToVector3();
				attribute.TexCoord = vertexTexCoords[v];
			}

			vertexData.Indices = gltfMesh.VertexIndices;

			modelMeshes.Add(newMesh);
		}

		nsAssetManager::Get().SaveModelAsset(gltfModel.Name, modelMeshes, dstFolderPath, false);
	}


	// TODO: Import materials
	

	// TODO: Import textures
	if (option.bImportTexture)
	{

	}


	// TODO: Import animation
	if (option.bImportAnimation)
	{

	}
}


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
		ns_ImportModel_GLB(option, dstFolderPath);
	}
}
