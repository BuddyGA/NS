#include "nsRenderer.h"
#include "nsTextureManager.h"
#include "nsMaterial.h"
#include "nsMesh.h"
#include "nsAnimationManager.h"
#include "nsGUICore.h"
#include "nsPhysicsManager.h"
#include "nsNavigationManager.h"
#include "nsWorld.h"
#include "API_VK/nsVulkanFunctions.h"



nsRenderer::nsRenderer(nsPlatformWindowHandle optWindowHandleForSwapchain) noexcept
	: FrameDatas()
	, FrameIndex(0)
	, Swapchain(nullptr)
	, RenderTargetDimension(1280, 720)
	, RenderPassFlags(0)
	, RenderFinalTexture(nsERenderFinalTexture::NONE)
	, RenderContextWorld(nullptr)
	, GUIContext(nullptr)
	, World(nullptr)
	, DebugDrawFlags(0)

#ifdef NS_ENGINE_DEBUG_DRAW
	, FrameDebugDatas()
#endif // NS_ENGINE_DEBUG_DRAW

{
	RenderPassFlags |= nsERenderPass::Forward;

	if (optWindowHandleForSwapchain)
	{
		Swapchain = ns_CreateObject<nsVulkanSwapchain>(optWindowHandleForSwapchain);
		RenderPassFlags |= nsERenderPass::Final;
	}

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		
		frame.RenderPassForward = nsVulkanRenderPass::CreateDefault_Forward();

		if (Swapchain)
		{
			frame.RenderPassFinal = nsVulkanRenderPass::CreateDefault_Final();
		}

		frame.CameraViewUniformBuffer = nsVulkan::CreateUniformBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, sizeof(nsRenderCameraView), "camera_view_uniform_buffer");
	}
}


nsRenderer::~nsRenderer() noexcept
{
	NS_ValidateV(0, TEXT("Not implemented yet!"));
}


bool nsRenderer::ShouldResizeTexture(nsTextureID texture) const noexcept
{
	if (!texture.IsValid())
	{
		return true;
	}

	const nsPointInt textureDimension = nsTextureManager::Get().GetTextureDimension(texture);
	return RenderTargetDimension != textureDimension;
}


void nsRenderer::BeginRender(int frameIndex, float deltaTime) noexcept
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];

	if (RenderPassFlags & nsERenderPass::Forward)
	{
		nsTextureManager& textureManager = nsTextureManager::Get();

		// Update scene render target size
		if (ShouldResizeTexture(frame.SceneRenderTarget))
		{
			if (frame.SceneRenderTarget.IsValid())
			{
				textureManager.DestroyTexture(frame.SceneRenderTarget);
			}

			frame.SceneRenderTarget = textureManager.CreateRenderTarget(nsName::Format("scene_render_target_%i", FrameIndex), nsETextureFormat::RENDER_TARGET_BGRA, RenderTargetDimension.X, RenderTargetDimension.Y);
			NS_Assert(frame.SceneRenderTarget != nsTextureID::INVALID);
		}


		// Update scene depth-stencil size
		if (ShouldResizeTexture(frame.SceneDepthStencil))
		{
			if (frame.SceneDepthStencil.IsValid())
			{
				textureManager.DestroyTexture(frame.SceneDepthStencil);
			}

			frame.SceneDepthStencil = textureManager.CreateDepthStencil(nsName::Format("scene_depth_stencil_%i", FrameIndex), nsETextureFormat::DEPTH_STENCIL_D24_S8, RenderTargetDimension.X, RenderTargetDimension.Y);
			NS_Assert(frame.SceneDepthStencil != nsTextureID::INVALID);
		}


		// Update camera view data
		NS_Assert(frame.CameraViewUniformBuffer);
		nsRenderCameraView* cameraViewMap = reinterpret_cast<nsRenderCameraView*>(frame.CameraViewUniformBuffer->MapMemory());
		{
			cameraViewMap->View = Viewport.GetViewMatrix();
			cameraViewMap->Projection = Viewport.GetProjectionMatrix();
			cameraViewMap->WorldPosition = Viewport.GetViewTransform().Position;
		}
		frame.CameraViewUniformBuffer->UnmapMemory();


		if (RenderContextWorld)
		{
			RenderContextWorld->UpdateResourcesAndBuildDrawCalls(FrameIndex);
		}


	#ifdef NS_ENGINE_DEBUG_DRAW

		if (World)
		{
			if (DebugDrawFlags & nsERenderDebugDraw::Collision)
			{
				nsPhysicsManager::Get().DebugDraw(World->GetPhysicsScene(), this);
			}

			if (DebugDrawFlags & nsERenderDebugDraw::Skeleton)
			{
				nsAnimationManager::Get().DebugDraw(this);
			}

			if (DebugDrawFlags & nsERenderDebugDraw::NavMesh)
			{
				nsNavigationManager::Get().DebugDraw(this);
			}
		}

		UpdateDebugDrawCalls();
	#endif // NS_ENGINE_DEBUG_DRAW

	}


	if (RenderPassFlags & nsERenderPass::Final)
	{
		NS_Assert(Swapchain);
		Swapchain->Resize();

		if (RenderFinalTexture != nsERenderFinalTexture::NONE)
		{
			nsMaterialManager& materialManager = nsMaterialManager::Get();

			if (FullscreenMaterial == nsMaterialID::INVALID)
			{
				FullscreenMaterial = materialManager.CreateMaterialInstance("mat_fullscreen", materialManager.GetDefaultMaterial_Fullscreen());
			}

			switch (RenderFinalTexture)
			{
				case nsERenderFinalTexture::SCENE_RENDER_TARGET: FinalTexture = frame.SceneRenderTarget; break;
				case nsERenderFinalTexture::SCENE_DEPTH_STENCIL: FinalTexture = frame.SceneDepthStencil; break;
				default: break;
			}

			materialManager.SetMaterialParameterTextureValue(FullscreenMaterial, "texture", FinalTexture);
			materialManager.BindMaterials(&FullscreenMaterial, 1);
		}

		if (GUIContext)
		{
			GUIContext->UpdateResourcesAndBuildDrawCalls(FrameIndex);
		}
	}
}


