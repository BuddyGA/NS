#pragma once

#include "nsViewport.h"
#include "nsRenderContextWorld.h"


class nsGUIContext;



enum class nsERenderFinalTexture : uint8
{
	NONE = 0,
	SCENE_RENDER_TARGET,
	SCENE_DEPTH_STENCIL,
};



namespace nsERenderDebugDraw
{
	enum Flags : uint8
	{
		None			= (0),
		Wireframe		= (1 << 0),
		Collision		= (1 << 1),
		Skeleton		= (1 << 2),

		// ...

		ALL				= (UINT8_MAX)
	};
};

typedef uint8 nsRenderDebugDrawFlags;



class NS_ENGINE_API nsRenderer
{
	NS_DECLARE_NOCOPY(nsRenderer)

private:
	struct Frame
	{
		nsVulkanRenderPass* RenderPassForward;
		nsVulkanRenderPass* RenderPassFinal;

		nsTextureID SceneRenderTarget;
		nsTextureID SceneDepthStencil;
		
		nsVulkanBuffer* CameraViewUniformBuffer;
		nsVulkanBuffer* LightStorageBuffer;

		// Forward pass global descriptor sets. [0]: Texture (Dynamic indexing), [1]: Environment, [2]: Camera
		VkDescriptorSet ForwardGlobalDescriptorSets[3];
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;

	nsVulkanSwapchain* Swapchain;
	nsTextureID FinalTexture;
	nsMaterialID FullscreenMaterial;

public:
	nsViewport Viewport;
	nsRenderPassFlags RenderPassFlags;
	nsPointInt RenderTargetDimension;
	nsERenderFinalTexture RenderFinalTexture;
	nsRenderContextWorld* RenderContextWorld;
	nsGUIContext* GUIContext;
	nsRenderDebugDrawFlags DebugDrawFlags;


public:
	nsRenderer(nsWindowHandle optWindowHandleForSwapchain = nullptr) noexcept;
	~nsRenderer() noexcept;

private:
	NS_NODISCARD_INLINE bool ShouldResizeTexture(nsTextureID texture) const noexcept
	{
		if (!texture.IsValid())
		{
			return true;
		}

		const nsPointInt textureDimension = nsTextureManager::Get().GetTextureDimension(texture);
		return RenderTargetDimension != textureDimension;
	}


public:
	void BeginRender(int frameIndex, float deltaTime) noexcept;
	void ExecuteRenderPass_Depth(VkCommandBuffer commandBuffer) noexcept;
	void ExecuteRenderPass_Shadow(VkCommandBuffer commandBuffer) noexcept;

private:
	void RenderPassForward_Mesh(VkCommandBuffer commandBuffer);
	void RenderPassForward_Wireframe(VkCommandBuffer commandBuffer);
	void RenderPassForward_PrimitiveBatch(VkCommandBuffer commandBuffer);

public:
	void ExecuteRenderPass_Forward(VkCommandBuffer commandBuffer) noexcept;
	void ExecuteRenderPass_Final(VkCommandBuffer commandBuffer) noexcept;
	void ExecuteDrawCalls() noexcept;


	// Get current frame scene render target
	NS_NODISCARD_INLINE nsTextureID GetSceneRenderTarget() const noexcept
	{
		return FrameDatas[FrameIndex].SceneRenderTarget;
	}


	// Get current frame scene depth-stencil
	NS_NODISCARD_INLINE nsTextureID GetSceneDepthStencil() const noexcept
	{
		return FrameDatas[FrameIndex].SceneDepthStencil;
	}

};
