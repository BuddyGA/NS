#pragma once

#include "nsVulkan.h"



struct nsMeshVertexData
{
	nsTArray<nsVertexMeshPosition> Positions;
	nsTArray<nsVertexMeshAttribute> Attributes;
	nsTArray<nsVertexMeshSkin> Skins;
	nsTArray<uint32> Indices;


	friend NS_INLINE void operator|(nsStream& stream, nsMeshVertexData& vertexData) noexcept
	{
		stream | vertexData.Positions;
		stream | vertexData.Attributes;
		stream | vertexData.Skins;
		stream | vertexData.Indices;
	}

};

typedef nsTArrayInline<nsMeshVertexData, NS_ENGINE_MESH_MAX_LOD> nsMeshLODGroup;



struct nsMeshDrawData
{
	int BaseVertex;
	int VertexCount;
	int BaseIndex;
	int IndexCount;
	int IndexVertexOffset;


public:
	nsMeshDrawData() noexcept
		: BaseVertex(0)
		, VertexCount(0)
		, BaseIndex(0)
		, IndexCount(0)
		, IndexVertexOffset(0)
	{
	}

};



struct nsMeshBound
{
	float SphereRadius;


public:
	nsMeshBound() noexcept
		: SphereRadius(0.0f)
	{
	}

};



struct nsMeshBindingInfo
{
	nsMeshID Mesh;
	int Lod;
	bool bIsSkinned;

public:
	nsMeshBindingInfo() noexcept
		: Mesh(nsMeshID::INVALID)
		, Lod(0)
		, bIsSkinned(false)
	{
	}

};




class NS_ENGINE_API nsMeshManager
{
	NS_DECLARE_SINGLETON(nsMeshManager)

private:
	struct Frame
	{
		nsVulkanBuffer* VertexPositionBuffer;
		nsVulkanBuffer* VertexAttributeBuffer;
		nsVulkanBuffer* VertexSkinBuffer;
		nsVulkanBuffer* IndexBuffer;
		nsVulkanBuffer* StagingBuffer;
		nsTArray<nsMeshBindingInfo> MeshBindingInfos;
		nsTArray<nsMeshID> MeshToDestroys;
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;
	bool bInitialized;


	enum Flag
	{
		MeshFlag_None				= (0),
		MeshFlag_Dirty				= (1 << 0),
		MeshFlag_PendingLoad		= (1 << 1),
		MeshFlag_Loaded				= (1 << 2),
		MeshFlag_AlwaysLoaded		= (1 << 3),
		MeshFlag_PendingDestroy		= (1 << 4),
	};

	nsTArrayFreeList<nsName> MeshNames;
	nsTArrayFreeList<uint32> MeshFlags;
	nsTArrayFreeList<nsMeshLODGroup> MeshLodGroups;
	nsTArrayFreeList<nsMeshDrawData> MeshDrawDatas;
	nsTArrayFreeList<nsMeshBound> MeshBounds;

	nsMeshID DefaultFloor;
	nsMeshID DefaultWall;
	nsMeshID DefaultBox;
	nsMeshID DefaultPlatform;
	nsMeshID DefaultSphere;


public:
	void Initialize() noexcept;

	NS_NODISCARD nsMeshID FindMesh(const nsName& name) const noexcept;
	NS_NODISCARD nsMeshID CreateMesh(nsName name) noexcept;
	void DestroyMesh(nsMeshID& mesh) noexcept;
	void UpdateMeshVertexData(nsMeshID mesh, int lodIndex, const nsVertexMeshPosition* vertexPositions, const nsVertexMeshAttribute* vertexAttributes, const nsVertexMeshSkin* vertexSkins, int vertexCount, const uint32* indices, int indexCount) noexcept;
	const nsMeshBound& RecomputeMeshBound(nsMeshID mesh) noexcept;

	void BeginFrame(int frameIndex) noexcept;
	void BindMeshes(const nsMeshBindingInfo* bindingInfos, int count) noexcept;
	void UpdateRenderResources() noexcept;


	// Get current frame vertex position buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetVertexPositionBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].VertexPositionBuffer;
	}


	// Get current frame vertex attribute buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetVertexAttributeBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].VertexAttributeBuffer;
	}


	// Get current frame vertex skin buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetVertexSkinBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].VertexSkinBuffer;
	}


	// Get current frame index buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetIndexBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].IndexBuffer;
	}


	NS_NODISCARD_INLINE bool IsMeshValid(nsMeshID mesh) const noexcept
	{
		return mesh.IsValid() && MeshFlags.IsValid(mesh.Id);
	}


	NS_INLINE int AddMeshLod(nsMeshID mesh) noexcept
	{
		NS_Assert(IsMeshValid(mesh));

		const int lod = MeshLodGroups[mesh.Id].GetCount();
		MeshLodGroups[mesh.Id].Add();

		return lod;
	}


	NS_NODISCARD_INLINE nsName GetMeshName(nsMeshID mesh) const noexcept
	{
		NS_Assert(IsMeshValid(mesh));

		return MeshNames[mesh.Id];
	}


	NS_NODISCARD_INLINE nsMeshLODGroup& GetMeshLodGroup(nsMeshID mesh) noexcept
	{
		NS_Assert(IsMeshValid(mesh));

		return MeshLodGroups[mesh.Id];
	}


	NS_NODISCARD_INLINE const nsMeshLODGroup& GetMeshLodGroup(nsMeshID mesh) const noexcept
	{
		NS_Assert(IsMeshValid(mesh));

		return MeshLodGroups[mesh.Id];
	}


	NS_NODISCARD_INLINE nsMeshVertexData& GetMeshVertexData(nsMeshID mesh, int lodIndex) noexcept
	{
		NS_Assert(IsMeshValid(mesh));
		nsMeshLODGroup& lodGroup = MeshLodGroups[mesh.Id];
		NS_Assert(lodIndex >= 0 && lodIndex < lodGroup.GetCount());

		return lodGroup[lodIndex];
	}


	NS_NODISCARD_INLINE const nsMeshVertexData& GetMeshVertexData(nsMeshID mesh, int lodIndex) const noexcept
	{
		NS_Assert(IsMeshValid(mesh));
		const nsMeshLODGroup& lodGroup = MeshLodGroups[mesh.Id];
		NS_Assert(lodIndex >= 0 && lodIndex < lodGroup.GetCount());

		return lodGroup[lodIndex];
	}


	NS_NODISCARD_INLINE const nsMeshDrawData& GetMeshDrawData(nsMeshID mesh) const noexcept
	{
		NS_Assert(IsMeshValid(mesh));

		return MeshDrawDatas[mesh.Id];
	}


	NS_NODISCARD_INLINE nsMeshID GetDefaultMesh_Floor() const noexcept
	{
		return DefaultFloor;
	}


	NS_NODISCARD_INLINE nsMeshID GetDefaultMesh_Wall() const noexcept
	{
		return DefaultWall;
	}

	
	NS_NODISCARD_INLINE nsMeshID GetDefaultMesh_Box() const noexcept
	{
		return DefaultBox;
	}


	NS_NODISCARD_INLINE nsMeshID GetDefaultMesh_Platform() const noexcept
	{
		return DefaultPlatform;
	}

};