void nsRenderer::ExecuteRenderPass_Depth(VkCommandBuffer commandBuffer) noexcept
{

}


void nsRenderer::ExecuteRenderPass_Shadow(VkCommandBuffer commandBuffer) noexcept
{

}


void nsRenderer::RenderPassForward_Mesh(VkCommandBuffer commandBuffer)
{
	if (RenderContextWorld == nullptr)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];
	nsMeshManager& meshManager = nsMeshManager::Get();
	nsTextureManager& textureManager = nsTextureManager::Get();
	nsMaterialManager& materialManager = nsMaterialManager::Get();

	// Bind vertex buffers. [0]: Position, [1]: Attribute
	VkBuffer vertexBuffers[3] = 
	{ 
		meshManager.GetVertexPositionBuffer()->GetVkBuffer(), 
		meshManager.GetVertexAttributeBuffer()->GetVkBuffer(),
		meshManager.GetVertexSkinBuffer()->GetVkBuffer()
	};

	VkDeviceSize vertexOffsets[3] = { 0, 0, 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 3, vertexBuffers, vertexOffsets);

	// Bind index buffer
	VkBuffer indexBuffer = meshManager.GetIndexBuffer()->GetVkBuffer();
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	const nsVulkanShaderResourceLayout* defaultShaderResourceLayout = materialManager.GetDefaultShaderResourceLayout_Forward();
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultShaderResourceLayout->GetVkPipelineLayout(), 0, 4, frame.ForwardGlobalDescriptorSets, 0, nullptr);

	nsMaterialID boundMaterial = nsMaterialID::INVALID;
	const nsVulkanShaderPipeline* boundShaderPipeline = nullptr;
	const nsTArray<nsRenderDrawCallPerMaterial>& drawCallMaterials = RenderContextWorld->GetDrawCallMeshes();

	for (int i = 0; i < drawCallMaterials.GetCount(); ++i)
	{
		const nsRenderDrawCallPerMaterial& perMaterial = drawCallMaterials[i];

		if (boundMaterial != perMaterial.Material)
		{
			boundMaterial = perMaterial.Material;
			const nsMaterialResource& materialResource = materialManager.GetMaterialResource(boundMaterial);
			const VkDescriptorSet materialDescriptorSet = materialManager.GetMaterialDescriptorSet(boundMaterial);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultShaderResourceLayout->GetVkPipelineLayout(), 4, 1, &materialDescriptorSet, 0, nullptr);

			if (boundShaderPipeline != materialResource.ShaderPipeline)
			{
				boundShaderPipeline = materialResource.ShaderPipeline;
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundShaderPipeline->GetVkPipeline());
			}
		}

		const nsTArray<nsRenderDrawCallPerMesh>& drawCallMeshes = perMaterial.Meshes;

		for (int j = 0; j < drawCallMeshes.GetCount(); ++j)
		{
			const nsRenderDrawCallPerMesh& perMesh = drawCallMeshes[j];
			const nsMeshDrawData& meshDrawData = meshManager.GetMeshDrawData(perMesh.Mesh);
			const nsTArray<nsRenderDrawCallPerInstance>& drawInstances = perMesh.Instances;

			for (int k = 0; k < drawInstances.GetCount(); ++k)
			{
				vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(nsMatrix4), &drawInstances[k].WorldTransform);
				vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(nsMatrix4), sizeof(int), &drawInstances[k].BoneTransformIndex);
				vkCmdDrawIndexed(commandBuffer, meshDrawData.IndexCount, 1, meshDrawData.BaseIndex, meshDrawData.IndexVertexOffset, 0);
			}
		}
	}
}


