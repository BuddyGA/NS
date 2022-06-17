#pragma once

#include "nsEngineTypes.h"


#ifdef _DEBUG
extern nsTArrayPair<uint64, nsName> g_VulkanDebugObjectNamePairs;
#define NS_VK_AddDebugObjectName(handle, name)									\
if (name.GetLength() > 0)														\
{																				\
	g_VulkanDebugObjectNamePairs.Add(reinterpret_cast<uint64>(handle), name);	\
}

#define NS_VK_PrintDestroyVulkanObject(handle)										\
const uint64 __handle = reinterpret_cast<uint64>(handle);							\
const nsName* __debugName = g_VulkanDebugObjectNamePairs.GetValueByKey(__handle);	\
if (__debugName)																	\
{																					\
	NS_LogDebug(VulkanLog, "Destroy vulkan object [%s]", **__debugName);			\
	g_VulkanDebugObjectNamePairs.Remove(__handle, false);							\
}

#else
#define NS_VK_AddDebugObjectName(handle, name) 
#define NS_VK_PrintDestroyVulkanObject(handle)

#endif // _DEBUG


#define NS_VK_SetDebugName(device, type, handle, name)							\
NS_VK_AddDebugObjectName(handle, name)											\
if (nsVulkan::IsValidationLayerEnabled() && name.GetLength() > 0)				\
{																				\
	VkDebugUtilsObjectNameInfoEXT debugNameInfo = {};							\
	debugNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;	\
	debugNameInfo.objectType = type;											\
	debugNameInfo.objectHandle = reinterpret_cast<uint64>(handle);				\
	debugNameInfo.pObjectName = *name;											\
	vkSetDebugUtilsObjectNameEXT(device, &debugNameInfo);						\
} 


extern nsLogCategory VulkanLog;



class nsVulkanBuffer;
class nsVulkanTexture;
class nsVulkanRenderPass;
class nsVulkanShader;
class nsVulkanShaderResourceLayout;
class nsVulkanShaderPipeline;
class nsVulkanSwapchain;



class nsVulkanBuffer
{
private:
	VkBufferCreateInfo Info;
	VmaMemoryUsage MemoryUsage;
	VkBuffer Buffer;
	VmaAllocation Allocation;
	void* MapPtr;

	nsName DebugName;


public:
	nsVulkanBuffer(VmaMemoryUsage memoryUsage, VkBufferUsageFlags usageFlags, VkDeviceSize size, nsName debugName = "") noexcept;
	virtual ~nsVulkanBuffer() noexcept;
	void Resize(VkDeviceSize newSize) noexcept;
	NS_NODISCARD void* MapMemory() noexcept;
	void UnmapMemory() noexcept;
	NS_NODISCARD VkDeviceSize GetMemorySize() noexcept;

	NS_NODISCARD_INLINE VkBuffer GetVkBuffer() const noexcept
	{
		return Buffer;
	}


	NS_DECLARE_NOCOPY(nsVulkanBuffer)

};



class nsVulkanTexture
{
private:
	VkImageCreateInfo Info;
	VkImage Image;
	VmaAllocation Allocation;
	VkImageAspectFlags AspectFlags;


public:
	nsVulkanTexture(VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags, VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName = "") noexcept;
	virtual ~nsVulkanTexture() noexcept;
	NS_NODISCARD VkDeviceSize GetMemorySize() const noexcept;

	NS_NODISCARD_INLINE VkFormat GetFormat() const noexcept
	{
		return Info.format;
	}

	NS_NODISCARD_INLINE uint32 GetWidth() const noexcept
	{
		return Info.extent.width;
	}

	NS_NODISCARD_INLINE uint32 GetHeight() const noexcept
	{
		return Info.extent.height;
	}

	NS_NODISCARD_INLINE uint32 GetMipLevels() const noexcept
	{
		return Info.mipLevels;
	}

	NS_NODISCARD_INLINE VkImageAspectFlags GetAspectFlags() const noexcept
	{
		return AspectFlags;
	}

