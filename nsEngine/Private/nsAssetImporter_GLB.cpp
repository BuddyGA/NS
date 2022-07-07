#include "nsAssetImporter.h"
#include "nsFileSystem.h"
#include "nsAnimationManager.h"
#include "nsConsole.h"
#include "ThirdParty/json.hpp"


static nsLogCategory AssetImporterGLB("nsAssetImporterGLB", nsELogVerbosity::LV_INFO);


static constexpr uint32 GLB_MAGIC					= 0x46546C67; // glTF
static constexpr uint32 GLB_CHUNK_TYPE_JSON			= 0x4E4F534A; // JSON
static constexpr uint32 GLB_CHUNK_TYPE_BIN			= 0x004E4942; // BIN
static constexpr int GLB_COMPONENT_TYPE_INT8		= 5120;
static constexpr int GLB_COMPONENT_TYPE_UINT8		= 5121;
static constexpr int GLB_COMPONENT_TYPE_INT16		= 5122;
static constexpr int GLB_COMPONENT_TYPE_UINT16		= 5123;
static constexpr int GLB_COMPONENT_TYPE_UINT32		= 5125;
static constexpr int GLB_COMPONENT_TYPE_FLOAT		= 5126;



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




// ================================================================================================================================================================ //
// MODEL/MESHES
// ================================================================================================================================================================ //
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
				if (componentType == GLB_COMPONENT_TYPE_UINT16)
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
					NS_Validate(componentType == GLB_COMPONENT_TYPE_UINT32);
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




// ================================================================================================================================================================ //
// SKELETON/ANIMATION
// ================================================================================================================================================================ //
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

typedef nsTArrayInline<nsGLB_Bone, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> nsGLB_BoneHierarchy;



struct nsGLB_Skeleton
{
	nsName Name;
	nsGLB_BoneHierarchy Bones;
};



struct nsGLB_Animation
{
	template<typename T>
	struct TChannel
	{
		T Value;
		float Timestamp;
	};

	struct KeyFrame
	{
		nsTArray<TChannel<nsVector3>> Positions;
		nsTArray<TChannel<nsQuaternion>> Rotations;
		nsTArray<TChannel<nsVector3>> Scales;
	};

	// Animation name
	nsName Name;

	// Frame count
	int FrameCount;

	// Duration (seconds)
	float Duration;

	// Key frame for each bone
	nsTArrayInline<KeyFrame, NS_ENGINE_ANIMATION_SKELETON_MAX_BONE> KeyFrames;


public:
	nsGLB_Animation()
	{
		FrameCount = 0;
		Duration = 0.0f;
	}

};


static nsGLB_Bone* ns_GLB_FindBoneWithNodeIndex(nsGLB_BoneHierarchy& bones, int nodeIndex, int* outBoneId = nullptr)
{
	for (int i = 0; i < bones.GetCount(); ++i)
	{
		if (bones[i].NodeId == nodeIndex)
		{
			if (outBoneId)
			{
				*outBoneId = i;
			}

			return &bones[i];
		}
	}

	return nullptr;
};