void nsRenderer::RenderPassForward_PrimitiveBatch(VkCommandBuffer commandBuffer)
{
	if (RenderContextWorld == nullptr)
	{
		return;
	}

	const nsRenderDrawCallPrimitiveBatch& drawPrimitiveMesh = RenderContextWorld->GetDrawCallPrimitiveBatchMesh();
	const nsRenderDrawCallPrimitiveBatch& drawPrimitiveLine = RenderContextWorld->GetDrawCallPrimitiveBatchLine();

	if (drawPrimitiveMesh.VertexCount > 0 || drawPrimitiveLine.VertexCount > 0)
	{
		Frame& frame = FrameDatas[FrameIndex];
		nsMeshManager& meshManager = nsMeshManager::Get();
		nsTextureManager& textureManager = nsTextureManager::Get();
		nsMaterialManager& materialManager = nsMaterialManager::Get();

		VkBuffer primitiveVertexBuffer = RenderContextWorld->GetPrimitiveVertexBuffer()->GetVkBuffer();
		VkBuffer primitiveIndexBuffer = RenderContextWorld->GetPrimitiveIndexBuffer()->GetVkBuffer();

		const nsVulkanShaderResourceLayout* primitiveShaderResourceLayout = materialManager.GetDefaultShaderResourceLayout_Primitive();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, primitiveShaderResourceLayout->GetVkPipelineLayout(), 0, 1, &frame.ForwardGlobalDescriptorSets[3], 0, nullptr);

		if (drawPrimitiveMesh.VertexCount > 0)
		{
			NS_Assert(drawPrimitiveMesh.IndexCount > 0);

			const nsMaterialResource& defaultMaterialResource = materialManager.GetMaterialResource(materialManager.GetDefaultMaterial_PrimitiveMesh());
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultMaterialResource.ShaderPipeline->GetVkPipeline());
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &primitiveVertexBuffer, &drawPrimitiveMesh.VertexBufferOffset);
			vkCmdBindIndexBuffer(commandBuffer, primitiveIndexBuffer, drawPrimitiveMesh.IndexBufferOffset, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, drawPrimitiveMesh.IndexCount, 1, 0, 0, 0);
		}

		if (drawPrimitiveLine.VertexCount > 0)
		{
			NS_Assert(drawPrimitiveLine.IndexCount > 0);

			const nsMaterialResource& defaultMaterialResource = materialManager.GetMaterialResource(materialManager.GetDefaultMaterial_PrimitiveLine());
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultMaterialResource.ShaderPipeline->GetVkPipeline());
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &primitiveVertexBuffer, &drawPrimitiveLine.VertexBufferOffset);
			vkCmdBindIndexBuffer(commandBuffer, primitiveIndexBuffer, drawPrimitiveLine.IndexBufferOffset, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, drawPrimitiveLine.IndexCount, 1, 0, 0, 0);
		}
	}
}