	NS_NODISCARD_INLINE VkImage GetVkImage() const noexcept
	{
		return Image;
	}


	NS_DECLARE_NOCOPY(nsVulkanTexture)

};



class nsVulkanTextureView
{
private:
	VkImageViewCreateInfo Info;
	VkImageView View;


public:
	nsVulkanTextureView(const nsVulkanTexture* texture, uint32 baseMip, uint32 mipCount, nsName debugName = "") noexcept;
	nsVulkanTextureView(VkImage swapchainImage, VkFormat format, nsName debugName = "") noexcept;
	~nsVulkanTextureView() noexcept;

	NS_NODISCARD_INLINE VkImageView GetVkImageView() const noexcept
	{
		return View;
	}


	NS_DECLARE_NOCOPY(nsVulkanTextureView)

};




class nsVulkanRenderPass
{
private:
	VkRenderPass RenderPass;
	nsTMap<int, VkAttachmentDescription> AttachmentTable;
	nsTArrayInline<VkAttachmentReference, 8> ColorAttachmentReferences;
	VkAttachmentReference DepthStencilAttachmentReference;
	VkAttachmentReference ResolveAttachmentReference;
	int AttachmentReferenceIndex;
	int ColorAttachmentCount;
	bool bHasDepthStencilAttachment;
	bool bHasResolveAttachment;
	VkSubpassDependency SubpassDependency;
	VkFramebuffer Framebuffer;
	nsPointInt FramebufferDimension;

	nsTArrayInline<const nsVulkanTextureView*, 8> PrevRenderTargetViews;
	const nsVulkanTextureView* PrevDepthStencilView;
	const nsVulkanTextureView* PrevResolveView;


public:
	nsVulkanRenderPass() noexcept;
	~nsVulkanRenderPass() noexcept;
	void Reset() noexcept;
	void AddRenderTargetAttachment(int attachmentId, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) noexcept;
	void SetDepthStencilAttachment(int attachmentId, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) noexcept;
	void SetResolveAttachment(int attachmentId, VkFormat format, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) noexcept;
	void SetSubpassDependency(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess) noexcept;
	void Build(nsName debugName = "") noexcept;
	void BeginRenderPass(VkCommandBuffer commandBuffer, const nsVulkanTextureView** renderTargetViews, int renderTargetCount, const nsVulkanTextureView* depthStencilView, const nsVulkanTextureView* resolveView, nsPointInt framebufferDimension, nsLinearColor clearColor, float clearDepth = 1.0f, uint8 clearStencil = 0) noexcept;
	void EndRenderPass(VkCommandBuffer commandBuffer) noexcept;


	NS_NODISCARD_INLINE VkRenderPass GetVkRenderPass() const noexcept
	{
		return RenderPass;
	}

	NS_NODISCARD_INLINE int GetColorAttachmentCount() const noexcept
	{
		return ColorAttachmentCount;
	}

	NS_NODISCARD_INLINE bool HasDepthStencilAttachment() const noexcept
	{
		return bHasDepthStencilAttachment;
	}

	NS_NODISCARD_INLINE bool HasAttachment(int attachmentId) const noexcept
	{
		return AttachmentTable.Exists(attachmentId);
	}

	NS_NODISCARD_INLINE VkImageLayout GetAttachmentInputLayout(int attachmentId) const noexcept
	{
		return AttachmentTable[attachmentId].initialLayout;
	}

	NS_NODISCARD_INLINE VkImageLayout GetAttachmentOutputLayout(int attachmentId) const noexcept
	{
		return AttachmentTable[attachmentId].finalLayout;
	}


public:
	static nsVulkanRenderPass* CreateDefault_Forward() noexcept;
	static nsVulkanRenderPass* CreateDefault_Final() noexcept;


	NS_NODISCARD static NS_INLINE const nsVulkanRenderPass* GetDefault_Forward() noexcept
	{
		static nsVulkanRenderPass* _renderPassForward = CreateDefault_Forward();
		return _renderPassForward;
	}


