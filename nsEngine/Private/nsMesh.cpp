#include "nsMesh.h"
#include "API_VK/nsVulkanFunctions.h"
#include "nsGeometryFactory.h"


NS_ENGINE_DEFINE_HANDLE(nsMeshID);


static nsLogCategory MeshLog("nsMeshLog", nsELogVerbosity::LV_DEBUG);


nsMeshManager::nsMeshManager() noexcept
	: bInitialized(false)
	, FrameDatas()
	, FrameIndex(0)
{
	MeshFlags.Reserve(64);
	MeshNames.Reserve(64);
	MeshLodGroups.Reserve(64);
	MeshDrawDatas.Reserve(64);
}


void nsMeshManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(MeshLog, "Initialize mesh manager");

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.VertexPositionBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_GPU_ONLY, NS_MEMORY_SIZE_MiB(1), "mesh_vertex_position_buffer");
		frame.VertexAttributeBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_GPU_ONLY, NS_MEMORY_SIZE_MiB(2), "mesh_vertex_attribute_buffer");
		frame.IndexBuffer = nsVulkan::CreateIndexBuffer(VMA_MEMORY_USAGE_GPU_ONLY, NS_MEMORY_SIZE_MiB(2), "mesh_index_buffer");
		frame.StagingBuffer = nsVulkan::CreateStagingBuffer(NS_MEMORY_SIZE_KiB(4), "mesh_staging_buffer");
	}


	// Initialize default meshes
	NS_LogInfo(MeshLog, "Creating default meshes...");
	
	// Floor
	{
		DefaultFloor = CreateMesh("mesh_default_floor");
		nsMeshVertexData& lod0 = MeshLodGroups[DefaultFloor.Id][0];
		nsGeometryFactory::AddMeshBox(lod0.Positions, lod0.Attributes, lod0.Indices, nsVector3(-1600.0f, -8.0f, -1600.0f), nsVector3(1600.0f, 8.0f, 1600.0f), nsVector2(8.0f));
	}

	// Wall
	{
		DefaultWall = CreateMesh("mesh_default_wall");
		nsMeshVertexData& lod0 = MeshLodGroups[DefaultWall.Id][0];
		nsGeometryFactory::AddMeshBox(lod0.Positions, lod0.Attributes, lod0.Indices, nsVector3(-256.0f, -128.0f, -8.0f), nsVector3(256.0f, 128.0f, 8.0f), nsVector2(4.0f));
	}

	// Box
	{
		DefaultBox = CreateMesh("mesh_default_box");
		nsMeshVertexData& lod0 = MeshLodGroups[DefaultBox.Id][0];
		nsGeometryFactory::AddMeshBox(lod0.Positions, lod0.Attributes, lod0.Indices, nsVector3(-50.0f), nsVector3(50.0f), nsVector2(1.0f));
	}

	// Platform
	{
		DefaultPlatform = CreateMesh("mesh_default_platform");
		nsMeshVertexData& lod0 = MeshLodGroups[DefaultPlatform.Id][0];
		nsGeometryFactory::AddMeshBox(lod0.Positions, lod0.Attributes, lod0.Indices, nsVector3(-256.0f, -8.0f, -128.0f), nsVector3(256.0f, 8.0f, 128.0f), nsVector2(4.0f));
	}

	bInitialized = true;
}


nsMeshID nsMeshManager::FindMesh(const nsName& name) const noexcept
{
	for (auto it = MeshNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return nsMeshID(it.GetIndex());
		}
	}

	return nsMeshID::INVALID;
}