void nsRenderer::RenderPassForward_Wireframe(VkCommandBuffer commandBuffer)
{
	if (RenderContextWorld == nullptr)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];
	nsMeshManager& meshManager = nsMeshManager::Get();
	nsTextureManager& textureManager = nsTextureManager::Get();
	nsMaterialManager& materialManager = nsMaterialManager::Get();

	// Bind vertex buffer
	const VkBuffer vertexBuffers[2] =
	{
		meshManager.GetVertexPositionBuffer()->GetVkBuffer(),
		meshManager.GetVertexSkinBuffer()->GetVkBuffer()
	};

	const VkDeviceSize vertexOffsets[2] = { 0, 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, vertexOffsets);

	// Bind index buffer
	VkBuffer indexBuffer = meshManager.GetIndexBuffer()->GetVkBuffer();
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	const nsVulkanShaderResourceLayout* defaultShaderResourceLayout = materialManager.GetDefaultShaderResourceLayout_Wireframe();
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultShaderResourceLayout->GetVkPipelineLayout(), 0, 2, &frame.ForwardGlobalDescriptorSets[2], 0, nullptr);

	const nsMaterialID defaultWireframeMaterial = materialManager.GetDefaultMaterial_Wireframe();
	const nsMaterialResource& materialResource = materialManager.GetMaterialResource(defaultWireframeMaterial);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialResource.ShaderPipeline->GetVkPipeline());

	const nsTArray<nsRenderDrawCallPerMaterial>& drawCallMaterials = RenderContextWorld->GetDrawCallMeshes();

	for (int i = 0; i < drawCallMaterials.GetCount(); ++i)
	{
		const nsTArray<nsRenderDrawCallPerMesh>& drawCallMeshes = drawCallMaterials[i].Meshes;

		for (int j = 0; j < drawCallMeshes.GetCount(); ++j)
		{
			const nsRenderDrawCallPerMesh& perMesh = drawCallMeshes[j];
			const nsMeshDrawData& meshDrawData = meshManager.GetMeshDrawData(perMesh.Mesh);
			const nsTArray<nsRenderDrawCallPerInstance>& drawInstances = perMesh.Instances;

			for (int k = 0; k < drawInstances.GetCount(); ++k)
			{
				vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(nsMatrix4), &drawInstances[k].WorldTransform);
				vkCmdPushConstants(commandBuffer, defaultShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(nsMatrix4), sizeof(int), &drawInstances[k].BoneTransformIndex);
				vkCmdDrawIndexed(commandBuffer, meshDrawData.IndexCount, 1, meshDrawData.BaseIndex, meshDrawData.IndexVertexOffset, 0);
			}
		}
	}
}