	NS_NODISCARD static NS_INLINE const nsVulkanRenderPass* GetDefault_Final() noexcept
	{
		static nsVulkanRenderPass* _renderPassFinal = CreateDefault_Final();
		return _renderPassFinal;
	}


	NS_DECLARE_NOCOPY(nsVulkanRenderPass)

};




class nsVulkanShader
{
private:
	VkShaderModule Module;
	VkShaderStageFlagBits Stage;
	nsTArray<uint32> ByteCodes;


public:
	nsVulkanShader(VkShaderStageFlagBits stage, const uint32* codes, uint64 codeSize, nsName debugName = "") noexcept;
	~nsVulkanShader() noexcept;
	NS_NODISCARD const nsTArray<uint32>& GetByteCodes() const noexcept;

	NS_NODISCARD_INLINE VkShaderStageFlagBits GetShaderStage() const noexcept
	{
		return Stage;
	}

	NS_NODISCARD_INLINE VkShaderModule GetVkShaderModule() const noexcept
	{
		return Module;
	}


	NS_DECLARE_NOCOPY(nsVulkanShader)

};



class nsVulkanShaderResourceLayout
{
private:
	struct DescriptorBindingInfo
	{
		VkShaderStageFlags ShaderStages;
		VkDescriptorType Type;
		int ArrayCount;
		bool bIsDynamicIndexing;
	};

	nsTMap<uint32, nsTArrayInline<VkDescriptorSetLayoutBinding, 8>> DescriptorSetLayoutBindings;
	nsTMap<int, nsTArrayInline<DescriptorBindingInfo, 8>> DescriptorBindingInfos;
	nsTArrayInline<VkDescriptorSetLayout, 8> DescriptorSetLayouts;
	nsTArrayInline<VkPushConstantRange, 4> PushConstants;
	VkPipelineLayout PipelineLayout;

	nsName DebugName;

public:
	nsVulkanShaderResourceLayout(nsName debugName = "") noexcept;
	//nsVulkanShaderResourceLayout(nsVulkanShader* vertexShader, nsVulkanShader* fragmentShader, nsName debugName = "") noexcept;
	~nsVulkanShaderResourceLayout() noexcept;

public:
	//void CreateDescriptorSetLayoutBindingsAndPushConstantFromShaderReflect(const nsVulkanShader* shader) noexcept;
	void Reset() noexcept;
	void AddDescriptorBindingUniform(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicOffset, bool bIsDynamicIndexing) noexcept;
	void AddDescriptorBindingStorage(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicOffset, bool bIsDynamicIndexing) noexcept;
	void AddDescriptorBindingTexture(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicIndexing) noexcept;
	void AddPushConstant(VkShaderStageFlags shaderStages, uint32 offset, uint32 size) noexcept;
	void Build();

public:
	NS_NODISCARD_INLINE VkPipelineLayout GetVkPipelineLayout() const noexcept
	{
		return PipelineLayout;
	}

	NS_NODISCARD_INLINE const nsTArrayInline<VkDescriptorSetLayout, 8>& GetDescriptorSetLayouts() const noexcept
	{
		return DescriptorSetLayouts;
	}


public:
	static nsVulkanShaderResourceLayout* GetDefault_TextureShaderResourceLayout() noexcept;


	NS_DECLARE_NOCOPY(nsVulkanShaderResourceLayout)

};



struct nsVulkanPipelineState
{
	const nsVulkanRenderPass* RenderPass;
	const nsVulkanShaderResourceLayout* ShaderResourceLayout;
	nsTArrayInline<VkVertexInputBindingDescription, 4> VertexInputBindings;
	nsTArrayInline<VkVertexInputAttributeDescription, 8> VertexInputAttributes;
	nsVulkanShader* VertexShader;
	nsVulkanShader* TessCtrlShader;
	nsVulkanShader* TessEvalShader;
	nsVulkanShader* GeometryShader;
	nsVulkanShader* FragmentShader;
	VkPrimitiveTopology Topology;
	VkPolygonMode PolygonMode;
	VkFrontFace FrontFace;
	VkCullModeFlagBits CullMode;
	nsEBlendMode BlendMode;
	float DepthBiasConstant;
	float DepthBiasSlope;
	float DepthBiasClamp;
	float MinDepthBound;
	float MaxDepthBound;
	bool bEnableDepthTest;
	bool bEnableDepthWrite;
	bool bEnableDepthBias;
	bool bEnableDepthClamp;
	bool bEnableDepthBound;
	bool bEnableStencilTest;
};



