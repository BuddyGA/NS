#include "nsRenderContextWorld.h"
#include "nsGeometryFactory.h"
#include "nsMaterial.h"
#include "nsAnimationManager.h"
#include "API_VK/nsVulkanFunctions.h"



NS_ENGINE_DEFINE_HANDLE(nsRenderMeshID);
NS_ENGINE_DEFINE_HANDLE(nsRenderPointLightID);



nsRenderContextWorld::nsRenderContextWorld() noexcept
	: FrameDatas()
	, FrameIndex(0)
{
	RenderEnvironment.DirectionalLight_ViewProjection = nsMatrix4::IDENTITY;
	RenderEnvironment.DirectionalLight_Direction = nsQuaternion::RotateVector(nsQuaternion::FromRotation(30.0f, 0.0f, 0.0f), nsVector3(0.0f, 0.0f, 1.0f));
	RenderEnvironment.DirectionalLight_ColorIntensity = nsVector4(1.0f, 1.0f, 1.0f, 1.0f);
	RenderEnvironment.AmbientLight_ColorStrength = nsVector4(1.0f, 1.0f, 1.0f, 0.25f);

	RenderMeshes.Reserve(64);

	PrimitiveBatchMeshVertices.Reserve(1024);
	PrimitiveBatchMeshIndices.Reserve(4096);

	PrimitiveBatchLineVertices.Reserve(1024);
	PrimitiveBatchLineIndices.Reserve(2048);

	DrawBindMaterials.Reserve(64);
	DrawBindMeshes.Reserve(64);
	DrawCallMeshes.Reserve(64);
}


void nsRenderContextWorld::AddPrimitiveMesh_Plane(const nsVector3& position, const nsVector3& normal, float width, const nsColor& color) noexcept
{
	const int baseVertex = PrimitiveBatchMeshVertices.GetCount();
	nsGeometryFactory::AddPrimitiveTriangle_Plane(PrimitiveBatchMeshVertices, PrimitiveBatchMeshIndices, normal, width, color);

	for (int i = baseVertex; i < PrimitiveBatchMeshVertices.GetCount(); ++i)
	{
		const nsVector3 vtxPosition = PrimitiveBatchMeshVertices[i].Position.ToVector3();
		PrimitiveBatchMeshVertices[i].Position = nsVector4(vtxPosition + position);
	}
}


void nsRenderContextWorld::AddPrimitiveMesh_AABB(const nsVector3& boxMin, const nsVector3& boxMax, const nsColor& color) noexcept
{
	nsGeometryFactory::AddPrimitiveTriangle_AABB(PrimitiveBatchMeshVertices, PrimitiveBatchMeshIndices, boxMin, boxMax, color);
}


void nsRenderContextWorld::AddPrimitiveMesh_Prism(const nsVector3& position, const nsQuaternion& rotation, float width, float height, const nsColor& color) noexcept
{
	const uint32 baseVertex = PrimitiveBatchMeshVertices.GetCount();
	nsGeometryFactory::AddPrimitiveTriangle_Prism(PrimitiveBatchMeshVertices, PrimitiveBatchMeshIndices, width, height, color);

	const nsMatrix4 worldMatrix = nsMatrix4::RotationQuat(rotation) * nsMatrix4::Translation(position);

	for (int i = baseVertex; i < PrimitiveBatchMeshVertices.GetCount(); ++i)
	{
		const nsVector4 vtxPosition = PrimitiveBatchMeshVertices[i].Position;
		PrimitiveBatchMeshVertices[i].Position = vtxPosition * worldMatrix;
	}
}