void nsRenderer::ExecuteRenderPass_Forward(VkCommandBuffer commandBuffer) noexcept
{
	Frame& frame = FrameDatas[FrameIndex];

	nsMeshManager& meshManager = nsMeshManager::Get();
	nsTextureManager& textureManager = nsTextureManager::Get();
	nsMaterialManager& materialManager = nsMaterialManager::Get();

	const nsVulkanTextureView* renderTargetView = textureManager.GetTextureView(frame.SceneRenderTarget);
	const nsVulkanTextureView* depthStencilView = textureManager.GetTextureView(frame.SceneDepthStencil);

	frame.RenderPassForward->BeginRenderPass(commandBuffer, &renderTargetView, 1, depthStencilView, nullptr, RenderTargetDimension, nsLinearColor(0.1f, 0.15f, 0.2f, 1.0f));

	if (RenderContextWorld)
	{
		const nsPointFloat viewportDimension = Viewport.GetDimension();

		// Bind viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(RenderTargetDimension.X);
		viewport.height = static_cast<float>(RenderTargetDimension.Y);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Bind scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32>(RenderTargetDimension.X), static_cast<uint32>(RenderTargetDimension.Y) };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		const nsVulkanShaderResourceLayout* defaultShaderResourceLayout = materialManager.GetDefaultShaderResourceLayout_Forward();

		// Global descriptor sets. 
		// [0]: Texture (Dynamic indexing) 
		// [1]: Environment 
		// [2]: BoneTransforms
		// [3]: Camera
		frame.ForwardGlobalDescriptorSets[0] = textureManager.GetDescriptorSet();

		if (frame.ForwardGlobalDescriptorSets[1] == VK_NULL_HANDLE)
		{
			NS_Assert(frame.ForwardGlobalDescriptorSets[2] == VK_NULL_HANDLE);
			frame.ForwardGlobalDescriptorSets[1] = nsVulkan::CreateDescriptorSet(defaultShaderResourceLayout, 1);
			frame.ForwardGlobalDescriptorSets[2] = nsVulkan::CreateDescriptorSet(defaultShaderResourceLayout, 2);
			frame.ForwardGlobalDescriptorSets[3] = nsVulkan::CreateDescriptorSet(defaultShaderResourceLayout, 3);
		}

		VkWriteDescriptorSet writeGlobalDescriptorSets[3] = {};
		{
			// Environment
			VkDescriptorBufferInfo environmentBufferInfo{};
			{
				environmentBufferInfo.buffer = RenderContextWorld->GetEnvironmentUniformBuffer()->GetVkBuffer();
				environmentBufferInfo.offset = 0;
				environmentBufferInfo.range = sizeof(nsRenderEnvironmentData);

				writeGlobalDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeGlobalDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeGlobalDescriptorSets[0].dstSet = frame.ForwardGlobalDescriptorSets[1];
				writeGlobalDescriptorSets[0].dstBinding = 0;
				writeGlobalDescriptorSets[0].dstArrayElement = 0;
				writeGlobalDescriptorSets[0].descriptorCount = 1;
				writeGlobalDescriptorSets[0].pBufferInfo = &environmentBufferInfo;
			}

			// Bone transforms
			VkDescriptorBufferInfo boneTransformsBufferInfo{};
			{
				boneTransformsBufferInfo.buffer = nsAnimationManager::Get().GetSkeletonPoseTransformStorageBuffer()->GetVkBuffer();
				boneTransformsBufferInfo.offset = 0;
				boneTransformsBufferInfo.range = VK_WHOLE_SIZE;

				writeGlobalDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeGlobalDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeGlobalDescriptorSets[1].dstSet = frame.ForwardGlobalDescriptorSets[2];
				writeGlobalDescriptorSets[1].dstBinding = 0;
				writeGlobalDescriptorSets[1].dstArrayElement = 0;
				writeGlobalDescriptorSets[1].descriptorCount = 1;
				writeGlobalDescriptorSets[1].pBufferInfo = &boneTransformsBufferInfo;
			}

			// Camera
			VkDescriptorBufferInfo cameraViewBufferInfo{};
			{
				cameraViewBufferInfo.buffer = frame.CameraViewUniformBuffer->GetVkBuffer();
				cameraViewBufferInfo.offset = 0;
				cameraViewBufferInfo.range = sizeof(nsRenderCameraView);

				writeGlobalDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeGlobalDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeGlobalDescriptorSets[2].dstSet = frame.ForwardGlobalDescriptorSets[3];
				writeGlobalDescriptorSets[2].dstBinding = 0;
				writeGlobalDescriptorSets[2].dstArrayElement = 0;
				writeGlobalDescriptorSets[2].descriptorCount = 1;
				writeGlobalDescriptorSets[2].pBufferInfo = &cameraViewBufferInfo;
			}

			vkUpdateDescriptorSets(nsVulkan::GetVkDevice(), 3, writeGlobalDescriptorSets, 0, nullptr);
		}

		if (DebugDrawFlags & nsERenderDebugDraw::Wireframe)
		{
			RenderPassForward_Wireframe(commandBuffer);
		}
		else
		{
			RenderPassForward_Mesh(commandBuffer);
		}

		//RenderPassForward_PrimitiveBatch(commandBuffer);


	#ifdef NS_ENGINE_DEBUG_DRAW
		if (DebugDrawCallMesh.IndexCount > 0 || DebugDrawCallMeshIgnoreDepth.IndexCount > 0 || DebugDrawCallLine.IndexCount > 0 || DebugDrawCallLineIgnoreDepth.IndexCount > 0)
		{
			FrameDebug& frameDebug = FrameDebugDatas[FrameIndex];

			const VkBuffer vertexBuffer = frameDebug.VertexBuffer->GetVkBuffer();
			const VkDeviceSize vertexOffset = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexOffset);
			vkCmdBindIndexBuffer(commandBuffer, frameDebug.IndexBuffer->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

			const nsVulkanShaderResourceLayout* defaultPrimitiveSRL = materialManager.GetDefaultShaderResourceLayout_Primitive();
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPrimitiveSRL->GetVkPipelineLayout(), 0, 1, &frame.ForwardGlobalDescriptorSets[3], 0, nullptr);

			if (DebugDrawCallMesh.IndexCount > 0)
			{
				const nsMaterialResource& resource = materialManager.GetDefaultMaterialResource_PrimitiveMesh();
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderPipeline->GetVkPipeline());
				vkCmdDrawIndexed(commandBuffer, DebugDrawCallMesh.IndexCount, 1, DebugDrawCallMesh.BaseIndex, DebugDrawCallMesh.IndexVertexOffset, 0);
			}

			if (DebugDrawCallMeshIgnoreDepth.IndexCount > 0)
			{
				const nsMaterialResource& resource = materialManager.GetDefaultMaterialResource_PrimitiveMesh_IgnoreDepth();
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderPipeline->GetVkPipeline());
				vkCmdDrawIndexed(commandBuffer, DebugDrawCallMeshIgnoreDepth.IndexCount, 1, DebugDrawCallMeshIgnoreDepth.BaseIndex, DebugDrawCallMeshIgnoreDepth.IndexVertexOffset, 0);
			}

			if (DebugDrawCallLine.IndexCount > 0)
			{
				const nsMaterialResource& resource = materialManager.GetDefaultMaterialResource_PrimitiveLine();
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderPipeline->GetVkPipeline());
				vkCmdDrawIndexed(commandBuffer, DebugDrawCallLine.IndexCount, 1, DebugDrawCallLine.BaseIndex, DebugDrawCallLine.IndexVertexOffset, 0);
			}

			if (DebugDrawCallLineIgnoreDepth.IndexCount > 0)
			{
				const nsMaterialResource& resource = materialManager.GetDefaultMaterialResource_PrimitiveLine_IgnoreDepth();
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderPipeline->GetVkPipeline());
				vkCmdDrawIndexed(commandBuffer, DebugDrawCallLineIgnoreDepth.IndexCount, 1, DebugDrawCallLineIgnoreDepth.BaseIndex, DebugDrawCallLineIgnoreDepth.IndexVertexOffset, 0);
			}
		}
	#endif // NS_ENGINE_DEBUG_DRAW

	}

	frame.RenderPassForward->EndRenderPass(commandBuffer);
}


