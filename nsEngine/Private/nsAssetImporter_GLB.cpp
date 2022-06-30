#include "nsAssetImporter.h"
#include "nsFileSystem.h"
#include "nsConsole.h"
#include "nsAnimation.h"
#include "ThirdParty/json.hpp"


static nsLogCategory AssetImporterGLB("nsAssetImporterGLB", nsELogVerbosity::LV_WARNING);


static constexpr uint32 GLB_MAGIC			= (0x46546C67); // glTF
static constexpr uint32 GLB_CHUNK_TYPE_JSON = (0x4E4F534A); // JSON
static constexpr uint32 GLB_CHUNK_TYPE_BIN	= (0x004E4942); // BIN



struct nsGLB_Node
{
	nsName Name;
	nsTransform Transform;
	nsTArrayInline<int, 16> Children;
};



struct nsGLB_Mesh
{
	nsName Name;
	nsTArray<nsVector3> VertexPositions;
	nsTArray<nsVector3> VertexNormals;
	nsTArray<nsVector4> VertexTangents;
	nsTArray<nsVector2> VertexTexCoords_0;
	nsTArray<nsVector2> VertexTexCoords_1;
	nsTArray<nsVector4> VertexWeights;
	nsTArray<uint32> VertexJoints;
	nsTArray<uint32> VertexIndices;
};



struct nsGLB_Model
{
	nsName Name;
	nsTArrayInline<nsGLB_Mesh, NS_ENGINE_ASSET_MODEL_MAX_MESH> Meshes;
};



struct nsGLB_Bone
{
	nsName Name;
	nsMatrix4 InverseBindPoseTransform;
	nsTransform LocalTransform;
	int NodeId;
	int ParentId;


public:
	nsGLB_Bone()
	{
		InverseBindPoseTransform = nsMatrix4::IDENTITY;
		NodeId = -1;
		ParentId = -1;
	}

};


struct nsGLB_Skeleton
{
	nsName Name;
	nsTArrayInline<nsGLB_Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> Bones;
};



template<typename T = void>
static const T* ns_GLB_GetBufferData(int& outCount, int& outComponentType, const uint8* binData, const nlohmann::json& jsonData, int accessorIndex)
{
	NS_Validate(binData);

	const nlohmann::json& jsonAccessorArray = jsonData["accessors"];
	const int accessorCount = static_cast<int>(jsonAccessorArray.size());
	NS_Validate(accessorIndex >= 0 && accessorIndex < accessorCount);

	const nlohmann::json& jsonAccessor = jsonAccessorArray[accessorIndex];
	outCount = jsonAccessor["count"];
	outComponentType = jsonAccessor["componentType"];
	const int bufferViewIndex = jsonAccessor["bufferView"];

	const nlohmann::json& jsonBufferViewArray = jsonData["bufferViews"];
	const int bufferViewCount = static_cast<int>(jsonBufferViewArray.size());
	NS_Validate(bufferViewIndex >= 0 && bufferViewIndex < bufferViewCount);

	const nlohmann::json& jsonBufferView = jsonBufferViewArray[bufferViewIndex];
	const int bufferDataIndex = jsonBufferView["buffer"];
	NS_Validate(bufferDataIndex == 0);

	const int bufferOffset = jsonBufferView["byteOffset"];
	const int bufferSizeBytes = jsonBufferView["byteLength"];
	NS_Validate(bufferSizeBytes > 0);

	return reinterpret_cast<const T*>(binData + bufferOffset);
}



template<typename T>
static bool ns_GLB_GetVertexData(nsTArray<T>& outVertexData, const char* attributeName, const uint8* binData, const nlohmann::json& jsonData, const nlohmann::json& jsonVertexAttributes)
{
	NS_Validate(attributeName);
	NS_Validate(binData);

	if (!jsonVertexAttributes.contains(attributeName))
	{
		return false;
	}

	const int accessorIndex = jsonVertexAttributes[attributeName];
	int vertexCount = 0;
	int componentType = 0;
	const T* vertexData = ns_GLB_GetBufferData<T>(vertexCount, componentType, binData, jsonData, accessorIndex);
	NS_Validate(vertexData);

	outVertexData.InsertAt(vertexData, vertexCount);

	return true;
}