void nsRenderContextWorld::AddPrimitiveMesh_Arrow(const nsVector3& position, const nsQuaternion& rotation, float shaftLength, float width, float tipHeight, const nsColor& shaftColor, const nsColor& arrowColor) noexcept
{
	const uint32 baseVertex = PrimitiveBatchMeshVertices.GetCount();
	nsGeometryFactory::AddPrimitiveTriangle_Arrow(PrimitiveBatchMeshVertices, PrimitiveBatchMeshIndices, shaftLength, width, tipHeight, shaftColor, arrowColor);

	const nsMatrix4 worldMatrix = nsMatrix4::RotationQuat(rotation) * nsMatrix4::Translation(position);

	for (int i = baseVertex; i < PrimitiveBatchMeshVertices.GetCount(); ++i)
	{
		const nsVector4 vtxPosition = PrimitiveBatchMeshVertices[i].Position;
		PrimitiveBatchMeshVertices[i].Position = vtxPosition * worldMatrix;
	}
}


void nsRenderContextWorld::AddPrimitiveLine(const nsVector3& start, const nsVector3& end, const nsColor& color) noexcept
{
	nsGeometryFactory::AddPrimitiveLine(PrimitiveBatchLineVertices, PrimitiveBatchLineIndices, start, end, color);
}


void nsRenderContextWorld::AddPrimitiveLine_Circle(const nsVector3& center, float radius, float halfArcRadian, nsEAxisType arcAxis, const nsColor& color) noexcept
{
	nsGeometryFactory::AddPrimitiveLine_Circle(PrimitiveBatchLineVertices, PrimitiveBatchLineIndices, center, radius, halfArcRadian, arcAxis, color);
}


void nsRenderContextWorld::AddPrimitiveLine_CircleAroundAxis(const nsVector3& center, const nsVector3& axis, float radius, float halfArcRadian, const nsColor& color) noexcept
{
	const uint32 baseVertex = PrimitiveBatchLineVertices.GetCount();
	nsGeometryFactory::AddPrimitiveLine_Circle(PrimitiveBatchLineVertices, PrimitiveBatchLineIndices, nsVector3::ZERO, radius, halfArcRadian, nsEAxisType::Y_Axis, color);

	const nsMatrix4 worldMatrix = nsMatrix4::RotationQuat(nsQuaternion::FromVectors(nsVector3::UP, axis.GetNormalized())) * nsMatrix4::Translation(center);

	for (int i = baseVertex; i < PrimitiveBatchLineVertices.GetCount(); ++i)
	{
		const nsVector4 vtxPosition = PrimitiveBatchLineVertices[i].Position;
		PrimitiveBatchLineVertices[i].Position = vtxPosition * worldMatrix;
	}
}