void nsRenderer::ExecuteRenderPass_Final(VkCommandBuffer commandBuffer) noexcept
{
	Frame& frame = FrameDatas[FrameIndex];

	NS_Assert(Swapchain);
	const nsPointInt swapchainDimension = Swapchain->GetDimension();

	int backbufferIndex = INT32_MAX;

	if (Swapchain->AcquireNextBackbuffer(backbufferIndex))
	{
		const nsVulkanTextureView* backbufferTextureView = Swapchain->GetBackbufferView(backbufferIndex);
		frame.RenderPassFinal->BeginRenderPass(commandBuffer, &backbufferTextureView, 1, nullptr, nullptr, swapchainDimension, nsLinearColor(0.0f, 0.0f, 0.0f, 1.0f));

		const nsPointFloat viewportDimension = Viewport.GetDimension();

		// Bind viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(RenderTargetDimension.X);
		viewport.height = static_cast<float>(RenderTargetDimension.Y);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Bind scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32>(RenderTargetDimension.X), static_cast<uint32>(RenderTargetDimension.Y) };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		if (RenderFinalTexture != nsERenderFinalTexture::NONE)
		{
			const VkDescriptorSet textureDescriptorSet = nsTextureManager::Get().GetDescriptorSet();

			const nsMaterialResource& resource = nsMaterialManager::Get().GetMaterialResource(FullscreenMaterial);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderResourceLayout->GetVkPipelineLayout(), 0, 1, &textureDescriptorSet, 0, nullptr);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resource.ShaderPipeline->GetVkPipeline());
			vkCmdPushConstants(commandBuffer, resource.ShaderResourceLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &FinalTexture);
			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		}

		if (GUIContext)
		{
			GUIContext->ExecuteDrawCalls(commandBuffer);
		}

		frame.RenderPassFinal->EndRenderPass(commandBuffer);

		// Presents
		Swapchain->Present();
	}
}


