#pragma once

#include "nsViewport.h"
#include "nsRenderContextWorld.h"
#include "nsRenderPrimitiveBatch.h"


class nsGUIContext;



namespace nsERenderPass
{
	enum Flag
	{
		None = (0),
		Depth = (1 << 0),
		Shadow = (1 << 1),
		Forward = (1 << 2),
		Transparency = (1 << 3),
		Final = (1 << 4),
	};
};

typedef uint8 nsRenderPassFlags;



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
		NavMesh			= (1 << 3),

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

		// Forward pass global descriptor sets. [0]: Texture (Dynamic indexing), [1]: Environment, [2]: BoneTransforms [3]: Camera
		VkDescriptorSet ForwardGlobalDescriptorSets[4];
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;

	nsVulkanSwapchain* Swapchain;
	nsTextureID FinalTexture;
	nsMaterialID FullscreenMaterial;

public:
	nsViewport Viewport;
	nsPointInt RenderTargetDimension;
	nsRenderPassFlags RenderPassFlags;
	nsERenderFinalTexture RenderFinalTexture;
	nsRenderContextWorld* RenderContextWorld;
	nsGUIContext* GUIContext;
	nsWorld* World;
	nsRenderDebugDrawFlags DebugDrawFlags;


public:
	nsRenderer(nsWindowHandle optWindowHandleForSwapchain = nullptr) noexcept;
	~nsRenderer() noexcept;

private:
	NS_NODISCARD bool ShouldResizeTexture(nsTextureID texture) const noexcept;

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



#ifdef NS_ENGINE_DEBUG_DRAW
private:
	struct FrameDebug
	{
		nsVulkanBuffer* VertexBuffer;
		nsVulkanBuffer* IndexBuffer;
	};

	FrameDebug FrameDebugDatas[NS_ENGINE_FRAME_BUFFERING];


	struct DebugDrawCall
	{
		int BaseIndex;
		int IndexCount;
		int IndexVertexOffset;
	};

	nsRenderPrimitiveBatchMesh DebugPrimitiveBatchMesh;
	DebugDrawCall DebugDrawCallMesh;

	nsRenderPrimitiveBatchMesh DebugPrimitiveBatchMeshIgnoreDepth;
	DebugDrawCall DebugDrawCallMeshIgnoreDepth;

	nsRenderPrimitiveBatchLine DebugPrimitiveBatchLine;
	DebugDrawCall DebugDrawCallLine;

	nsRenderPrimitiveBatchLine DebugPrimitiveBatchLineIgnoreDepth;
	DebugDrawCall DebugDrawCallLineIgnoreDepth;


private:
	void UpdateDebugDrawCalls();


public:
	NS_INLINE void DebugDrawMeshTriangle(const nsVector3& a, const nsVector3& b, const nsVector3& c, nsColor color, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchMesh& batchMesh = bIgnoreDepth ? DebugPrimitiveBatchMeshIgnoreDepth : DebugPrimitiveBatchMesh;
		batchMesh.AddTriangle(a, b, c, color);
	}


	NS_INLINE void DebugDrawMeshAABB(const nsVector3& boxMin, const nsVector3& boxMax, nsColor color, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchMesh& batchMesh = bIgnoreDepth ? DebugPrimitiveBatchMeshIgnoreDepth : DebugPrimitiveBatchMesh;
		batchMesh.AddBoxAABB(boxMin, boxMax, color);
	}


	NS_INLINE void DebugDrawMeshPrism(const nsVector3& position, const nsQuaternion& rotation, float width, float height, nsColor color, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchMesh& batchMesh = bIgnoreDepth ? DebugPrimitiveBatchMeshIgnoreDepth : DebugPrimitiveBatchMesh;
		batchMesh.AddPrism(position, rotation, width, height, color);
	}


	NS_INLINE void DebugDrawLine(const nsVector3& start, const nsVector3& end, nsColor color, uint8 order = 0, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchLine& batchLine = bIgnoreDepth ? DebugPrimitiveBatchLineIgnoreDepth : DebugPrimitiveBatchLine;
		batchLine.AddLine(start, end, color);
	}


	NS_INLINE void DebugDrawLineCircle(const nsVector3& center, float radius, float halfArcRadian, nsEAxisType arcAxis, nsColor color, uint8 order = 0, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchLine& batchLine = bIgnoreDepth ? DebugPrimitiveBatchLineIgnoreDepth : DebugPrimitiveBatchLine;
		batchLine.AddCircle(center, radius, halfArcRadian, arcAxis, color);
	}


	NS_INLINE void DebugDrawLineCircleAroundAxis(const nsVector3& center, const nsVector3& axis, float radius, float halfArcRadian, nsColor color, uint8 order = 0, bool bIgnoreDepth = false)
	{
		nsRenderPrimitiveBatchLine& batchLine = bIgnoreDepth ? DebugPrimitiveBatchLineIgnoreDepth : DebugPrimitiveBatchLine;
		batchLine.AddCircleAroundAxis(center, radius, axis, halfArcRadian, color);
	}

#endif // NS_ENGINE_DEBUG_DRAW

};