static void ns_GLB_ImportModels(const nsAssetImportOption_Model& option, const nsString& dstFolderPath, const uint8* binData, const nlohmann::json& jsonData)
{
	if (!option.bImportMesh)
	{
		return;
	}

	if (!jsonData.contains("meshes"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import model from file [%s]. <meshes> not found in json data!", *option.SourceFile);
		return;
	}

	const nlohmann::json& jsonAccessorArray = jsonData["accessors"];
	const nlohmann::json& jsonBufferViewArray = jsonData["bufferViews"];
	const nlohmann::json& jsonModelArray = jsonData["meshes"];
	const int modelCount = static_cast<int>(jsonModelArray.size());
	nsTArray<nsGLB_Model> glbModels(modelCount, nsGLB_Model());

	for (int i = 0; i < modelCount; ++i)
	{
		const nlohmann::json& jsonModel = jsonModelArray[i];
		
		nsGLB_Model& glbModel = glbModels[i];
		glbModel.Name = nsName::Format("mdl_%s", jsonModel["name"].get<std::string>().c_str());

		if (!jsonModel.contains("primitives"))
		{
			NS_CONSOLE_Warning(AssetImporterGLB, "Fail to import model [%s] from file [%s]. <primitives> data not found!", *glbModel.Name, *option.SourceFile);
			continue;
		}

		const nlohmann::json jsonMeshArray = jsonModel["primitives"];
		const int meshCount = static_cast<int>(jsonMeshArray.size());

		for (int m = 0; m < meshCount; ++m)
		{
			nsGLB_Mesh& mesh = glbModel.Meshes.Add();
			mesh.Name = nsName::Format("%s_%i", *glbModel.Name, m);

			const nlohmann::json& jsonMesh = jsonMeshArray[m];
			const nlohmann::json& jsonVertexAttributes = jsonMesh["attributes"];

			// Vertex position
			bool bSuccess = ns_GLB_GetVertexData(mesh.VertexPositions, "POSITION", binData, jsonData, jsonVertexAttributes);
			NS_ValidateV(bSuccess, "Fail to get vertex [POSITION] data from model [%s] in GLB file [%s]", *mesh.Name, *option.SourceFile);

			for (int vtx = 0; vtx < mesh.VertexPositions.GetCount(); ++vtx)
			{
				mesh.VertexPositions[vtx] *= option.MeshScaleMultiplier;
			}

			// Vertex normal
			bSuccess = ns_GLB_GetVertexData(mesh.VertexNormals, "NORMAL", binData, jsonData, jsonVertexAttributes);
			NS_ValidateV(bSuccess, "Fail to get vertex [NORMAL] data from model [%s] in GLB file [%s]", *mesh.Name, *option.SourceFile);
			NS_Validate(mesh.VertexNormals.GetCount() == mesh.VertexPositions.GetCount());

			// Vertex tangent
			bSuccess = ns_GLB_GetVertexData(mesh.VertexTangents, "TANGENT", binData, jsonData, jsonVertexAttributes);
			NS_ValidateV(bSuccess, "Fail to get vertex [TANGENT] data from model [%s] in GLB file [%s]", *mesh.Name, *option.SourceFile);
			NS_Validate(mesh.VertexTangents.GetCount() == mesh.VertexPositions.GetCount());

			// Vertex texCoord_0
			bSuccess = ns_GLB_GetVertexData(mesh.VertexTexCoords_0, "TEXCOORD_0", binData, jsonData, jsonVertexAttributes);
			NS_ValidateV(bSuccess, "Fail to get vertex [TEXCOORD_0] data from model [%s] in GLB file [%s]", *mesh.Name, *option.SourceFile);
			NS_Validate(mesh.VertexTexCoords_0.GetCount() == mesh.VertexPositions.GetCount());

			// Vertex weights
			const bool bHasVertexWeights = ns_GLB_GetVertexData(mesh.VertexWeights, "WEIGHTS_0", binData, jsonData, jsonVertexAttributes);
			if (bHasVertexWeights)
			{
				NS_Validate(mesh.VertexWeights.GetCount() == mesh.VertexPositions.GetCount());
			}

			// Vertex joints
			const bool bHasVertexJoints = ns_GLB_GetVertexData(mesh.VertexJoints, "JOINTS_0", binData, jsonData, jsonVertexAttributes);
			if (bHasVertexJoints)
			{
				NS_Validate(mesh.VertexJoints.GetCount() == mesh.VertexPositions.GetCount());
			}

			// Vertex indices
			{
				const int accessorIndex = jsonMesh["indices"];
				int indexCount = 0;
				int componentType = 0;
				const void* indicesBufferData = ns_GLB_GetBufferData(indexCount, componentType, binData, jsonData, accessorIndex);
				mesh.VertexIndices.Resize(indexCount);

				// UINT16
				if (componentType == 5123)
				{
					const uint16* indices = reinterpret_cast<const uint16*>(indicesBufferData);

					for (int idx = 0; idx < indexCount; ++idx)
					{
						mesh.VertexIndices[idx] = indices[idx];
					}
				}
				// UINT32
				else
				{
					nsPlatform::Memory_Copy(mesh.VertexIndices.GetData(), indicesBufferData, sizeof(uint32) * indexCount);
				}
			}
		}


		nsMeshManager& meshManager = nsMeshManager::Get();
		nsAssetModelMeshes modelMeshes;

		for (int i = 0; i < meshCount; ++i)
		{
			const nsGLB_Mesh& mesh = glbModel.Meshes[i];
			const int vertexCount = mesh.VertexPositions.GetCount();

			const nsMeshID newMesh = meshManager.CreateMesh(mesh.Name);
			nsMeshVertexData& vertexData = meshManager.GetMeshVertexData(newMesh, 0);
			vertexData.Positions = mesh.VertexPositions;

			vertexData.Attributes.Resize(vertexCount);
			vertexData.Skins.Resize(vertexCount);

			const nsTArray<nsVector3>& vertexNormals = mesh.VertexNormals;
			const nsTArray<nsVector4>& vertexTangents = mesh.VertexTangents;
			const nsTArray<nsVector2>& vertexTexCoords = mesh.VertexTexCoords_0;
			const nsTArray<nsVector4>& vertexWeights = mesh.VertexWeights;
			const nsTArray<uint32>& vertexJoints = mesh.VertexJoints;

			for (int v = 0; v < vertexCount; ++v)
			{
				nsVertexMeshAttribute& attribute = vertexData.Attributes[v];
				attribute.Normal = vertexNormals[v];
				attribute.Tangent = vertexTangents[v].ToVector3();
				attribute.TexCoord = vertexTexCoords[v];

				if (vertexWeights.GetCount() > 0)
				{
					NS_Validate(vertexJoints.GetCount() == vertexWeights.GetCount());
					nsVertexMeshSkin& skin = vertexData.Skins[v];
					skin.Weights = vertexWeights[v];
					skin.Joints = vertexJoints[v];
				}
			}

			vertexData.Indices = mesh.VertexIndices;

			modelMeshes.Add(newMesh);
		}

		nsAssetManager::Get().SaveModelAsset(glbModel.Name, modelMeshes, dstFolderPath, false);

		NS_CONSOLE_Log(AssetImporterGLB, "Imported model [%s] from source file [%s]", *glbModel.Name, *option.SourceFile);
	}
}



static nsGLB_Bone* ns_GLB_FindBoneWithNodeIndex(nsTArrayInline<nsGLB_Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE>& bones, int nodeIndex)
{
	for (int i = 0; i < bones.GetCount(); ++i)
	{
		if (bones[i].NodeId == nodeIndex)
		{
			return &bones[i];
		}
	}

	return nullptr;
};


static void ns_GLB_ImportSkeletons(const nsAssetImportOption_Model& option, const nsString& dstFolderPath, const uint8* binData, const nlohmann::json& jsonData, const nsTArray<nsGLB_Node>& glbNodes)
{
	if (!option.bImportSkeleton)
	{
		return;
	}

	if (!jsonData.contains("skins"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import skeleton from file [%s]. <skins> not found in json data!", *option.SourceFile);
		return;
	}

	const nlohmann::json& jsonAccessorArray = jsonData["accessors"];
	const nlohmann::json& jsonBufferViewArray = jsonData["bufferViews"];
	const nlohmann::json& jsonSkeletonArray = jsonData["skins"];
	const int skeletonCount = static_cast<int>(jsonSkeletonArray.size());
	const nsMatrix4 scaleMatrix = nsMatrix4::Scale(100.0f);

	for (int i = 0; i < skeletonCount; ++i)
	{
		const nlohmann::json& jsonSkeleton = jsonSkeletonArray[i];

		nsGLB_Skeleton glbSkeleton;
		glbSkeleton.Name = nsName::Format("skl_%s", jsonSkeleton["name"].get<std::string>().c_str());
		const nlohmann::json& jsonBoneArray = jsonSkeleton["joints"];
		const int boneCount = static_cast<int>(jsonBoneArray.size());
		glbSkeleton.Bones.Resize(boneCount);

		const int accessorIndex = jsonSkeleton["inverseBindMatrices"];
		int matrixCount = 0;
		int componentType = 0;
		const nsMatrix4* inverseBindPoseMatrices = ns_GLB_GetBufferData<nsMatrix4>(matrixCount, componentType, binData, jsonData, accessorIndex);
		NS_Validate(matrixCount == boneCount);

		for (int j = 0; j < boneCount; ++j)
		{
			const int nodeIndex = jsonBoneArray[j];
			const nsGLB_Node& node = glbNodes[nodeIndex];

			const nsMatrix4 invBindMatrix = inverseBindPoseMatrices[j];
			nsGLB_Bone& glbBone = glbSkeleton.Bones[j];
			glbBone.Name = node.Name;
			glbBone.InverseBindPoseTransform = invBindMatrix * scaleMatrix;
			glbBone.LocalTransform = node.Transform;
			glbBone.LocalTransform.Position *= 100.0f;
			glbBone.NodeId = nodeIndex;
			glbBone.ParentId = -1;
		}


		// Adjust bone hierarchy
		for (int j = 0; j < boneCount; ++j)
		{
			const int nodeIndex = jsonBoneArray[j];
			const nsGLB_Node& node = glbNodes[nodeIndex];

			for (int c = 0; c < node.Children.GetCount(); ++c)
			{
				const int childNodeIndex = node.Children[c];
				nsGLB_Bone* childBone = ns_GLB_FindBoneWithNodeIndex(glbSkeleton.Bones, childNodeIndex);
				NS_Validate(childBone);
				childBone->ParentId = j;
			}
		}


		nsAnimationManager& animationManager = nsAnimationManager::Get();
		const nsAnimationSkeletonID skeleton = animationManager.CreateSkeleton(glbSkeleton.Name);
		nsAnimationSkeletonData& data = animationManager.GetSkeletonData(skeleton);
		data.BoneNames.Resize(boneCount);
		data.BoneDatas.Resize(boneCount);

		for (int j = 0; j < boneCount; ++j)
		{
			const nsGLB_Bone& glbBone = glbSkeleton.Bones[j];
			data.BoneNames[j] = glbBone.Name;

			nsAnimationSkeletonData::Bone& bone = data.BoneDatas[j];
			bone.InverseBindPoseTransform = glbBone.InverseBindPoseTransform;
			bone.PoseTransform = nsMatrix4::IDENTITY;
			bone.LocalTransform = glbBone.LocalTransform;
			bone.ParentId = glbBone.ParentId;
		}

		nsAssetManager::Get().SaveSkeletonAsset(glbSkeleton.Name, skeleton, dstFolderPath, false);

		NS_CONSOLE_Log(AssetImporterGLB, "Imported skeleton [%s] from source file [%s]", *glbSkeleton.Name, *option.SourceFile);
	}
}



void nsAssetImporter::ImportAssetFromModelFile_GLB(const nsAssetImportOption_Model& option, const nsString& dstFolderPath)
{
	nsTArray<uint8> bytes;

	if (!nsFileSystem::FileReadBinary(option.SourceFile, bytes))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Fail to read data!", *option.SourceFile);
		return;
	}

	nsBinaryStreamReader reader(bytes);

	// Header
	{
		uint32 magic = 0;
		reader | magic;

		if (magic != GLB_MAGIC)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Invalid GLB file!", *option.SourceFile);
			return;
		}

		uint32 version = 0;
		reader | version;
		
		if (version != 2)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Asset importer for GLB only support GLB file version 2! [Source GLB File Version: %u]", *option.SourceFile, version);
			return;
		}

		uint32 length = 0;
		reader | length;

		if (length <= 12)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Emtpy GLB data!", *option.SourceFile);
			return;
		}
	}


	// Chunk-0 (JSON)
	nlohmann::json jsonData;
	{
		uint32 chunkLength = 0;
		reader | chunkLength;

		if (chunkLength == 0)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Empty chunk-0 (JSON) data size!", *option.SourceFile);
			return;
		}

		uint32 chunkType = 0;
		reader | chunkType;

		if (chunkType != GLB_CHUNK_TYPE_JSON)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Invalid chunk-0 type!", *option.SourceFile);
			return;
		}

		nsString jsonString;
		jsonString.Resize(chunkLength);
		reader.SerializeData(*jsonString, chunkLength);

		nsFileSystem::FileWriteText("debug_glb_import.json", *jsonString);

		jsonData = nlohmann::json::parse(*jsonString);
	}


	// Chunk-1 (BIN)
	const uint8* binData = nullptr;
	{
		uint32 chunkLength = 0;
		reader | chunkLength;

		if (chunkLength == 0)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Empty chunk-1 (Binary) data size!", *option.SourceFile);
			return;
		}

		uint32 chunkType = 0;
		reader | chunkType;

		if (chunkType != GLB_CHUNK_TYPE_BIN)
		{
			NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. Invalid chunk-1 type!", *option.SourceFile);
			return;
		}

		binData = reader.GetBufferData() + reader.GetCurrentOffset();
		NS_Validate(binData);
	}


	if (!jsonData.contains("accessors"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. <accessors> not found in json data!", *option.SourceFile);
		return;
	}

	if (!jsonData.contains("bufferViews"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import asset from file [%s]. <bufferViews> not found in json data!", *option.SourceFile);
		return;
	}


	// Validate nodes
	nlohmann::json jsonNodeArray;
	int nodeCount = 0;

	if (jsonData.contains("nodes"))
	{
		jsonNodeArray = jsonData["nodes"];
		nodeCount = static_cast<int>(jsonNodeArray.size());
	}

	nsTArray<nsGLB_Node> glbNodes(nodeCount, nsGLB_Node());

	for (int i = 0; i < nodeCount; ++i)
	{
		const nlohmann::json& jsonNode = jsonNodeArray[i];

		nsGLB_Node& node = glbNodes[i];
		node.Name = jsonNode["name"].get<std::string>().c_str();

		if (jsonNode.contains("translation"))
		{
			const nlohmann::json& translation = jsonNode["translation"];
			node.Transform.Position = nsVector3(translation[0], translation[1], translation[2]);
		}

		if (jsonNode.contains("rotation"))
		{
			const nlohmann::json& rotation = jsonNode["rotation"];
			node.Transform.Rotation = nsQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
		}

		if (jsonNode.contains("children"))
		{
			const nlohmann::json& children = jsonNode["children"];
			const int childrenCount = static_cast<int>(children.size());

			for (int c = 0; c < childrenCount; ++c)
			{
				node.Children.Add(children[c]);
			}
		}
	}

	ns_GLB_ImportModels(option, dstFolderPath, binData, jsonData);
	ns_GLB_ImportSkeletons(option, dstFolderPath, binData, jsonData, glbNodes);
}