void nsRenderer::ExecuteDrawCalls() noexcept
{
	// TODO: 
	// - Update each render pass output layout based on RenderPassFlags. This is useful for debugging when we turn on/off render pass
	// - OR VK_KHR_dynamic_rendering


	VkCommandBuffer commandBuffer = nsVulkan::AllocateGraphicsCommandBuffer();
	NS_VK_BeginCommand(commandBuffer);
	{
		NS_Assert(RenderTargetDimension.X > 0 && RenderTargetDimension.Y > 0);

		// TODO: Optimize render passes with multithreading
		

		// RenderPass - Depth
		if (RenderPassFlags & nsERenderPass::Depth)
		{
			ExecuteRenderPass_Depth(commandBuffer);
		}

		// RenderPass - Shadow
		if (RenderPassFlags & nsERenderPass::Shadow)
		{
			ExecuteRenderPass_Shadow(commandBuffer);
		}

		// RenderPass - Forward
		if (RenderPassFlags & nsERenderPass::Forward)
		{
			ExecuteRenderPass_Forward(commandBuffer);
		}

		// RenderPass - Final
		if (RenderPassFlags & nsERenderPass::Final)
		{
			ExecuteRenderPass_Final(commandBuffer);
		}
	}
	NS_VK_EndCommand(commandBuffer);

	nsVulkan::SubmitGraphicsCommandBuffer(&commandBuffer, 1);


#ifdef NS_ENGINE_DEBUG_DRAW
	DebugPrimitiveBatchMesh.Clear();
	DebugDrawCallMesh.IndexCount = 0;

	DebugPrimitiveBatchMeshIgnoreDepth.Clear();
	DebugDrawCallMeshIgnoreDepth.IndexCount = 0;

	DebugPrimitiveBatchLine.Clear();
	DebugDrawCallLine.IndexCount = 0;
	
	DebugPrimitiveBatchLineIgnoreDepth.Clear();
	DebugDrawCallLineIgnoreDepth.IndexCount = 0;
#endif // NS_ENGINE_DEBUG_DRAW
}




#ifdef NS_ENGINE_DEBUG_DRAW

