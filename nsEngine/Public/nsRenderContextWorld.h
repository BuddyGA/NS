#pragma once

#include "nsMesh.h"



struct nsRenderCameraView
{
	nsMatrix4 View;
	nsMatrix4 Projection;
	nsVector3 WorldPosition;
};



struct nsRenderEnvironment
{
	nsMatrix4 DirectionalLight_ViewProjection;
	nsVector4 DirectionalLight_Direction;
	nsVector4 DirectionalLight_ColorIntensity;
	nsVector4 AmbientLight_ColorStrength;
};



struct nsRenderMesh
{
	nsMatrix4 WorldTransform;
	nsMaterialID Material;
	nsMeshID Mesh;
};



struct nsRenderDrawCallPerMesh
{
	nsMeshID Mesh;
	nsTArray<nsMatrix4> WorldTransforms;


public:
	nsRenderDrawCallPerMesh(nsMeshID mesh = nsMeshID::INVALID) noexcept
		: Mesh(mesh)
	{
	}

	NS_INLINE bool operator==(const nsRenderDrawCallPerMesh& rhs) const noexcept
	{
		return Mesh == rhs.Mesh;
	}

};



struct nsRenderDrawCallPerMaterial
{
	nsMaterialID Material;
	nsTArray<nsRenderDrawCallPerMesh> Meshes;


public:
	nsRenderDrawCallPerMaterial(nsMaterialID material = nsMaterialID::INVALID) noexcept
		: Material(material)
	{
	}

	NS_INLINE bool operator==(const nsRenderDrawCallPerMaterial& rhs) const noexcept
	{
		return Material == rhs.Material;
	}

};



struct nsRenderDrawCallPrimitiveBatch
{
	uint64 VertexBufferOffset;
	uint64 IndexBufferOffset;
	uint32 VertexCount;
	uint32 IndexCount;


public:
	nsRenderDrawCallPrimitiveBatch() noexcept
		: VertexBufferOffset(0)
		, IndexBufferOffset(0)
		, VertexCount(0)
		, IndexCount(0)
	{
	}


	NS_INLINE void Reset() noexcept
	{
		VertexBufferOffset = 0;
		IndexBufferOffset = 0;
		VertexCount = 0;
		IndexCount = 0;
	}

};




class NS_ENGINE_API nsRenderContextWorld
{
private:
	struct Frame
	{
		nsVulkanBuffer* EnvironmentUniformBuffer;
		nsVulkanBuffer* PrimitiveVertexBuffer;
		nsVulkanBuffer* PrimitiveIndexBuffer;
		nsVulkanBuffer* PrimitiveStagingBuffer;
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;

	nsRenderEnvironment RenderEnvironment;
	nsTArrayFreeList<nsRenderMesh> RenderMeshes;
	
	nsTArray<nsVertexPrimitive> PrimitiveBatchLineVertices;
	nsTArray<uint32> PrimitiveBatchLineIndices;

	nsTArray<nsVertexPrimitive> PrimitiveBatchMeshVertices;
	nsTArray<uint32> PrimitiveBatchMeshIndices;

	nsTArray<nsMaterialID> DrawBindMaterials;
	nsTArray<nsMeshBindingInfo> DrawBindMeshes;
	nsTArray<nsRenderDrawCallPerMaterial> DrawCallMeshes;
	nsRenderDrawCallPrimitiveBatch DrawCallPrimitiveBatchMesh;
	nsRenderDrawCallPrimitiveBatch DrawCallPrimitiveBatchLine;


public:
	nsRenderContextWorld() noexcept;
	void AddPrimitiveMesh_Plane(const nsVector3& position, const nsVector3& normal, float width, const nsColor& color) noexcept;
	void AddPrimitiveMesh_AABB(const nsVector3& boxMin, const nsVector3& boxMax, const nsColor& color) noexcept;
	void AddPrimitiveMesh_Prism(const nsVector3& position, const nsQuaternion& rotation, float width, float height, const nsColor& color) noexcept;
	void AddPrimitiveMesh_Arrow(const nsVector3& position, const nsQuaternion& rotation, float shaftLength, float width, float tipHeight, const nsColor& shaftColor, const nsColor& arrowColor) noexcept;
	void AddPrimitiveLine(const nsVector3& start, const nsVector3& end, const nsColor& color) noexcept;
	void AddPrimitiveLine_Circle(const nsVector3& center, float radius, float halfArcRadian, nsEAxisType arcAxis, const nsColor& color) noexcept;
	void AddPrimitiveLine_CircleAroundAxis(const nsVector3& center, const nsVector3& axis, float radius, float halfArcRadian, const nsColor& color) noexcept;
	void UpdateResourcesAndBuildDrawCalls(int frameIndex) noexcept;


	NS_NODISCARD_INLINE bool IsRenderMeshValid(nsRenderContextMeshID renderMesh) const noexcept
	{
		return renderMesh.IsValid() && RenderMeshes.IsValid(renderMesh.Id);
	}


	NS_NODISCARD_INLINE nsRenderContextMeshID AddRenderMesh(nsMeshID mesh, nsMaterialID material, const nsMatrix4& transform) noexcept
	{
		NS_Assert(mesh != nsMeshID::INVALID);
		NS_Assert(material != nsMaterialID::INVALID);

		nsRenderMesh value{};
		value.WorldTransform = transform;
		value.Material = material;
		value.Mesh = mesh;

		return RenderMeshes.Add(value);
	}

	NS_INLINE void UpdateRenderMesh(nsRenderContextMeshID id, nsMeshID newMesh, nsMaterialID newMaterial, const nsMatrix4& newTransform) noexcept
	{
		NS_Assert(IsRenderMeshValid(id));
		NS_Assert(newMesh != nsMeshID::INVALID);
		NS_Assert(newMaterial != nsMaterialID::INVALID);

		nsRenderMesh& value = RenderMeshes[id.Id];
		value.WorldTransform = newTransform;
		value.Material = newMaterial;
		value.Mesh = newMesh;
	}

	NS_INLINE void RemoveRenderMesh(nsRenderContextMeshID& id) noexcept
	{
		NS_Assert(IsRenderMeshValid(id));

		RenderMeshes.RemoveAt(id.Id);
		id = nsRenderContextMeshID::INVALID;
	}


	// Get current frame environment uniform buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetEnvironmentUniformBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].EnvironmentUniformBuffer;
	}


	// Get current frame primitive vertex buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetPrimitiveVertexBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].PrimitiveVertexBuffer;
	}


	// Get current frame primitive index buffer
	NS_NODISCARD_INLINE const nsVulkanBuffer* GetPrimitiveIndexBuffer() const noexcept
	{
		return FrameDatas[FrameIndex].PrimitiveIndexBuffer;
	}


	// Get draw call data (meshes)
	NS_NODISCARD_INLINE const nsTArray<nsRenderDrawCallPerMaterial>& GetDrawCallMeshes() const noexcept
	{
		return DrawCallMeshes;
	}


	// Get draw call data (primitive batch mesh)
	NS_NODISCARD_INLINE const nsRenderDrawCallPrimitiveBatch& GetDrawCallPrimitiveBatchMesh() const noexcept
	{
		return DrawCallPrimitiveBatchMesh;
	}


	// Get draw call data (primitive batch line)
	NS_NODISCARD_INLINE const nsRenderDrawCallPrimitiveBatch& GetDrawCallPrimitiveBatchLine() const noexcept
	{
		return DrawCallPrimitiveBatchLine;
	}

};