static void ns_GLB_ImportAnimations(const nsAssetImportOption_Model& option, const nsString& dstFolderPath, const uint8* binData, const nlohmann::json& jsonData, nsGLB_Skeleton& glbSkeleton, const nsTArray<nsGLB_Node>& glbNodes)
{
	if (!option.bImportAnimation)
	{
		return;
	}

	if (!jsonData.contains("animations"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import animation from file [%s]. <animations> not found in json data!", *option.SourceFile);
		return;
	}

	const nlohmann::json& jsonAnimationArray = jsonData["animations"];
	const int animationCount = static_cast<int>(jsonAnimationArray.size());

	for (int i = 0; i < animationCount; ++i)
	{
		const nlohmann::json& jsonAnimation = jsonAnimationArray[i];

		nsGLB_Animation glbAnimation;
		glbAnimation.Name = nsName::Format("anim_%s", jsonAnimation["name"].get<std::string>().c_str());
		glbAnimation.KeyFrames.Resize(glbSkeleton.Bones.GetCount());

		const nlohmann::json& jsonChannelArray = jsonAnimation["channels"];
		const int channelCount = static_cast<int>(jsonChannelArray.size());

		const nlohmann::json& jsonSamplerArray = jsonAnimation["samplers"];
		const int samplerCount = static_cast<int>(jsonSamplerArray.size());


		for (int c = 0; c < channelCount; ++c)
		{
			const nlohmann::json& jsonChannel = jsonChannelArray[c];
			const int samplerIndex = jsonChannel["sampler"];
			const int nodeIndex = jsonChannel["target"]["node"];
			const nsName pathType = jsonChannel["target"]["path"].get<std::string>().c_str();

			int boneId = -1;
			ns_GLB_FindBoneWithNodeIndex(glbSkeleton.Bones, nodeIndex, &boneId);

			if (boneId == -1)
			{
				continue;
			}

			nsGLB_Animation::KeyFrame& keyFrame = glbAnimation.KeyFrames[boneId];

			const nlohmann::json& jsonSampler = jsonSamplerArray[samplerIndex];
			const int inputAccessorIndex = jsonSampler["input"]; // timestamp
			const int outputAccessorIndex = jsonSampler["output"]; // position/rotation/scale

			int inputCount = 0;
			int inputComponentType = 0;
			const float* timeStamps = ns_GLB_GetBufferData<float>(inputCount, inputComponentType, binData, jsonData, inputAccessorIndex);
			NS_Validate(inputComponentType == GLB_COMPONENT_TYPE_FLOAT);
			glbAnimation.FrameCount = nsMath::Max(glbAnimation.FrameCount, inputCount);

			int outputCount = 0;
			int outputComponentType = 0;

			if (pathType == "translation")
			{
				const nsVector3* positions = ns_GLB_GetBufferData<nsVector3>(outputCount, outputComponentType, binData, jsonData, outputAccessorIndex);
				NS_Validate(inputCount == outputCount);

				for (int t = 0; t < inputCount; ++t)
				{
					nsGLB_Animation::TChannel<nsVector3>& positionChannel = keyFrame.Positions.Add();
					positionChannel.Value = positions[t] * 100.0f;
					positionChannel.Timestamp = timeStamps[t];
					glbAnimation.Duration = nsMath::Max(glbAnimation.Duration, positionChannel.Timestamp);
				}
			}
			else if (pathType == "rotation")
			{
				const nsQuaternion* rotations = ns_GLB_GetBufferData<nsQuaternion>(outputCount, outputComponentType, binData, jsonData, outputAccessorIndex);
				NS_Validate(inputCount == outputCount);

				for (int t = 0; t < inputCount; ++t)
				{
					nsGLB_Animation::TChannel<nsQuaternion>& rotationChannel = keyFrame.Rotations.Add();
					rotationChannel.Value = rotations[t];
					rotationChannel.Timestamp = timeStamps[t];
					glbAnimation.Duration = nsMath::Max(glbAnimation.Duration, rotationChannel.Timestamp);
				}
			}
			else if (pathType == "scale")
			{
				const nsVector3* scales = ns_GLB_GetBufferData<nsVector3>(outputCount, outputComponentType, binData, jsonData, outputAccessorIndex);
				NS_Validate(inputCount == outputCount);

				for (int t = 0; t < inputCount; ++t)
				{
					nsGLB_Animation::TChannel<nsVector3>& scaleChannel = keyFrame.Scales.Add();
					scaleChannel.Value = scales[t];
					scaleChannel.Timestamp = timeStamps[t];
					glbAnimation.Duration = nsMath::Max(glbAnimation.Duration, scaleChannel.Timestamp);
				}
			}
			else
			{
				NS_ValidateV(0, "Unknown path type [%s]!", *pathType);
			}
		}


		nsAnimationManager& animationManager = nsAnimationManager::Get();
		const nsAnimationClipID clip = animationManager.CreateClip(glbAnimation.Name, glbSkeleton.Name);
		nsAnimationClipData& data = animationManager.GetClipData(clip);
		data.SkeletonName = glbSkeleton.Name;
		data.FrameCount = glbAnimation.FrameCount;
		data.Duration = glbAnimation.Duration;

		const int keyFrameCount = glbAnimation.KeyFrames.GetCount();
		data.KeyFrames.Resize(keyFrameCount);

		for (int k = 0; k < keyFrameCount; ++k)
		{
			const nsGLB_Animation::KeyFrame& glbKeyFrame = glbAnimation.KeyFrames[k];
			nsAnimationKeyFrame& keyFrame = data.KeyFrames[k];

			// Copy position channels
			const int pCount = glbKeyFrame.Positions.GetCount();
			keyFrame.PositionChannels.Resize(pCount);

			for (int p = 0; p < pCount; ++p)
			{
				const nsGLB_Animation::TChannel<nsVector3>& glbPositionChannel = glbKeyFrame.Positions[p];
				nsAnimationKeyFrame::TChannel<nsVector3>& positionChannel = keyFrame.PositionChannels[p];
				positionChannel.Value = glbPositionChannel.Value;
				positionChannel.Timestamp = glbPositionChannel.Timestamp;
			}


			// Copy rotation channels
			const int rCount = glbKeyFrame.Rotations.GetCount();
			keyFrame.RotationChannels.Resize(rCount);

			for (int r = 0; r < rCount; ++r)
			{
				const nsGLB_Animation::TChannel<nsQuaternion>& glbRotationChannel = glbKeyFrame.Rotations[r];
				nsAnimationKeyFrame::TChannel<nsQuaternion>& rotationChannel = keyFrame.RotationChannels[r];
				rotationChannel.Value = glbRotationChannel.Value;
				rotationChannel.Timestamp = glbRotationChannel.Timestamp;
			}


			// Copy scale channels
			const int sCount = glbKeyFrame.Scales.GetCount();
			keyFrame.ScaleChannels.Resize(sCount);

			for (int s = 0; s < sCount; ++s)
			{
				const nsGLB_Animation::TChannel<nsVector3>& glbScaleChannel = glbKeyFrame.Scales[s];
				nsAnimationKeyFrame::TChannel<nsVector3>& scaleChannel = keyFrame.ScaleChannels[s];
				scaleChannel.Value = glbScaleChannel.Value;
				scaleChannel.Timestamp = glbScaleChannel.Timestamp;
			}
		}

		nsAssetManager::Get().SaveAnimationAsset(glbAnimation.Name, clip, dstFolderPath, false);
		NS_CONSOLE_Log(AssetImporterGLB, "Imported animation [%s] from source file [%s]", *glbAnimation.Name, *option.SourceFile);
	}
}


static void ns_GLB_ImportSkeletonAndAnimations(const nsAssetImportOption_Model& option, const nsString& dstFolderPath, const uint8* binData, const nlohmann::json& jsonData, const nsTArray<nsGLB_Node>& glbNodes)
{
	if (!(option.bImportSkeleton || option.bImportAnimation))
	{
		return;
	}

	if (!jsonData.contains("skins"))
	{
		NS_CONSOLE_Error(AssetImporterGLB, "Fail to import skeleton or animation from file [%s]. <skins> not found in json data!", *option.SourceFile);
		return;
	}

	const nlohmann::json& jsonAccessorArray = jsonData["accessors"];
	const nlohmann::json& jsonBufferViewArray = jsonData["bufferViews"];
	const nlohmann::json& jsonSkeletonArray = jsonData["skins"];
	const int skeletonCount = static_cast<int>(jsonSkeletonArray.size());
	const nsMatrix4 scaleMatrix = nsMatrix4::Scale(100.0f);
	NS_Validate(skeletonCount == 1);

	const nlohmann::json& jsonSkeleton = jsonSkeletonArray[0];

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

	if (option.bImportSkeleton)
	{
		nsAssetManager::Get().SaveSkeletonAsset(glbSkeleton.Name, skeleton, dstFolderPath, false);
		NS_CONSOLE_Log(AssetImporterGLB, "Imported skeleton [%s] from source file [%s]", *glbSkeleton.Name, *option.SourceFile);
	}

	if (option.bImportAnimation)
	{
		ns_GLB_ImportAnimations(option, dstFolderPath, binData, jsonData, glbSkeleton, glbNodes);
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
	ns_GLB_ImportSkeletonAndAnimations(option, dstFolderPath, binData, jsonData, glbNodes);
}