void nsRenderer::UpdateDebugDrawCalls()
{
	const uint64 vertexBufferSize = DebugPrimitiveBatchMesh.GetVertexSizeBytes() + DebugPrimitiveBatchMeshIgnoreDepth.GetVertexSizeBytes() + DebugPrimitiveBatchLine.GetVertexSizeBytes() + DebugPrimitiveBatchLineIgnoreDepth.GetVertexSizeBytes();

	if (vertexBufferSize == 0)
	{
		return;
	}

	const uint64 indexBufferSize = DebugPrimitiveBatchMesh.GetIndexSizeBytes() + DebugPrimitiveBatchMeshIgnoreDepth.GetIndexSizeBytes() + DebugPrimitiveBatchLine.GetIndexSizeBytes() + DebugPrimitiveBatchLineIgnoreDepth.GetIndexSizeBytes();
	NS_Assert(indexBufferSize > 0);

	FrameDebug& frame = FrameDebugDatas[FrameIndex];

	if (frame.VertexBuffer == nullptr)
	{
		frame.VertexBuffer = nsVulkan::CreateVertexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, vertexBufferSize, nsName::Format("debug_vertex_buffer_%i", FrameIndex));
	}
	else
	{
		frame.VertexBuffer->Resize(vertexBufferSize);
	}

	if (frame.IndexBuffer == nullptr)
	{
		frame.IndexBuffer = nsVulkan::CreateIndexBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, indexBufferSize, nsName::Format("debug_index_buffer_%i", FrameIndex));
	}
	else
	{
		frame.IndexBuffer->Resize(indexBufferSize);
	}

	nsVertexPrimitive* vtxBufferMap = reinterpret_cast<nsVertexPrimitive*>(frame.VertexBuffer->MapMemory());
	uint32* idxBufferMap = reinterpret_cast<uint32*>(frame.IndexBuffer->MapMemory());

	int debugVertexCount = 0;
	int debugIndexCount = 0;

	if (!DebugPrimitiveBatchMesh.IsEmpty())
	{
		const nsTArray<nsVertexPrimitive>& vertices = DebugPrimitiveBatchMesh.GetVertices();
		nsPlatform::Memory_Copy(vtxBufferMap + debugVertexCount, vertices.GetData(), DebugPrimitiveBatchMesh.GetVertexSizeBytes());

		const nsTArray<uint32>& indices = DebugPrimitiveBatchMesh.GetIndices();
		nsPlatform::Memory_Copy(idxBufferMap + debugIndexCount, indices.GetData(), DebugPrimitiveBatchMesh.GetIndexSizeBytes());

		DebugDrawCallMesh.BaseIndex = debugIndexCount;
		DebugDrawCallMesh.IndexCount = indices.GetCount();
		DebugDrawCallMesh.IndexVertexOffset = debugVertexCount;

		debugVertexCount += vertices.GetCount();
		debugIndexCount += indices.GetCount();
	}

	if (!DebugPrimitiveBatchMeshIgnoreDepth.IsEmpty())
	{
		const nsTArray<nsVertexPrimitive>& vertices = DebugPrimitiveBatchMeshIgnoreDepth.GetVertices();
		nsPlatform::Memory_Copy(vtxBufferMap + debugVertexCount, vertices.GetData(), DebugPrimitiveBatchMeshIgnoreDepth.GetVertexSizeBytes());

		const nsTArray<uint32>& indices = DebugPrimitiveBatchMeshIgnoreDepth.GetIndices();
		nsPlatform::Memory_Copy(idxBufferMap + debugIndexCount, indices.GetData(), DebugPrimitiveBatchMeshIgnoreDepth.GetIndexSizeBytes());

		DebugDrawCallMeshIgnoreDepth.BaseIndex = debugIndexCount;
		DebugDrawCallMeshIgnoreDepth.IndexCount = indices.GetCount();
		DebugDrawCallMeshIgnoreDepth.IndexVertexOffset = debugVertexCount;

		debugVertexCount += vertices.GetCount();
		debugIndexCount += indices.GetCount();
	}


	if (!DebugPrimitiveBatchLine.IsEmpty())
	{
		const nsTArray<nsVertexPrimitive>& vertices = DebugPrimitiveBatchLine.GetVertices();
		nsPlatform::Memory_Copy(vtxBufferMap + debugVertexCount, vertices.GetData(), DebugPrimitiveBatchLine.GetVertexSizeBytes());

		const nsTArray<uint32>& indices = DebugPrimitiveBatchLine.GetIndices();
		nsPlatform::Memory_Copy(idxBufferMap + debugIndexCount, indices.GetData(), DebugPrimitiveBatchLine.GetIndexSizeBytes());

		DebugDrawCallLine.BaseIndex = debugIndexCount;
		DebugDrawCallLine.IndexCount = indices.GetCount();
		DebugDrawCallLine.IndexVertexOffset = debugVertexCount;

		debugVertexCount += vertices.GetCount();
		debugIndexCount += indices.GetCount();
	}

	if (!DebugPrimitiveBatchLineIgnoreDepth.IsEmpty())
	{
		const nsTArray<nsVertexPrimitive>& vertices = DebugPrimitiveBatchLineIgnoreDepth.GetVertices();
		nsPlatform::Memory_Copy(vtxBufferMap + debugVertexCount, vertices.GetData(), DebugPrimitiveBatchLineIgnoreDepth.GetVertexSizeBytes());

		const nsTArray<uint32>& indices = DebugPrimitiveBatchLineIgnoreDepth.GetIndices();
		nsPlatform::Memory_Copy(idxBufferMap + debugIndexCount, indices.GetData(), DebugPrimitiveBatchLineIgnoreDepth.GetIndexSizeBytes());

		DebugDrawCallLineIgnoreDepth.BaseIndex = debugIndexCount;
		DebugDrawCallLineIgnoreDepth.IndexCount = indices.GetCount();
		DebugDrawCallLineIgnoreDepth.IndexVertexOffset = debugVertexCount;

		debugVertexCount += vertices.GetCount();
		debugIndexCount += indices.GetCount();
	}


	frame.VertexBuffer->UnmapMemory();
	frame.IndexBuffer->UnmapMemory();
}


#endif // NS_ENGINE_DEBUG_DRAW