nsMeshID nsMeshManager::CreateMesh(nsName name) noexcept
{
	if (FindMesh(name) != nsMeshID::INVALID)
	{
		NS_LogWarning(MeshLog, "Mesh with name [%s] already exists!", *name);
	}

	const int nameId = MeshNames.Add(name);
	const int flagId = MeshFlags.Add(MeshFlag_Dirty);
	const int lodGroupId = MeshLodGroups.Add();
	const int drawDataId = MeshDrawDatas.Add();
	const int boundId = MeshBounds.Add();
	NS_Assert(nameId == flagId && flagId == lodGroupId && lodGroupId == drawDataId && drawDataId == boundId);

	MeshLodGroups[lodGroupId].Clear();
	MeshLodGroups[lodGroupId].Add();

	MeshDrawDatas[drawDataId] = nsMeshDrawData();
	MeshBounds[boundId] = nsMeshBound();

	return nsMeshID(nameId);
}


void nsMeshManager::DestroyMesh(nsMeshID& mesh) noexcept
{
	if (IsMeshValid(mesh))
	{
		NS_LogDebug(MeshLog, "Destroy mesh [%s]", *MeshNames[mesh.Id]);

		const int id = mesh.Id;
		MeshNames.RemoveAt(id);
		MeshFlags.RemoveAt(id);
		MeshLodGroups.RemoveAt(id);
		MeshDrawDatas.RemoveAt(id);
		MeshBounds.RemoveAt(id);
		mesh = nsMeshID::INVALID;
	}
}


void nsMeshManager::UpdateMeshVertexData(nsMeshID mesh, int lodIndex, const nsVertexMeshPosition* vertexPositions, const nsVertexMeshAttribute* vertexAttributes, const nsVertexMeshSkin* vertexSkins, int vertexCount, const uint32* indices, int indexCount) noexcept
{
	NS_Assert(IsMeshValid(mesh));
	nsMeshLODGroup& lodGroup = MeshLodGroups[mesh.Id];
	NS_Assert(lodIndex >= 0 && lodIndex < lodGroup.GetCount());

	nsMeshVertexData& data = lodGroup[lodIndex];

	if (vertexPositions)
	{
		NS_Assert(vertexCount > 0);
		data.Positions.Clear();
		data.Positions.InsertAt(vertexPositions, vertexCount, 0);
	}

	if (vertexAttributes)
	{
		NS_Assert(vertexCount > 0);
		data.Attributes.Clear();
		data.Attributes.InsertAt(vertexAttributes, vertexCount, 0);
	}

	if (vertexSkins)
	{
		NS_Assert(vertexCount > 0);
		data.Skins.Clear();
		data.Skins.InsertAt(vertexSkins, vertexCount, 0);
	}

	if (indices)
	{
		NS_Assert(indexCount > 0);
		data.Indices.Clear();
		data.Indices.InsertAt(indices, indexCount, 0);
	}

	MeshFlags[mesh.Id] |= MeshFlag_Dirty;
}


const nsMeshBound& nsMeshManager::RecomputeMeshBound(nsMeshID mesh) noexcept
{
	NS_Assert(IsMeshValid(mesh));

	nsAABB aabb(FLT_MAX, -FLT_MAX);
	const nsMeshVertexData& data = MeshLodGroups[mesh.Id][0];

	for (int v = 0; v < data.Positions.GetCount(); ++v)
	{
		const nsVertexMeshPosition& vertex = data.Positions[v];
		aabb.Min = nsMath::MinVector3(aabb.Min, vertex);
		aabb.Max = nsMath::MaxVector3(aabb.Max, vertex);
	}

	nsMeshBound& bound = MeshBounds[mesh.Id];
	bound.SphereRadius = (aabb.Max - aabb.Min).GetMagnitude() / 2.0f;

	return bound;
}


void nsMeshManager::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];
	frame.MeshBindingInfos.Clear();
	frame.MeshToDestroys.Clear();
}