class nsVulkanShaderPipeline
{
public:
	nsVulkanPipelineState PipelineState;
	VkPipeline Pipeline;


public:
	nsVulkanShaderPipeline(VkPipelineCache pipelineCache, const nsVulkanPipelineState& pipelineState, nsName debugName = "") noexcept;
	~nsVulkanShaderPipeline() noexcept;

	NS_NODISCARD_INLINE const nsVulkanPipelineState& GetPipelineState() const noexcept
	{
		return PipelineState;
	}

	NS_NODISCARD_INLINE VkPipeline GetVkPipeline() const noexcept
	{
		return Pipeline;
	}


	NS_DECLARE_NOCOPY(nsVulkanShaderPipeline)

};



class nsVulkanSwapchain
{
private:
	VkSurfaceKHR Surface;
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VkSurfaceFormatKHR SurfaceFormat;
	VkPresentModeKHR RequestedPresentMode;
	VkPresentModeKHR PresentMode;
	VkSwapchainKHR Swapchain;

	nsTArrayInline<nsVulkanTextureView*, NS_ENGINE_FRAME_BUFFERING> BackbufferViews;
	uint32 BackbufferIndex;

	nsTArrayInline<VkSemaphore, NS_ENGINE_FRAME_BUFFERING> ImageAcquiredSemaphores;
	int ImageAcquiredSemaphoreIndex;

	VkResult LastAcquireImageResult;


public:
	nsVulkanSwapchain(nsWindowHandle windowHandle) noexcept;
	~nsVulkanSwapchain() noexcept;
	void SetVsync(bool bEnabled) noexcept;
	void Resize() noexcept;
	bool AcquireNextBackbuffer(int& outBackbufferIndex) noexcept;
	void Present() noexcept;

	NS_NODISCARD_INLINE int GetCurrentBackbufferIndex() const noexcept
	{
		return BackbufferIndex;
	}

	NS_NODISCARD_INLINE nsVulkanTextureView* GetBackbufferView(int index) const noexcept
	{
		return BackbufferViews[index];
	}

	NS_NODISCARD_INLINE nsPointInt GetDimension() const noexcept
	{
		return nsPointInt(static_cast<int>(SurfaceCapabilities.currentExtent.width), static_cast<int>(SurfaceCapabilities.currentExtent.height));
	}

	NS_NODISCARD_INLINE VkSwapchainKHR GetVkSwapchain() const noexcept
	{
		return Swapchain;
	}


	NS_DECLARE_NOCOPY(nsVulkanSwapchain)

};




namespace nsVulkan
{
	extern void Initialize(bool bEnableValidationLayer) noexcept;
	NS_NODISCARD bool IsValidationLayerEnabled() noexcept;
	NS_NODISCARD VkInstance GetVkInstance() noexcept;
	NS_NODISCARD VkPhysicalDevice GetVkPhysicalDevice() noexcept;
	NS_NODISCARD VkDevice GetVkDevice() noexcept;
	NS_NODISCARD VmaAllocator GetVmaAllocator() noexcept;

	void WaitDeviceIdle() noexcept;
	bool CheckPresentSupport(VkSurfaceKHR surface) noexcept;
	nsTArrayInline<VkSurfaceFormatKHR, 16> GetSurfaceFormats(VkSurfaceKHR surface) noexcept;
	nsTArrayInline<VkPresentModeKHR, 16> GetPresentModes(VkSurfaceKHR surface) noexcept;

	NS_NODISCARD VkSemaphore CreateObjectSemaphore() noexcept;
	void DestroyObjectSemaphore(VkSemaphore& semaphore) noexcept;