void nsRenderContextWorld::UpdateResourcesAndBuildDrawCalls(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	DrawBindMaterials.Clear();
	DrawBindMeshes.Clear();
	//DrawBindAnimationInstances.Clear();
	DrawCallMeshes.Clear();
	DrawCallPrimitiveBatchMesh.Reset();
	DrawCallPrimitiveBatchLine.Reset();


	Frame& frame = FrameDatas[FrameIndex];

	if (frame.EnvironmentUniformBuffer == nullptr)
	{
		frame.EnvironmentUniformBuffer = nsVulkan::CreateUniformBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, sizeof(nsRenderEnvironmentData), "environment_uniform_buffer");
	}

	void* environmentMap = frame.EnvironmentUniformBuffer->MapMemory();
	nsPlatform::Memory_Copy(environmentMap, &RenderEnvironment, sizeof(nsRenderEnvironmentData));
	frame.EnvironmentUniformBuffer->UnmapMemory();
	

	const int meshCount = RenderMeshes.GetCount();

	for (auto it = RenderMeshes.CreateConstIterator(); it; ++it)
	{
		const nsMaterialID material = it->Material;
		int perMaterialIndex = 0;

		if (DrawCallMeshes.AddUnique(it->Material, &perMaterialIndex))
		{
			DrawBindMaterials.Add(material);
		}

		nsRenderDrawCallPerMaterial& perMaterial = DrawCallMeshes[perMaterialIndex];

		const nsMeshID mesh = it->Mesh;
		int perMeshIndex = 0;

		if (perMaterial.Meshes.AddUnique(it->Mesh, &perMeshIndex))
		{
			nsMeshBindingInfo& bindingInfo = DrawBindMeshes.Add();
			bindingInfo.Mesh = mesh;
			bindingInfo.Lod = 0;
			bindingInfo.bIsSkinned = it->AnimationInstance != nsAnimationInstanceID::INVALID;
		}

		nsRenderDrawCallPerMesh& perMesh = perMaterial.Meshes[perMeshIndex];
		nsRenderDrawCallPerInstance& instance = perMesh.Instances.Add();
		instance.WorldTransform = it->WorldTransform;
		instance.BoneTransformIndex = -1;

		if (it->AnimationInstance != nsAnimationInstanceID::INVALID)
		{
			//DrawBindAnimationInstances.AddUnique(it->AnimationInstance);
			instance.BoneTransformIndex = nsAnimationManager::Get().GetInstanceBoneTransformIndex(it->AnimationInstance);
		}
	}

	nsMaterialManager::Get().BindMaterials(DrawBindMaterials.GetData(), DrawBindMaterials.GetCount());
	nsMeshManager::Get().BindMeshes(DrawBindMeshes.GetData(), DrawBindMeshes.GetCount());


	const uint64 primitiveMeshVertexSize = sizeof(nsVertexPrimitive) * PrimitiveBatchMeshVertices.GetCount();
	const uint64 primitiveMeshIndexSize = sizeof(uint32) * PrimitiveBatchMeshIndices.GetCount();

	const uint64 primitiveLineVertexSize = sizeof(nsVertexPrimitive) * PrimitiveBatchLineVertices.GetCount();
	const uint64 primitiveLineIndexSize = sizeof(uint32) * PrimitiveBatchLineIndices.GetCount();

	const uint64 stagingBufferSize = primitiveMeshVertexSize + primitiveMeshIndexSize + primitiveLineVertexSize + primitiveLineIndexSize;

	if (stagingBufferSize == 0)
	{
		return;
	}

	if (frame.PrimitiveVertexBuffer == nullptr)
	{
		frame.PrimitiveVertexBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_GPU_ONLY, primitiveMeshVertexSize + primitiveLineVertexSize, "primitive_vertex_buffer");
	}
	else
	{
		frame.PrimitiveVertexBuffer->Resize(primitiveMeshVertexSize + primitiveLineVertexSize);
	}

	if (frame.PrimitiveIndexBuffer == nullptr)
	{
		frame.PrimitiveIndexBuffer = nsVulkan::CreateIndexBuffer(VMA_MEMORY_USAGE_GPU_ONLY, primitiveMeshIndexSize + primitiveLineIndexSize, "primitive_index_buffer");
	}
	else
	{
		frame.PrimitiveIndexBuffer->Resize(primitiveMeshIndexSize + primitiveLineIndexSize);
	}

	if (frame.PrimitiveStagingBuffer == nullptr)
	{
		frame.PrimitiveStagingBuffer = nsVulkan::CreateStagingBuffer(stagingBufferSize, "primitive_staging_buffer");
	}
	else
	{
		frame.PrimitiveStagingBuffer->Resize(stagingBufferSize);
	}

	nsTArrayInline<VkBufferCopy, 2> vertexCopyRegions;
	nsTArrayInline<VkBufferCopy, 2> indexCopyRegions;

	uint8* stagingMap = reinterpret_cast<uint8*>(frame.PrimitiveStagingBuffer->MapMemory());
	{
		uint64 srcOffset = 0;
		uint64 vertexDstOffset = 0;
		uint64 indexDstOffset = 0;

		if (primitiveMeshVertexSize > 0)
		{
			NS_Assert(primitiveMeshIndexSize > 0);

			VkBufferCopy& vertexCopy = vertexCopyRegions.Add();
			vertexCopy.srcOffset = srcOffset;
			vertexCopy.dstOffset = vertexDstOffset;
			vertexCopy.size = primitiveMeshVertexSize;

			DrawCallPrimitiveBatchMesh.VertexBufferOffset = vertexDstOffset;
			DrawCallPrimitiveBatchMesh.VertexCount = static_cast<uint32>(PrimitiveBatchMeshVertices.GetCount());

			nsPlatform::Memory_Copy(stagingMap, PrimitiveBatchMeshVertices.GetData(), primitiveMeshVertexSize);
			srcOffset += primitiveMeshVertexSize;
			vertexDstOffset += primitiveMeshVertexSize;

			VkBufferCopy& indexCopy = indexCopyRegions.Add();
			indexCopy.srcOffset = srcOffset;
			indexCopy.dstOffset = 0;
			indexCopy.size = primitiveMeshIndexSize;

			DrawCallPrimitiveBatchMesh.IndexBufferOffset = indexDstOffset;
			DrawCallPrimitiveBatchMesh.IndexCount = static_cast<uint32>(PrimitiveBatchMeshIndices.GetCount());

			nsPlatform::Memory_Copy(stagingMap + srcOffset, PrimitiveBatchMeshIndices.GetData(), primitiveMeshIndexSize);
			srcOffset += primitiveMeshIndexSize;
			indexDstOffset += primitiveMeshIndexSize;
		}

		if (primitiveLineVertexSize > 0)
		{
			NS_Assert(primitiveLineIndexSize > 0);

			VkBufferCopy& vertexCopy = vertexCopyRegions.Add();
			vertexCopy.srcOffset = srcOffset;
			vertexCopy.dstOffset = vertexDstOffset;
			vertexCopy.size = primitiveLineVertexSize;

			DrawCallPrimitiveBatchLine.VertexBufferOffset = vertexDstOffset;
			DrawCallPrimitiveBatchLine.VertexCount = static_cast<uint32>(PrimitiveBatchLineVertices.GetCount());

			nsPlatform::Memory_Copy(stagingMap + srcOffset, PrimitiveBatchLineVertices.GetData(), primitiveLineVertexSize);
			srcOffset += primitiveLineVertexSize;
			vertexDstOffset += primitiveLineVertexSize;

			VkBufferCopy& indexCopy = indexCopyRegions.Add();
			indexCopy.srcOffset = srcOffset;
			indexCopy.dstOffset = indexDstOffset;
			indexCopy.size = primitiveLineIndexSize;

			DrawCallPrimitiveBatchLine.IndexBufferOffset = indexDstOffset;
			DrawCallPrimitiveBatchLine.IndexCount = static_cast<uint32>(PrimitiveBatchLineIndices.GetCount());

			nsPlatform::Memory_Copy(stagingMap + srcOffset, PrimitiveBatchLineIndices.GetData(), primitiveLineIndexSize);
			srcOffset += primitiveLineIndexSize;
			indexDstOffset += primitiveLineIndexSize;
		}

		NS_Assert(srcOffset == stagingBufferSize);
		NS_Assert(vertexDstOffset == (primitiveMeshVertexSize + primitiveLineVertexSize));
		NS_Assert(indexDstOffset == (primitiveMeshIndexSize + primitiveLineIndexSize));
	}
	frame.PrimitiveStagingBuffer->UnmapMemory();


	VkCommandBuffer transferCommand = nsVulkan::AllocateTransferCommandBuffer();
	NS_VK_BeginCommand(transferCommand);
	{
		vkCmdCopyBuffer(transferCommand, frame.PrimitiveStagingBuffer->GetVkBuffer(), frame.PrimitiveVertexBuffer->GetVkBuffer(), static_cast<uint32>(vertexCopyRegions.GetCount()), vertexCopyRegions.GetData());
		vkCmdCopyBuffer(transferCommand, frame.PrimitiveStagingBuffer->GetVkBuffer(), frame.PrimitiveIndexBuffer->GetVkBuffer(), static_cast<uint32>(indexCopyRegions.GetCount()), indexCopyRegions.GetData());
	}
	NS_VK_EndCommand(transferCommand);

	nsVulkan::SubmitTransferCommandBuffer(&transferCommand, 1);
	

	PrimitiveBatchMeshVertices.Clear();
	PrimitiveBatchMeshIndices.Clear();

	PrimitiveBatchLineVertices.Clear();
	PrimitiveBatchLineIndices.Clear();
}