void nsMeshManager::BindMeshes(const nsMeshBindingInfo* bindingInfos, int count) noexcept
{
	if (bindingInfos == nullptr || count <= 0)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];

	for (int i = 0; i < count; ++i)
	{
		const nsMeshBindingInfo& info = bindingInfos[i];
		NS_Assert(IsMeshValid(info.Mesh));
		
		uint32& flags = MeshFlags[info.Mesh.Id];
		NS_AssertV(!(flags & MeshFlag_PendingDestroy), "Cannot bind mesh that has marked pending destroy!");

		if (flags & MeshFlag_Dirty)
		{
			flags |= MeshFlag_PendingLoad;
		}

		frame.MeshBindingInfos.Add(info);
	}
}


void nsMeshManager::Update() noexcept
{
	Frame& frame = FrameDatas[FrameIndex];

	if (frame.MeshBindingInfos.GetCount() == 0)
	{
		return;
	}

	uint64 vertexPositionBufferSize = 0;
	uint64 vertexAttributeBufferSize = 0;
	uint64 vertexIndexBufferSize = 0;
	int baseVertex = 0;
	int baseIndex = 0;
	int indexVertexOffset = 0;

	for (int i = 0; i < frame.MeshBindingInfos.GetCount(); ++i)
	{
		const nsMeshBindingInfo& bindingInfo = frame.MeshBindingInfos[i];
		const int id = bindingInfo.Mesh.Id;
		const nsMeshLODGroup& lodGroup = MeshLodGroups[id];

		const int lodCount = lodGroup.GetCount();
		const int maxLodLevel = lodCount > 0 ? (lodCount - 1) : 0;
		const int selectedLod = nsMath::Clamp(bindingInfo.Lod, 0, maxLodLevel);
		const nsMeshVertexData& vertexData = lodGroup[selectedLod];
		const int vertexCount = vertexData.Positions.GetCount();
		NS_Assert(vertexData.Attributes.GetCount() == vertexCount);

		vertexPositionBufferSize += sizeof(nsVertexMeshPosition) * vertexCount;
		vertexAttributeBufferSize += sizeof(nsVertexMeshAttribute) * vertexCount;

		const int indexCount = vertexData.Indices.GetCount();
		vertexIndexBufferSize += sizeof(uint32) * indexCount;

		nsMeshDrawData& drawData = MeshDrawDatas[id];
		drawData.BaseVertex = baseVertex;
		drawData.VertexCount = vertexCount;
		drawData.BaseIndex = baseIndex;
		drawData.IndexCount = indexCount;
		drawData.IndexVertexOffset = indexVertexOffset;

		baseVertex += vertexCount;
		baseIndex += indexCount;
		indexVertexOffset += vertexCount;
	}

	frame.VertexPositionBuffer->Resize(vertexPositionBufferSize);
	frame.VertexAttributeBuffer->Resize(vertexAttributeBufferSize);
	frame.IndexBuffer->Resize(vertexIndexBufferSize);

	const uint64 stagingBufferSize = vertexPositionBufferSize + vertexAttributeBufferSize + vertexIndexBufferSize;
	frame.StagingBuffer->Resize(stagingBufferSize);

	uint8* stagingMap = reinterpret_cast<uint8*>(frame.StagingBuffer->MapMemory());
	uint64 stagingOffset = 0;

	// Vertex positions copy to staging buffer
	for (int i = 0; i < frame.MeshBindingInfos.GetCount(); ++i)
	{
		const nsMeshBindingInfo& bindingInfo = frame.MeshBindingInfos[i];
		const int id = bindingInfo.Mesh.Id;
		const nsMeshLODGroup& lodGroup = MeshLodGroups[id];
		const int lodCount = lodGroup.GetCount();
		const int maxLodLevel = lodCount > 0 ? (lodCount - 1) : 0;
		const int selectedLod = nsMath::Clamp(bindingInfo.Lod, 0, maxLodLevel);
		const nsMeshVertexData& vertexData = lodGroup[selectedLod];
		const uint64 vertexPositionSize = sizeof(nsVertexMeshPosition) * vertexData.Positions.GetCount();
		nsPlatform::Memory_Copy(stagingMap + stagingOffset, vertexData.Positions.GetData(), vertexPositionSize);
		stagingOffset += vertexPositionSize;
	}

	NS_Assert(stagingOffset == vertexPositionBufferSize);


	// Vertex attributes copy to staging buffer
	for (int i = 0; i < frame.MeshBindingInfos.GetCount(); ++i)
	{
		const nsMeshBindingInfo& bindingInfo = frame.MeshBindingInfos[i];
		const int id = bindingInfo.Mesh.Id;
		const nsMeshLODGroup& lodGroup = MeshLodGroups[id];
		const int lodCount = lodGroup.GetCount();
		const int maxLodLevel = lodCount > 0 ? (lodCount - 1) : 0;
		const int selectedLod = nsMath::Clamp(bindingInfo.Lod, 0, maxLodLevel);
		const nsMeshVertexData& vertexData = lodGroup[selectedLod];
		const uint64 vertexAttributeSize = sizeof(nsVertexMeshAttribute) * vertexData.Attributes.GetCount();
		nsPlatform::Memory_Copy(stagingMap + stagingOffset, vertexData.Attributes.GetData(), vertexAttributeSize);
		stagingOffset += vertexAttributeSize;
	}

	NS_Assert(stagingOffset == (vertexPositionBufferSize + vertexAttributeBufferSize));


	// Vertex indices copy to staging buffer
	for (int i = 0; i < frame.MeshBindingInfos.GetCount(); ++i)
	{
		const nsMeshBindingInfo& bindingInfo = frame.MeshBindingInfos[i];
		const int id = bindingInfo.Mesh.Id;
		const nsMeshLODGroup& lodGroup = MeshLodGroups[id];
		const int lodCount = lodGroup.GetCount();
		const int maxLodLevel = lodCount > 0 ? (lodCount - 1) : 0;
		const int selectedLod = nsMath::Clamp(bindingInfo.Lod, 0, maxLodLevel);
		const nsMeshVertexData& vertexData = lodGroup[selectedLod];
		const uint64 vertexIndexSize = sizeof(uint32) * vertexData.Indices.GetCount();
		nsPlatform::Memory_Copy(stagingMap + stagingOffset, vertexData.Indices.GetData(), vertexIndexSize);
		stagingOffset += vertexIndexSize;
	}

	NS_Assert(stagingOffset == (vertexPositionBufferSize + vertexAttributeBufferSize + vertexIndexBufferSize));

	frame.StagingBuffer->UnmapMemory();


	VkCommandBuffer transferCommandBuffer = nsVulkan::AllocateTransferCommandBuffer();
	NS_VK_BeginCommand(transferCommandBuffer);
	{
		VkBufferCopy copyRegion{};

		// Vertex position copy
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = vertexPositionBufferSize;
		vkCmdCopyBuffer(transferCommandBuffer, frame.StagingBuffer->GetVkBuffer(), frame.VertexPositionBuffer->GetVkBuffer(), 1, &copyRegion);

		// Vertex attribute copy
		copyRegion.srcOffset += copyRegion.size;
		copyRegion.dstOffset = 0;
		copyRegion.size = vertexAttributeBufferSize;
		vkCmdCopyBuffer(transferCommandBuffer, frame.StagingBuffer->GetVkBuffer(), frame.VertexAttributeBuffer->GetVkBuffer(), 1, &copyRegion);

		// Vertex index copy
		copyRegion.srcOffset += copyRegion.size;
		copyRegion.dstOffset = 0;
		copyRegion.size = vertexIndexBufferSize;
		vkCmdCopyBuffer(transferCommandBuffer, frame.StagingBuffer->GetVkBuffer(), frame.IndexBuffer->GetVkBuffer(), 1, &copyRegion);
	}
	NS_VK_EndCommand(transferCommandBuffer);

	nsVulkan::SubmitTransferCommandBuffer(&transferCommandBuffer, 1);
}