	NS_NODISCARD nsVulkanBuffer* CreateVertexBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanBuffer* CreateIndexBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanBuffer* CreateUniformBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanBuffer* CreateStorageBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanBuffer* CreateStagingBuffer(VkDeviceSize size, nsName debugName = "") noexcept;
	void DestroyBuffer(nsVulkanBuffer*& buffer) noexcept;

	NS_NODISCARD nsVulkanTexture* CreateTexture2D(VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanTexture* CreateTextureRenderTarget(VkFormat format, uint32 width, uint32 height, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanTexture* CreateTextureDepthStencil(VkFormat format, uint32 width, uint32 height, bool bHasStencil, nsName debugName = "") noexcept;
	void DestroyTexture(nsVulkanTexture*& texture) noexcept;

	NS_NODISCARD void SetupTextureBarrier_TransferDest(const nsVulkanTexture* texture, nsTArray<VkImageMemoryBarrier>& outImageMemoryBarriers) noexcept;
	NS_NODISCARD void SetupTextureBarrier_ShaderRead(const nsVulkanTexture* texture, nsTArray<VkImageMemoryBarrier>& outImageMemoryBarriers) noexcept;

	NS_NODISCARD nsVulkanTextureView* CreateTextureView(const nsVulkanTexture* texture, uint32 baseMip, uint32 mipCount, nsName debugName = "") noexcept;
	NS_NODISCARD nsVulkanTextureView* CreateBackbufferView(VkImage swapchainImage, VkFormat format, nsName debugName = "") noexcept;
	void DestroyTextureView(nsVulkanTextureView*& view) noexcept;

	NS_NODISCARD VkSampler GetDefaultSampler() noexcept;

	NS_NODISCARD nsVulkanShader* CreateShaderModule(VkShaderStageFlagBits type, const uint32* codes, uint64 codeSize, nsName debugName = "") noexcept;
	void DestroyShaderModule(nsVulkanShader*& shaderModule) noexcept;

	NS_NODISCARD nsVulkanShaderResourceLayout* CreateShaderResourceLayout(nsName debugName = "") noexcept;
	//NS_NODISCARD nsVulkanShaderResourceLayout* CreateShaderResourceLayout(nsVulkanShader* vertexShader, nsVulkanShader* fragmentShader, nsName debugName = "") noexcept;
	void DestroyShaderResourceLayout(nsVulkanShaderResourceLayout*& shaderResourceLayout) noexcept;
	
	NS_NODISCARD VkDescriptorSet CreateDescriptorSet(const nsVulkanShaderResourceLayout* shaderResourceLayout, int descriptorSlot) noexcept;
	NS_NODISCARD VkDescriptorSet CreateDescriptorSetDynamicIndexing(const nsVulkanShaderResourceLayout* shaderResourceLayout, int descriptorSlot, int bindingCount) noexcept;
	void DestroyDescriptorSet(VkDescriptorSet& descriptorSet) noexcept;

	NS_NODISCARD nsVulkanShaderPipeline* CreateShaderPipeline(const nsVulkanPipelineState& pipelineState, nsName debugName = "") noexcept;
	void DestroyShaderPipeline(nsVulkanShaderPipeline*& shaderPipeline) noexcept;

	NS_NODISCARD VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const VkImageView* attachments, uint32 attachmentCount, uint32 width, uint32 height) noexcept;
	void DestroyFramebuffer(VkFramebuffer& framebuffer) noexcept;

	NS_NODISCARD VkCommandBuffer AllocateGraphicsCommandBuffer() noexcept;
	void SubmitGraphicsCommandBuffer(const VkCommandBuffer* commandBuffers, int commandCount) noexcept;
	
	NS_NODISCARD VkCommandBuffer AllocateTransferCommandBuffer() noexcept;
	void SubmitTransferCommandBuffer(const VkCommandBuffer* commandBuffers, int commandCount) noexcept;

	void SubmitPresent(VkSwapchainKHR swapchain, uint32 imageIndex, VkSemaphore imageAcquiredSemaphore) noexcept;

	void BeginFrame(int frameIndex) noexcept;
	void Execute() noexcept;

};
