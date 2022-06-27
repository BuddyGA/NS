#include "nsVulkan.h"
#include "nsVulkanFunctions.h"

#define VMA_IMPLEMENTATION
#include <vulkan/vk_mem_alloc.h>


#define NS_VK_DEFINE_FUNCTION(func) PFN_##func func


// Global functions
NS_VK_DEFINE_FUNCTION(vkGetInstanceProcAddr);
NS_VK_DEFINE_FUNCTION(vkEnumerateInstanceLayerProperties);
NS_VK_DEFINE_FUNCTION(vkEnumerateInstanceExtensionProperties);
NS_VK_DEFINE_FUNCTION(vkCreateInstance);
NS_VK_DEFINE_FUNCTION(vkDestroyInstance);


// Instance functions
NS_VK_DEFINE_FUNCTION(vkEnumeratePhysicalDevices);
NS_VK_DEFINE_FUNCTION(vkEnumerateDeviceExtensionProperties);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceProperties);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceFeatures);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceFeatures2);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceMemoryProperties2KHR);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
NS_VK_DEFINE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
NS_VK_DEFINE_FUNCTION(vkDestroySurfaceKHR);
NS_VK_DEFINE_FUNCTION(vkCreateDevice);

#ifdef VK_USE_PLATFORM_WIN32_KHR
NS_VK_DEFINE_FUNCTION(vkCreateWin32SurfaceKHR);
#endif // VK_USE_PLATFORM_WIN32_KHR


// Device functions
NS_VK_DEFINE_FUNCTION(vkGetDeviceProcAddr);
NS_VK_DEFINE_FUNCTION(vkDestroyDevice);
NS_VK_DEFINE_FUNCTION(vkDeviceWaitIdle);
NS_VK_DEFINE_FUNCTION(vkGetDeviceQueue);
NS_VK_DEFINE_FUNCTION(vkQueueWaitIdle);
NS_VK_DEFINE_FUNCTION(vkQueueSubmit);
NS_VK_DEFINE_FUNCTION(vkQueuePresentKHR);
NS_VK_DEFINE_FUNCTION(vkCreateFence);
NS_VK_DEFINE_FUNCTION(vkDestroyFence);
NS_VK_DEFINE_FUNCTION(vkWaitForFences);
NS_VK_DEFINE_FUNCTION(vkResetFences);
NS_VK_DEFINE_FUNCTION(vkCreateSemaphore);
NS_VK_DEFINE_FUNCTION(vkDestroySemaphore);
NS_VK_DEFINE_FUNCTION(vkAllocateMemory);
NS_VK_DEFINE_FUNCTION(vkFreeMemory);
NS_VK_DEFINE_FUNCTION(vkMapMemory);
NS_VK_DEFINE_FUNCTION(vkUnmapMemory);
NS_VK_DEFINE_FUNCTION(vkFlushMappedMemoryRanges);
NS_VK_DEFINE_FUNCTION(vkInvalidateMappedMemoryRanges);
NS_VK_DEFINE_FUNCTION(vkGetBufferMemoryRequirements);
NS_VK_DEFINE_FUNCTION(vkGetBufferMemoryRequirements2KHR);
NS_VK_DEFINE_FUNCTION(vkCreateBuffer);
NS_VK_DEFINE_FUNCTION(vkDestroyBuffer);
NS_VK_DEFINE_FUNCTION(vkBindBufferMemory);
NS_VK_DEFINE_FUNCTION(vkBindBufferMemory2KHR);
NS_VK_DEFINE_FUNCTION(vkGetImageMemoryRequirements);
NS_VK_DEFINE_FUNCTION(vkGetImageMemoryRequirements2KHR);
NS_VK_DEFINE_FUNCTION(vkCreateImage);
NS_VK_DEFINE_FUNCTION(vkDestroyImage);
NS_VK_DEFINE_FUNCTION(vkBindImageMemory);
NS_VK_DEFINE_FUNCTION(vkBindImageMemory2KHR);
NS_VK_DEFINE_FUNCTION(vkCreateImageView);
NS_VK_DEFINE_FUNCTION(vkDestroyImageView);
NS_VK_DEFINE_FUNCTION(vkCreateSampler);
NS_VK_DEFINE_FUNCTION(vkDestroySampler);
NS_VK_DEFINE_FUNCTION(vkCreateRenderPass);
NS_VK_DEFINE_FUNCTION(vkDestroyRenderPass);
NS_VK_DEFINE_FUNCTION(vkCreateFramebuffer);
NS_VK_DEFINE_FUNCTION(vkDestroyFramebuffer);
NS_VK_DEFINE_FUNCTION(vkCreateShaderModule);
NS_VK_DEFINE_FUNCTION(vkDestroyShaderModule);
NS_VK_DEFINE_FUNCTION(vkCreateDescriptorPool);
NS_VK_DEFINE_FUNCTION(vkDestroyDescriptorPool);
NS_VK_DEFINE_FUNCTION(vkCreateDescriptorSetLayout);
NS_VK_DEFINE_FUNCTION(vkDestroyDescriptorSetLayout);
NS_VK_DEFINE_FUNCTION(vkAllocateDescriptorSets);
NS_VK_DEFINE_FUNCTION(vkFreeDescriptorSets);
NS_VK_DEFINE_FUNCTION(vkUpdateDescriptorSets);
NS_VK_DEFINE_FUNCTION(vkCreatePipelineLayout);
NS_VK_DEFINE_FUNCTION(vkDestroyPipelineLayout);
NS_VK_DEFINE_FUNCTION(vkCreateGraphicsPipelines);
NS_VK_DEFINE_FUNCTION(vkDestroyPipeline);
NS_VK_DEFINE_FUNCTION(vkCreateSwapchainKHR);
NS_VK_DEFINE_FUNCTION(vkDestroySwapchainKHR);
NS_VK_DEFINE_FUNCTION(vkGetSwapchainImagesKHR);
NS_VK_DEFINE_FUNCTION(vkAcquireNextImageKHR);
NS_VK_DEFINE_FUNCTION(vkCreateCommandPool);
NS_VK_DEFINE_FUNCTION(vkDestroyCommandPool);
NS_VK_DEFINE_FUNCTION(vkResetCommandPool);
NS_VK_DEFINE_FUNCTION(vkAllocateCommandBuffers);
NS_VK_DEFINE_FUNCTION(vkFreeCommandBuffers);
NS_VK_DEFINE_FUNCTION(vkBeginCommandBuffer);
NS_VK_DEFINE_FUNCTION(vkEndCommandBuffer);
NS_VK_DEFINE_FUNCTION(vkCmdBeginRenderPass);
NS_VK_DEFINE_FUNCTION(vkCmdEndRenderPass);
NS_VK_DEFINE_FUNCTION(vkCmdCopyBuffer);
NS_VK_DEFINE_FUNCTION(vkCmdCopyBufferToImage);
NS_VK_DEFINE_FUNCTION(vkCmdPipelineBarrier);
NS_VK_DEFINE_FUNCTION(vkCmdBindVertexBuffers);
NS_VK_DEFINE_FUNCTION(vkCmdBindIndexBuffer);
NS_VK_DEFINE_FUNCTION(vkCmdSetViewport);
NS_VK_DEFINE_FUNCTION(vkCmdSetScissor);
NS_VK_DEFINE_FUNCTION(vkCmdBindPipeline);
NS_VK_DEFINE_FUNCTION(vkCmdPushConstants);
NS_VK_DEFINE_FUNCTION(vkCmdBindDescriptorSets);
NS_VK_DEFINE_FUNCTION(vkCmdDraw);
NS_VK_DEFINE_FUNCTION(vkCmdDrawIndexed);
NS_VK_DEFINE_FUNCTION(vkCreatePipelineCache);
NS_VK_DEFINE_FUNCTION(vkDestroyPipelineCache);

NS_VK_DEFINE_FUNCTION(vkCreateDebugUtilsMessengerEXT);
NS_VK_DEFINE_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
NS_VK_DEFINE_FUNCTION(vkSetDebugUtilsObjectNameEXT);
NS_VK_DEFINE_FUNCTION(vkSetDebugUtilsObjectTagEXT);
NS_VK_DEFINE_FUNCTION(vkQueueBeginDebugUtilsLabelEXT);
NS_VK_DEFINE_FUNCTION(vkQueueEndDebugUtilsLabelEXT);
NS_VK_DEFINE_FUNCTION(vkQueueInsertDebugUtilsLabelEXT);
NS_VK_DEFINE_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
NS_VK_DEFINE_FUNCTION(vkCmdEndDebugUtilsLabelEXT);
NS_VK_DEFINE_FUNCTION(vkCmdInsertDebugUtilsLabelEXT);


#define NS_VK_GetInstanceFunction(func) func = (PFN_##func)vkGetInstanceProcAddr(Instance, #func)
#define NS_VK_GetDeviceFunction(func) func = (PFN_##func)vkGetDeviceProcAddr(Device, #func)


nsLogCategory VulkanLog("nsVulkanLog", nsELogVerbosity::LV_DEBUG);

static nsMemory VulkanMemory("vulkan_default", NS_MEMORY_SIZE_KiB(128), 16);
static nsModuleHandle Module;
static VkInstance Instance;
static VkDebugUtilsMessengerEXT DebugUtilsMessenger;
static VkPhysicalDevice PhysicalDevice;
static VkDevice Device;
static VkQueue GraphicsQueue;
static VkQueue ComputeQueue;
static VkQueue TransferQueue;
static VkQueue PresentSupportQueue;
static uint32 GraphicsQueueFamilyIndex;
static uint32 ComputeQueueFamilyIndex;
static uint32 TransferQueueFamilyIndex;
static uint32 PresentSupportQueueFamilyIndex;
static VmaAllocator Allocator;
static VkDescriptorPool DescriptorPool;
static VkPipelineCache PipelineCache;

static bool bValidationLayer;
static bool bVulkanInitialized;


struct nsVulkanFrame
{
	// Transfer queue data
	VkCommandPool TransferCommandPool;
	nsTArrayInline<VkCommandBuffer, 4> TransferCommandBuffers;
	int TransferCommandBufferAllocateIndex;
	VkSemaphore TransferSignalSemaphore;
	VkFence TransferFence;

	// Graphics queue data
	VkCommandPool GraphicsCommandPool;
	nsTArrayInline<VkCommandBuffer, 8> GraphicsCommandBuffers;
	int GraphicsCommandBufferAllocateIndex;
	VkSemaphore GraphicsSignalSemaphore;
	VkFence GraphicsFence;

	// Cleanup
	nsTArrayInline<VkFence, 8> WaitFences;
	nsTArrayInline<nsVulkanBuffer*, 16> PendingDestroyBuffers;
	nsTArrayInline<nsVulkanTexture*, 16> PendingDestroyTextures;
	nsTArrayInline<nsVulkanTextureView*, 16> PendingDestroyTextureViews;
	nsTArrayInline<nsVulkanShader*, 8> PendingDestroyShaderModules;
	nsTArrayInline<nsVulkanShaderResourceLayout*, 8> PendingDestroyShaderResourceLayouts;
	nsTArrayInline<nsVulkanShaderPipeline*, 8> PendingDestroyShaderPipelines;
	nsTArrayInline<VkSemaphore, 8> PendingDestroySemaphores;
	nsTArrayInline<VkFramebuffer, 16> PendingDestroyFramebuffers;
	nsTArrayInline<VkDescriptorSet, 16> PendingDestroyDescriptorSets;

	// Execution
	nsTArrayInline<VkCommandBuffer, 4> SubmittedTransferCommandBuffers;
	nsTArrayInline<VkCommandBuffer, 8> SubmittedGraphicsCommandBuffers;
	nsTArrayInline<VkSemaphore, 4> GraphicsWaitSemaphores;
	nsTArrayInline<VkPipelineStageFlags, 4> GraphicsWaitDstMasks;
	nsTArrayInline<VkSwapchainKHR, 8> PresentSwapchains;
	nsTArrayInline<uint32, 8> PresentImageIndices;
	nsTArrayInline<VkSemaphore, 8> PresentImageAcquiredSemaphores;
};

static int FrameIndex;
static nsVulkanFrame FrameDatas[NS_ENGINE_FRAME_BUFFERING];


#ifdef _DEBUG
#define NS_VK_NewObject(type, name, ...) VulkanMemory.AllocateConstructDebug<type>(name, __VA_ARGS__)
nsTArrayPair<uint64, nsName> g_VulkanDebugObjectNamePairs;

#else
#define NS_VK_NewObject(type, name, ...) VulkanMemory.AllocateContruct<type>(__VA_ARGS__)

#endif // _DEBUG



static VkBool32 ns_VulkanDebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	const char* type = nullptr;

	if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	{
		type = "VALIDATION";
	}
	else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	{
		type = "PERFORMANCE";
	}
	else
	{
		type = "GENERAL";
	}

	nsString message = nsString::Format("[%s]\n\t%s\n", type, pCallbackData->pMessage);

	if (pCallbackData->objectCount > 0)
	{
		for (uint32 i = 0; i < pCallbackData->objectCount; ++i)
		{
			message += nsString::Format("\tObjectName: %s\n", pCallbackData->pObjects[i].pObjectName);
		}
	}

	if (pCallbackData->queueLabelCount > 0)
	{
		for (uint32 i = 0; i < pCallbackData->queueLabelCount; ++i)
		{
			message += nsString::Format("\tQueue: %s\n", pCallbackData->pQueueLabels[i].pLabelName);
		}
	}

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		NS_LogWarning(VulkanLog, *message);
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		NS_LogInfo(VulkanLog, *message);
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		NS_LogError(VulkanLog, *message);
		NS_ValidateV(0, "Vulkan validation error!");
	}
	else
	{
		NS_LogDebug(VulkanLog, *message);
	}

	return VK_FALSE;
}


static NS_INLINE VkCommandPool ns_VulkanCreateCommandPool(uint32 queueFamilyIndex) noexcept
{
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = 0;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool commandPool = VK_NULL_HANDLE;
	vkCreateCommandPool(Device, &createInfo, nullptr, &commandPool);

	return commandPool;
}


static NS_INLINE void ns_VulkanAllocateCommandBuffers(VkCommandPool commandPool, uint32 allocateCount, VkCommandBuffer* outCommandBuffers) noexcept
{
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = allocateCount;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(Device, &allocateInfo, outCommandBuffers);
}


static NS_INLINE VkFence ns_VulkanCreateFence() noexcept
{
	VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	VkFence fence = VK_NULL_HANDLE;
	vkCreateFence(Device, &createInfo, nullptr, &fence);

	return fence;
}


static NS_INLINE VkImageMemoryBarrier ns_VulkanImageMemoryBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32 baseMip, uint32 mipCount, uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) noexcept
{
	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.image = image;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	barrier.subresourceRange.aspectMask = aspectFlags;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = baseMip;
	barrier.subresourceRange.levelCount = mipCount;

	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;

	return barrier;
}





void nsVulkan::Initialize(bool bEnableValidationLayer) noexcept
{
	if (bVulkanInitialized)
	{
		return;
	}

	NS_LogInfo(VulkanLog, "Initialize vulkan");
	bValidationLayer = bEnableValidationLayer;

	NS_Assert(Module == nullptr);
	Module = nsPlatform::Module_Load(NS_VK_MODULE_NAME);

	vkGetInstanceProcAddr = nsPlatform::Module_GetFunctionAs<PFN_vkGetInstanceProcAddr>(Module, "vkGetInstanceProcAddr");
	vkEnumerateInstanceLayerProperties = nsPlatform::Module_GetFunctionAs<PFN_vkEnumerateInstanceLayerProperties>(Module, "vkEnumerateInstanceLayerProperties");
	vkEnumerateInstanceExtensionProperties = nsPlatform::Module_GetFunctionAs<PFN_vkEnumerateInstanceExtensionProperties>(Module, "vkEnumerateInstanceExtensionProperties");
	vkCreateInstance = nsPlatform::Module_GetFunctionAs<PFN_vkCreateInstance>(Module, "vkCreateInstance");
	vkDestroyInstance = nsPlatform::Module_GetFunctionAs<PFN_vkDestroyInstance>(Module, "vkDestroyInstance");

	// Validate instance layers
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	nsTArray<VkLayerProperties> instanceLayerProperties(static_cast<int>(instanceLayerCount));
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.GetData());

	nsTArray<const char*> instanceLayers;

	if (bEnableValidationLayer)
	{
		instanceLayers.Add("VK_LAYER_KHRONOS_validation");
	};


	for (int i = 0; i < instanceLayers.GetCount(); ++i)
	{
		const char* name = instanceLayers[i];
		bool bSupported = false;

		for (uint32 j = 0; j < instanceLayerCount; ++j)
		{
			if (strcmp(name, instanceLayerProperties[j].layerName) == 0)
			{
				bSupported = true;
				break;
			}
		}

		if (!bSupported)
		{
			NS_LogError(VulkanLog, "Required instance layer %s not supported!", name);
			NS_Abort();
			return;
		}
	}

	nsTArray<const char*> instanceExtensions =
	{
	#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#endif // VK_USE_PLATFORM_WIN32_KHR

		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
	};

	if (bEnableValidationLayer)
	{
		instanceExtensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}


	auto CheckInstanceExtension = [](const char* layerName, const char* extensionName)
	{
		uint32 count = 0;
		vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
		NS_Assert(count <= 32);

		VkExtensionProperties properties[32];
		vkEnumerateInstanceExtensionProperties(layerName, &count, properties);

		for (uint32 i = 0; i < count; ++i)
		{
			if (strcmp(extensionName, properties[i].extensionName) == 0)
			{
				return true;
			}
		}

		return false;
	};


	for (int i = 0; i < instanceExtensions.GetCount(); ++i)
	{
		const char* name = instanceExtensions[i];
		bool bSupported = false;

		if (CheckInstanceExtension(nullptr, name))
		{
			bSupported = true;
		}
		else
		{
			for (uint32 j = 0; j < instanceLayerCount; ++j)
			{
				if (CheckInstanceExtension(instanceLayers[j], name))
				{
					bSupported = true;
					break;
				}
			}
		}

		if (!bSupported)
		{
			NS_LogError(VulkanLog, "Fail to initialize vulkan. Required instance extension %s not supported!", name);
			NS_Abort();
			return;
		}
	}


	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_2;
	appInfo.pEngineName = "nsEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "nsApplication";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.GetData();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32>(instanceLayers.GetCount());
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.GetData();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32>(instanceExtensions.GetCount());
	vkCreateInstance(&instanceCreateInfo, nullptr, &Instance);

	NS_VK_GetInstanceFunction(vkEnumeratePhysicalDevices);
	NS_VK_GetInstanceFunction(vkEnumerateDeviceExtensionProperties);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceProperties);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceFeatures);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceFeatures2);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceMemoryProperties);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceMemoryProperties2KHR);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceQueueFamilyProperties);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceSurfaceSupportKHR);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceSurfaceFormatsKHR);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceSurfacePresentModesKHR);
	NS_VK_GetInstanceFunction(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	NS_VK_GetInstanceFunction(vkDestroySurfaceKHR);
	NS_VK_GetInstanceFunction(vkCreateDevice);
	NS_VK_GetInstanceFunction(vkGetDeviceProcAddr);

#ifdef VK_USE_PLATFORM_WIN32_KHR
	NS_VK_GetInstanceFunction(vkCreateWin32SurfaceKHR);
#endif // VK_USE_PLATFORM_WIN32_KHR

	NS_VK_GetInstanceFunction(vkCreateDebugUtilsMessengerEXT);
	NS_VK_GetInstanceFunction(vkDestroyDebugUtilsMessengerEXT);

	if (bEnableValidationLayer)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
		debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		debugUtilsCreateInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)ns_VulkanDebugMessengerCallback;
		vkCreateDebugUtilsMessengerEXT(Instance, &debugUtilsCreateInfo, nullptr, &DebugUtilsMessenger);
	}


	uint32 gpuCount = 0;
	vkEnumeratePhysicalDevices(Instance, &gpuCount, nullptr);
	nsTArray<VkPhysicalDevice> gpuList(static_cast<int>(gpuCount));
	vkEnumeratePhysicalDevices(Instance, &gpuCount, gpuList.GetData());
	bool bFoundSuitableGPU = false;


	// Local helper function to check device extension
	auto CheckDeviceExtension = [](VkPhysicalDevice physicalDevice, const char* layerName, const char* extensionName)
	{
		uint32 count = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &count, nullptr);
		nsTArray<VkExtensionProperties> properties(static_cast<int>(count));
		vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &count, properties.GetData());

		for (int i = 0; i < properties.GetCount(); ++i)
		{
			if (strcmp(extensionName, properties[i].extensionName) == 0)
			{
				return true;
			}
		}

		return false;
	};

	const nsTArray<const char*>& deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		//VK_KHR_MAINTENANCE2_EXTENSION_NAME,
		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		//VK_KHR_MULTIVIEW_EXTENSION_NAME,
		//VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
		//VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME
		//VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};


	for (int i = 0; i < gpuList.GetCount(); ++i)
	{
		VkPhysicalDevice gpu = gpuList[i];
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(gpu, &properties);

		// Required descrete GPU
		if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			continue;

		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
		VkPhysicalDeviceFeatures2 featuresEXT = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &descriptorIndexingFeatures };
		vkGetPhysicalDeviceFeatures2(gpu, &featuresEXT);

		// Required features
		const bool bHaveFeatures =
			featuresEXT.features.geometryShader &&
			featuresEXT.features.tessellationShader &&
			featuresEXT.features.textureCompressionBC &&
			featuresEXT.features.pipelineStatisticsQuery &&
			descriptorIndexingFeatures.descriptorBindingPartiallyBound &&
			descriptorIndexingFeatures.runtimeDescriptorArray;

		if (!bHaveFeatures)
		{
			continue;
		}

		bool bSupportRequiredExtensions = true;

		for (int i = 0; i < deviceExtensions.GetCount(); ++i)
		{
			const char* name = deviceExtensions[i];
			bool bSupported = false;

			if (CheckDeviceExtension(gpu, nullptr, name))
			{
				bSupported = true;
			}
			else
			{
				for (int j = 0; j < instanceLayers.GetCount(); ++j)
				{
					if (CheckDeviceExtension(gpu, instanceLayers[j], name))
					{
						bSupported = true;
						break;
					}
				}
			}

			if (!bSupported)
			{
				bSupportRequiredExtensions = false;
				break;
			}
		}

		if (!bSupportRequiredExtensions)
			continue;

		uint32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
		nsTArray<VkQueueFamilyProperties> queueFamilyProperties(static_cast<int>(queueFamilyCount));
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProperties.GetData());

		uint32 graphicsQueueFamilyIndex = UINT32_MAX;
		uint32 dedicatedComputeQueueFamilyIndex = UINT32_MAX;
		uint32 dedicatedTransferQueueFamilyIndex = UINT32_MAX;

		for (uint32 i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFlags& queueFlags = queueFamilyProperties[i].queueFlags;
			const uint32 graphicsQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

			if (graphicsQueueFamilyIndex == UINT32_MAX && (queueFlags & graphicsQueueFlags))
			{
				graphicsQueueFamilyIndex = i;
			}

			if (dedicatedComputeQueueFamilyIndex == UINT32_MAX && !(queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				dedicatedComputeQueueFamilyIndex = i;
			}

			if (dedicatedTransferQueueFamilyIndex == UINT32_MAX && !(queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFlags & VK_QUEUE_TRANSFER_BIT))
			{
				dedicatedTransferQueueFamilyIndex = i;
			}
		}

		if (graphicsQueueFamilyIndex != UINT32_MAX)
		{
			PhysicalDevice = gpu;
			GraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
			ComputeQueueFamilyIndex = (dedicatedComputeQueueFamilyIndex == UINT32_MAX) ? graphicsQueueFamilyIndex : dedicatedComputeQueueFamilyIndex;
			TransferQueueFamilyIndex = (dedicatedTransferQueueFamilyIndex == UINT32_MAX) ? graphicsQueueFamilyIndex : dedicatedTransferQueueFamilyIndex;
			PresentSupportQueueFamilyIndex = UINT32_MAX;
			bFoundSuitableGPU = true;

			NS_LogDebug(VulkanLog, "Found supported GPU [Vendor: %s, Name: %s, DriverVersion: %u.%u.%u, VideoMemory: %u MiB]",
				ns_VendorName(static_cast<int>(properties.vendorID)), properties.deviceName,
				VK_VERSION_MAJOR(properties.driverVersion), VK_VERSION_MINOR(properties.driverVersion), VK_VERSION_PATCH(properties.driverVersion),
				0 / NS_MEMORY_SIZE_MiB(1)
			);

			break;
		}
	}

	if (!bFoundSuitableGPU)
	{
		NS_LogError(VulkanLog, "GPU with vulkan support not found!");
		NS_Abort();
	}


	const float queuePriority = 1.0f;

	nsTArray<VkDeviceQueueCreateInfo> queueInfos;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;
	queueInfos.Add(queueCreateInfo);

	if (ComputeQueueFamilyIndex != GraphicsQueueFamilyIndex)
	{
		queueCreateInfo.queueFamilyIndex = ComputeQueueFamilyIndex;
		queueInfos.Add(queueCreateInfo);
	}

	if (TransferQueueFamilyIndex != GraphicsQueueFamilyIndex)
	{
		queueCreateInfo.queueFamilyIndex = TransferQueueFamilyIndex;
		queueInfos.Add(queueCreateInfo);
	}

	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
	VkPhysicalDeviceFeatures2 enabledFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &descriptorIndexingFeatures };
	vkGetPhysicalDeviceFeatures2(PhysicalDevice, &enabledFeatures);
	NS_Validate(descriptorIndexingFeatures.descriptorBindingPartiallyBound&& descriptorIndexingFeatures.runtimeDescriptorArray);
	enabledFeatures.pNext = &descriptorIndexingFeatures;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = &enabledFeatures;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.GetData();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.GetCount());
	deviceCreateInfo.pQueueCreateInfos = queueInfos.GetData();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32>(queueInfos.GetCount());
	vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &Device);

	NS_VK_GetDeviceFunction(vkDestroyDevice);
	NS_VK_GetDeviceFunction(vkDeviceWaitIdle);
	NS_VK_GetDeviceFunction(vkGetDeviceQueue);
	NS_VK_GetDeviceFunction(vkQueueWaitIdle);
	NS_VK_GetDeviceFunction(vkQueueSubmit);
	NS_VK_GetDeviceFunction(vkQueuePresentKHR);
	NS_VK_GetDeviceFunction(vkCreateFence);
	NS_VK_GetDeviceFunction(vkDestroyFence);
	NS_VK_GetDeviceFunction(vkWaitForFences);
	NS_VK_GetDeviceFunction(vkResetFences);
	NS_VK_GetDeviceFunction(vkCreateSemaphore);
	NS_VK_GetDeviceFunction(vkDestroySemaphore);
	NS_VK_GetDeviceFunction(vkAllocateMemory);
	NS_VK_GetDeviceFunction(vkFreeMemory);
	NS_VK_GetDeviceFunction(vkMapMemory);
	NS_VK_GetDeviceFunction(vkUnmapMemory);
	NS_VK_GetDeviceFunction(vkFlushMappedMemoryRanges);
	NS_VK_GetDeviceFunction(vkInvalidateMappedMemoryRanges);
	NS_VK_GetDeviceFunction(vkGetBufferMemoryRequirements);
	NS_VK_GetDeviceFunction(vkGetBufferMemoryRequirements2KHR);
	NS_VK_GetDeviceFunction(vkCreateBuffer);
	NS_VK_GetDeviceFunction(vkDestroyBuffer);
	NS_VK_GetDeviceFunction(vkBindBufferMemory);
	NS_VK_GetDeviceFunction(vkBindBufferMemory2KHR);
	NS_VK_GetDeviceFunction(vkGetImageMemoryRequirements);
	NS_VK_GetDeviceFunction(vkGetImageMemoryRequirements2KHR);
	NS_VK_GetDeviceFunction(vkCreateImage);
	NS_VK_GetDeviceFunction(vkDestroyImage);
	NS_VK_GetDeviceFunction(vkBindImageMemory);
	NS_VK_GetDeviceFunction(vkBindImageMemory2KHR);
	NS_VK_GetDeviceFunction(vkCreateImageView);
	NS_VK_GetDeviceFunction(vkDestroyImageView);
	NS_VK_GetDeviceFunction(vkCreateSampler);
	NS_VK_GetDeviceFunction(vkDestroySampler);
	NS_VK_GetDeviceFunction(vkCreateRenderPass);
	NS_VK_GetDeviceFunction(vkDestroyRenderPass);
	NS_VK_GetDeviceFunction(vkCreateFramebuffer);
	NS_VK_GetDeviceFunction(vkDestroyFramebuffer);
	NS_VK_GetDeviceFunction(vkCreateShaderModule);
	NS_VK_GetDeviceFunction(vkDestroyShaderModule);
	NS_VK_GetDeviceFunction(vkCreateDescriptorPool);
	NS_VK_GetDeviceFunction(vkDestroyDescriptorPool);
	NS_VK_GetDeviceFunction(vkCreateDescriptorSetLayout);
	NS_VK_GetDeviceFunction(vkDestroyDescriptorSetLayout);
	NS_VK_GetDeviceFunction(vkAllocateDescriptorSets);
	NS_VK_GetDeviceFunction(vkFreeDescriptorSets);
	NS_VK_GetDeviceFunction(vkUpdateDescriptorSets);
	NS_VK_GetDeviceFunction(vkCreatePipelineLayout);
	NS_VK_GetDeviceFunction(vkDestroyPipelineLayout);
	NS_VK_GetDeviceFunction(vkCreateGraphicsPipelines);
	NS_VK_GetDeviceFunction(vkDestroyPipeline);
	NS_VK_GetDeviceFunction(vkCreateSwapchainKHR);
	NS_VK_GetDeviceFunction(vkDestroySwapchainKHR);
	NS_VK_GetDeviceFunction(vkGetSwapchainImagesKHR);
	NS_VK_GetDeviceFunction(vkAcquireNextImageKHR);
	NS_VK_GetDeviceFunction(vkCreateCommandPool);
	NS_VK_GetDeviceFunction(vkDestroyCommandPool);
	NS_VK_GetDeviceFunction(vkResetCommandPool);
	NS_VK_GetDeviceFunction(vkAllocateCommandBuffers);
	NS_VK_GetDeviceFunction(vkFreeCommandBuffers);
	NS_VK_GetDeviceFunction(vkBeginCommandBuffer);
	NS_VK_GetDeviceFunction(vkEndCommandBuffer);
	NS_VK_GetDeviceFunction(vkCmdBeginRenderPass);
	NS_VK_GetDeviceFunction(vkCmdEndRenderPass);
	NS_VK_GetDeviceFunction(vkCmdCopyBuffer);
	NS_VK_GetDeviceFunction(vkCmdCopyBufferToImage);
	NS_VK_GetDeviceFunction(vkCmdPipelineBarrier);
	NS_VK_GetDeviceFunction(vkCmdBindVertexBuffers);
	NS_VK_GetDeviceFunction(vkCmdBindIndexBuffer);
	NS_VK_GetDeviceFunction(vkCmdSetViewport);
	NS_VK_GetDeviceFunction(vkCmdSetScissor);
	NS_VK_GetDeviceFunction(vkCmdBindPipeline);
	NS_VK_GetDeviceFunction(vkCmdPushConstants);
	NS_VK_GetDeviceFunction(vkCmdBindDescriptorSets);
	NS_VK_GetDeviceFunction(vkCmdDraw);
	NS_VK_GetDeviceFunction(vkCmdDrawIndexed);
	NS_VK_GetDeviceFunction(vkCreatePipelineCache);
	NS_VK_GetDeviceFunction(vkDestroyPipelineCache);

	if (bValidationLayer)
	{
		NS_VK_GetDeviceFunction(vkCreateDebugUtilsMessengerEXT);
		NS_VK_GetDeviceFunction(vkDestroyDebugUtilsMessengerEXT);
		NS_VK_GetDeviceFunction(vkSetDebugUtilsObjectNameEXT);
		NS_VK_GetDeviceFunction(vkSetDebugUtilsObjectTagEXT);
		NS_VK_GetDeviceFunction(vkQueueBeginDebugUtilsLabelEXT);
		NS_VK_GetDeviceFunction(vkQueueEndDebugUtilsLabelEXT);
		NS_VK_GetDeviceFunction(vkQueueInsertDebugUtilsLabelEXT);
		NS_VK_GetDeviceFunction(vkCmdBeginDebugUtilsLabelEXT);
		NS_VK_GetDeviceFunction(vkCmdEndDebugUtilsLabelEXT);
		NS_VK_GetDeviceFunction(vkCmdInsertDebugUtilsLabelEXT);
	}

	vkGetDeviceQueue(Device, GraphicsQueueFamilyIndex, 0, &GraphicsQueue);
	NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_QUEUE, GraphicsQueue, nsName("vk_queue_graphics"));

	ComputeQueue = GraphicsQueue;
	TransferQueue = GraphicsQueue;
	PresentSupportQueue = VK_NULL_HANDLE;

	if (ComputeQueueFamilyIndex != GraphicsQueueFamilyIndex)
	{
		vkGetDeviceQueue(Device, ComputeQueueFamilyIndex, 0, &ComputeQueue);
		NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_QUEUE, ComputeQueue, nsName("vk_queue_compute"));
	}

	if (TransferQueueFamilyIndex != GraphicsQueueFamilyIndex)
	{
		vkGetDeviceQueue(Device, TransferQueueFamilyIndex, 0, &TransferQueue);
		NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_QUEUE, TransferQueue, nsName("vk_queue_transfer"));
	}


	// Setup memory allocator
	VmaVulkanFunctions vmaFunctions = {};
	vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vmaFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vmaFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vmaFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
	vmaFunctions.vkAllocateMemory = vkAllocateMemory;
	vmaFunctions.vkFreeMemory = vkFreeMemory;
	vmaFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vmaFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vmaFunctions.vkCreateBuffer = vkCreateBuffer;
	vmaFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vmaFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vmaFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	vmaFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vmaFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	vmaFunctions.vkCreateImage = vkCreateImage;
	vmaFunctions.vkDestroyImage = vkDestroyImage;
	vmaFunctions.vkBindImageMemory = vkBindImageMemory;
	vmaFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
	vmaFunctions.vkMapMemory = vkMapMemory;
	vmaFunctions.vkUnmapMemory = vkUnmapMemory;
	vmaFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vmaFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vmaFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	allocatorCreateInfo.instance = Instance;
	allocatorCreateInfo.physicalDevice = PhysicalDevice;
	allocatorCreateInfo.device = Device;
	allocatorCreateInfo.pHeapSizeLimit = NULL;
	allocatorCreateInfo.preferredLargeHeapBlockSize = (32 * 1024 * 1024);
	allocatorCreateInfo.pVulkanFunctions = &vmaFunctions;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
	vmaCreateAllocator(&allocatorCreateInfo, &Allocator);


	VkDescriptorPoolSize descriptorPoolSizes[3] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT * NS_ENGINE_FRAME_BUFFERING) },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8 * NS_ENGINE_FRAME_BUFFERING },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8 * NS_ENGINE_FRAME_BUFFERING },
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
	descriptorPoolCreateInfo.poolSizeCount = 3;
	descriptorPoolCreateInfo.maxSets = 64;
	vkCreateDescriptorPool(Device, &descriptorPoolCreateInfo, nullptr, &DescriptorPool);

	PipelineCache = VK_NULL_HANDLE;
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };


	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		nsVulkanFrame& frame = FrameDatas[i];

		frame.TransferCommandPool = ns_VulkanCreateCommandPool(TransferQueueFamilyIndex);
		frame.TransferCommandBuffers.Resize(4);
		ns_VulkanAllocateCommandBuffers(frame.TransferCommandPool, 4, frame.TransferCommandBuffers.GetData());
		frame.TransferCommandBufferAllocateIndex = 0;
		frame.TransferFence = ns_VulkanCreateFence();
		frame.TransferSignalSemaphore = CreateObjectSemaphore();
		NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_SEMAPHORE, frame.TransferSignalSemaphore, nsName::Format("vk_transfer_signal_semaphore_%i", i));


		frame.GraphicsCommandPool = ns_VulkanCreateCommandPool(GraphicsQueueFamilyIndex);
		frame.GraphicsCommandBuffers.Resize(4);
		ns_VulkanAllocateCommandBuffers(frame.GraphicsCommandPool, 4, frame.GraphicsCommandBuffers.GetData());
		frame.GraphicsCommandBufferAllocateIndex = 0;
		frame.GraphicsFence = ns_VulkanCreateFence();
		frame.GraphicsSignalSemaphore = CreateObjectSemaphore();
		NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_SEMAPHORE, frame.GraphicsSignalSemaphore, nsName::Format("vk_graphics_signal_semaphore_%i", i));
	}


	bVulkanInitialized = true;
}


bool nsVulkan::IsValidationLayerEnabled() noexcept
{
	return bValidationLayer;
}


VkInstance nsVulkan::GetVkInstance() noexcept
{
	NS_Assert(bVulkanInitialized);
	return Instance;
}


VkPhysicalDevice nsVulkan::GetVkPhysicalDevice() noexcept
{
	NS_Assert(bVulkanInitialized);
	return PhysicalDevice;
}


VkDevice nsVulkan::GetVkDevice() noexcept
{
	NS_Assert(bVulkanInitialized);
	return Device;
}


VmaAllocator nsVulkan::GetVmaAllocator() noexcept
{
	NS_Assert(bVulkanInitialized);
	return Allocator;
}


void nsVulkan::WaitDeviceIdle() noexcept
{
	vkDeviceWaitIdle(Device);
}


bool nsVulkan::CheckPresentSupport(VkSurfaceKHR surface) noexcept
{
	VkBool32 bPresentSupported = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsQueueFamilyIndex, surface, &bPresentSupported);

	if (bPresentSupported)
	{
		PresentSupportQueueFamilyIndex = GraphicsQueueFamilyIndex;
		PresentSupportQueue = GraphicsQueue;
	}
	else
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, ComputeQueueFamilyIndex, surface, &bPresentSupported);

		if (bPresentSupported)
		{
			PresentSupportQueueFamilyIndex = ComputeQueueFamilyIndex;
			PresentSupportQueue = ComputeQueue;
		}
		else
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, TransferQueueFamilyIndex, surface, &bPresentSupported);

			if (bPresentSupported)
			{
				PresentSupportQueueFamilyIndex = TransferQueueFamilyIndex;
				PresentSupportQueue = TransferQueue;
			}
		}
	}

	return bPresentSupported == VK_TRUE;
}


nsTArrayInline<VkSurfaceFormatKHR, 16> nsVulkan::GetSurfaceFormats(VkSurfaceKHR surface) noexcept
{
	uint32 count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, surface, &count, nullptr);
	NS_Assert(count <= 16);
	nsTArrayInline<VkSurfaceFormatKHR, 16> surfaceFormats;
	surfaceFormats.Resize(static_cast<int>(count));
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, surface, &count, surfaceFormats.GetData());

	return surfaceFormats;
}


nsTArrayInline<VkPresentModeKHR, 16> nsVulkan::GetPresentModes(VkSurfaceKHR surface) noexcept
{
	uint32 count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, surface, &count, nullptr);
	nsTArrayInline<VkPresentModeKHR, 16> presentModes;
	presentModes.Resize(static_cast<int>(count));
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, surface, &count, presentModes.GetData());

	return presentModes;
}


VkSemaphore nsVulkan::CreateObjectSemaphore() noexcept
{
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkSemaphore semaphore = VK_NULL_HANDLE;
	vkCreateSemaphore(Device, &createInfo, nullptr, &semaphore);

	return semaphore;
}


void nsVulkan::DestroyObjectSemaphore(VkSemaphore& semaphore) noexcept
{
	if (semaphore)
	{
		FrameDatas[FrameIndex].PendingDestroySemaphores.Add(semaphore);
		semaphore = VK_NULL_HANDLE;
	}
}


nsVulkanBuffer* nsVulkan::CreateVertexBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanBuffer, debugName, memoryUsage, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, debugName);
}


nsVulkanBuffer* nsVulkan::CreateIndexBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanBuffer, debugName, memoryUsage, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, debugName);
}


nsVulkanBuffer* nsVulkan::CreateUniformBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanBuffer, debugName, memoryUsage, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size, debugName);
}


nsVulkanBuffer* nsVulkan::CreateStorageBuffer(VmaMemoryUsage memoryUsage, VkDeviceSize size, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanBuffer, debugName, memoryUsage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size, debugName);
}


nsVulkanBuffer* nsVulkan::CreateStagingBuffer(VkDeviceSize size, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanBuffer, debugName, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, debugName);
}


void nsVulkan::DestroyBuffer(nsVulkanBuffer*& buffer) noexcept
{
	if (buffer)
	{
		FrameDatas[FrameIndex].PendingDestroyBuffers.Add(buffer);
		buffer = nullptr;
	}
}


nsVulkanTexture* nsVulkan::CreateTexture2D(VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName) noexcept
{
	// nsVulkanTexture(VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags, VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName = "")
	return NS_VK_NewObject(nsVulkanTexture, debugName, 
		VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_IMAGE_ASPECT_COLOR_BIT, 
		format, 
		width, height, 
		mipLevels, 
		debugName
	);
}


nsVulkanTexture* nsVulkan::CreateTextureRenderTarget(VkFormat format, uint32 width, uint32 height, nsName debugName) noexcept
{
	// nsVulkanTexture(VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags, VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName = "")
	return NS_VK_NewObject(nsVulkanTexture, debugName, 
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_IMAGE_ASPECT_COLOR_BIT, 
		format, 
		width, height, 
		1, 
		debugName
	);
}


nsVulkanTexture* nsVulkan::CreateTextureDepthStencil(VkFormat format, uint32 width, uint32 height, bool bHasStencil, nsName debugName) noexcept
{
	// nsVulkanTexture(VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags, VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName = "")
	return NS_VK_NewObject(nsVulkanTexture, debugName, 
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		bHasStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT,
		format, 
		width, height, 
		1, 
		debugName
	);
}


void nsVulkan::DestroyTexture(nsVulkanTexture*& texture) noexcept
{
	if (texture)
	{
		FrameDatas[FrameIndex].PendingDestroyTextures.Add(texture);
		texture = nullptr;
	}
}


void nsVulkan::SetupTextureBarrier_TransferDest(const nsVulkanTexture* texture, nsTArray<VkImageMemoryBarrier>& outImageMemoryBarriers) noexcept
{
	NS_Assert(texture);

	outImageMemoryBarriers.Add(ns_VulkanImageMemoryBarrier(texture->GetVkImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->GetAspectFlags(), 0, static_cast<uint32>(texture->GetMipLevels()), VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED));
}


void nsVulkan::SetupTextureBarrier_ShaderRead(const nsVulkanTexture* texture, nsTArray<VkImageMemoryBarrier>& outImageMemoryBarriers) noexcept
{
	NS_Assert(texture);

	outImageMemoryBarriers.Add(ns_VulkanImageMemoryBarrier(texture->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture->GetAspectFlags(), 0, static_cast<uint32>(texture->GetMipLevels()), VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED));
}


nsVulkanTextureView* nsVulkan::CreateTextureView(const nsVulkanTexture* texture, uint32 baseMip, uint32 mipCount, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanTextureView, debugName, texture, baseMip, mipCount, debugName);
}


nsVulkanTextureView* nsVulkan::CreateBackbufferView(VkImage swapchainImage, VkFormat format, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanTextureView, debugName, swapchainImage, format, debugName);
}


void nsVulkan::DestroyTextureView(nsVulkanTextureView*& view) noexcept
{
	if (view)
	{
		FrameDatas[FrameIndex].PendingDestroyTextureViews.Add(view);
		view = nullptr;
	}
}


nsVulkanShader* nsVulkan::CreateShaderModule(VkShaderStageFlagBits type, const uint32* codes, uint64 codeSize, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanShader, debugName, type, codes, codeSize, debugName);
}


void nsVulkan::DestroyShaderModule(nsVulkanShader*& shaderModule) noexcept
{
	if (shaderModule)
	{
		FrameDatas[FrameIndex].PendingDestroyShaderModules.Add(shaderModule);
		shaderModule = nullptr;
	}
}

/*
nsVulkanShaderResourceLayout* nsVulkan::CreateShaderResourceLayout(nsVulkanShader* vertexShader, nsVulkanShader* fragmentShader, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanShaderResourceLayout, debugName, vertexShader, fragmentShader, debugName);
}
*/

nsVulkanShaderResourceLayout* nsVulkan::CreateShaderResourceLayout(nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanShaderResourceLayout, debugName, debugName);
}


void nsVulkan::DestroyShaderResourceLayout(nsVulkanShaderResourceLayout*& shaderResourceLayout) noexcept
{
	if (shaderResourceLayout)
	{
		FrameDatas[FrameIndex].PendingDestroyShaderResourceLayouts.Add(shaderResourceLayout);
		shaderResourceLayout = nullptr;
	}
}


VkDescriptorSet nsVulkan::CreateDescriptorSet(const nsVulkanShaderResourceLayout* shaderResourceLayout, int descriptorSlot) noexcept
{
	NS_Assert(shaderResourceLayout);

	const nsTArrayInline<VkDescriptorSetLayout, 8>& descriptorSetLayouts = shaderResourceLayout->GetDescriptorSetLayouts();
	NS_Assert(descriptorSlot >= 0 && descriptorSlot < descriptorSetLayouts.GetCount());

	VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorPool = DescriptorPool;
	allocateInfo.pSetLayouts = &descriptorSetLayouts[descriptorSlot];
	allocateInfo.descriptorSetCount = 1;
	
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	vkAllocateDescriptorSets(Device, &allocateInfo, &descriptorSet);

	return descriptorSet;
}


VkDescriptorSet nsVulkan::CreateDescriptorSetDynamicIndexing(const nsVulkanShaderResourceLayout* shaderResourceLayout, int descriptorSlot, int bindingCount) noexcept
{
	const nsTArrayInline<VkDescriptorSetLayout, 8>& descriptorSetLayouts = shaderResourceLayout->GetDescriptorSetLayouts();
	NS_Assert(descriptorSlot >= 0 && descriptorSlot < descriptorSetLayouts.GetCount());

	VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorPool = DescriptorPool;
	allocateInfo.pSetLayouts = &descriptorSetLayouts[descriptorSlot];
	allocateInfo.descriptorSetCount = 1;

	VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	countInfo.descriptorSetCount = 1;
	countInfo.pDescriptorCounts = reinterpret_cast<uint32*>(&bindingCount);

	allocateInfo.pNext = &countInfo;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	vkAllocateDescriptorSets(Device, &allocateInfo, &descriptorSet);

	return descriptorSet;
}


void nsVulkan::DestroyDescriptorSet(VkDescriptorSet& descriptorSet) noexcept
{
	if (descriptorSet)
	{
		FrameDatas[FrameIndex].PendingDestroyDescriptorSets.Add(descriptorSet);
		descriptorSet = VK_NULL_HANDLE;
	}
}


nsVulkanShaderPipeline* nsVulkan::CreateShaderPipeline(const nsVulkanPipelineState& pipelineState, nsName debugName) noexcept
{
	return NS_VK_NewObject(nsVulkanShaderPipeline, debugName, PipelineCache, pipelineState, debugName);
}


void nsVulkan::DestroyShaderPipeline(nsVulkanShaderPipeline*& shaderPipeline) noexcept
{
	if (shaderPipeline)
	{
		FrameDatas[FrameIndex].PendingDestroyShaderPipelines.Add(shaderPipeline);
		shaderPipeline = nullptr;
	}
}


VkFramebuffer nsVulkan::CreateFramebuffer(VkRenderPass renderPass, const VkImageView* attachments, uint32 attachmentCount, uint32 width, uint32 height) noexcept
{
	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = renderPass;
	createInfo.pAttachments = attachments;
	createInfo.attachmentCount = attachmentCount;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	vkCreateFramebuffer(Device, &createInfo, nullptr, &framebuffer);

	return framebuffer;
}


void nsVulkan::DestroyFramebuffer(VkFramebuffer& framebuffer) noexcept
{
	if (framebuffer)
	{
		FrameDatas[FrameIndex].PendingDestroyFramebuffers.Add(framebuffer);
		framebuffer = VK_NULL_HANDLE;
	}
}


VkSampler nsVulkan::GetDefaultSampler() noexcept
{
	static VkSampler sampler;

	if (sampler == VK_NULL_HANDLE)
	{
		VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.maxAnisotropy = 0.0f;
		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_NEVER;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 16.0f;
		createInfo.mipLodBias = 0.0f;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		vkCreateSampler(Device, &createInfo, nullptr, &sampler);
		NS_VK_SetDebugName(Device, VK_OBJECT_TYPE_SAMPLER, sampler, nsName("vk_sampler_def_linear"));
	}

	return sampler;
}


VkCommandBuffer nsVulkan::AllocateGraphicsCommandBuffer() noexcept
{
	nsVulkanFrame& frame = FrameDatas[FrameIndex];
	NS_AssertV(frame.GraphicsCommandBufferAllocateIndex + 1 < frame.GraphicsCommandBuffers.GetCount(), "Exceeds maximum capacity graphics command buffer pool!");
	return frame.GraphicsCommandBuffers[frame.GraphicsCommandBufferAllocateIndex++];
}


void nsVulkan::SubmitGraphicsCommandBuffer(const VkCommandBuffer* commandBuffers, int commandCount) noexcept
{
	NS_Assert(commandCount > 0);

	nsVulkanFrame& frame = FrameDatas[FrameIndex];
	
	for (int i = 0; i < commandCount; ++i)
	{
		NS_Assert(commandBuffers[i]);
		frame.SubmittedGraphicsCommandBuffers.Add(commandBuffers[i]);
	}
}


VkCommandBuffer nsVulkan::AllocateTransferCommandBuffer() noexcept
{
	nsVulkanFrame& frame = FrameDatas[FrameIndex];
	NS_AssertV(frame.TransferCommandBufferAllocateIndex + 1 < frame.TransferCommandBuffers.GetCount(), "Exceeds maximum capacity transfer command buffer pool!");
	return frame.TransferCommandBuffers[frame.TransferCommandBufferAllocateIndex++];
}


void nsVulkan::SubmitTransferCommandBuffer(const VkCommandBuffer* commandBuffers, int commandCount) noexcept
{
	NS_Assert(commandCount > 0);

	nsVulkanFrame& frame = FrameDatas[FrameIndex];

	for (int i = 0; i < commandCount; ++i)
	{
		NS_Assert(commandBuffers[i]);
		frame.SubmittedTransferCommandBuffers.Add(commandBuffers[i]);
	}
}


void nsVulkan::SubmitPresent(VkSwapchainKHR swapchain, uint32 imageIndex, VkSemaphore imageAcquiredSemaphore) noexcept
{
	nsVulkanFrame& frame = FrameDatas[FrameIndex];
	frame.PresentSwapchains.Add(swapchain);
	frame.PresentImageIndices.Add(imageIndex);
	frame.GraphicsWaitSemaphores.Add(imageAcquiredSemaphore);
	frame.GraphicsWaitDstMasks.Add(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}



#define NS_VK_DestroyResources(resourceArray) \
for (int i = 0; i < resourceArray.GetCount(); ++i) \
{ \
	VulkanMemory.DeallocateDestruct(resourceArray[i]); \
} \
resourceArray.Clear(); 


void nsVulkan::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;
	nsVulkanFrame& frame = FrameDatas[FrameIndex];

	if (!frame.WaitFences.IsEmpty())
	{
		vkWaitForFences(Device, static_cast<uint32>(frame.WaitFences.GetCount()), frame.WaitFences.GetData(), VK_TRUE, UINT64_MAX);
		vkResetFences(Device, static_cast<uint32>(frame.WaitFences.GetCount()), frame.WaitFences.GetData());
		frame.WaitFences.Clear();
	}

	NS_VK_DestroyResources(frame.PendingDestroyBuffers);
	NS_VK_DestroyResources(frame.PendingDestroyTextures);
	NS_VK_DestroyResources(frame.PendingDestroyTextureViews);
	NS_VK_DestroyResources(frame.PendingDestroyShaderModules);
	NS_VK_DestroyResources(frame.PendingDestroyShaderResourceLayouts);
	NS_VK_DestroyResources(frame.PendingDestroyShaderPipelines);


	for (int i = 0; i < frame.PendingDestroySemaphores.GetCount(); ++i)
	{
		vkDestroySemaphore(Device, frame.PendingDestroySemaphores[i], nullptr);
	}
	frame.PendingDestroySemaphores.Clear();


	for (int i = 0; i < frame.PendingDestroyFramebuffers.GetCount(); ++i)
	{
		vkDestroyFramebuffer(Device, frame.PendingDestroyFramebuffers[i], nullptr);
	}
	frame.PendingDestroyFramebuffers.Clear();


	if (!frame.PendingDestroyDescriptorSets.IsEmpty())
	{
		vkFreeDescriptorSets(Device, DescriptorPool, static_cast<uint32>(frame.PendingDestroyDescriptorSets.GetCount()), frame.PendingDestroyDescriptorSets.GetData());
		frame.PendingDestroyDescriptorSets.Clear();
	}


	vkResetCommandPool(Device, frame.TransferCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	frame.TransferCommandBufferAllocateIndex = 0;

	vkResetCommandPool(Device, frame.GraphicsCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	frame.GraphicsCommandBufferAllocateIndex = 0;

	frame.SubmittedTransferCommandBuffers.Clear();
	frame.SubmittedGraphicsCommandBuffers.Clear();
	frame.GraphicsWaitSemaphores.Clear();
	frame.GraphicsWaitDstMasks.Clear();
	frame.PresentSwapchains.Clear();
	frame.PresentImageIndices.Clear();
	frame.PresentImageAcquiredSemaphores.Clear();
}


void nsVulkan::Execute() noexcept
{
	nsVulkanFrame& frameData = FrameDatas[FrameIndex];

	const bool bExecuteTransfers = !frameData.SubmittedTransferCommandBuffers.IsEmpty();
	const bool bExecuteGraphics = !frameData.SubmittedGraphicsCommandBuffers.IsEmpty();
	const bool bExecutePresents = !frameData.PresentSwapchains.IsEmpty();

	// Transfers
	if (bExecuteTransfers)
	{
		VkSubmitInfo transferSubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		transferSubmit.pCommandBuffers = frameData.SubmittedTransferCommandBuffers.GetData();
		transferSubmit.commandBufferCount = static_cast<uint32>(frameData.SubmittedTransferCommandBuffers.GetCount());

		if (bExecuteGraphics)
		{
			transferSubmit.pSignalSemaphores = &frameData.TransferSignalSemaphore;
			transferSubmit.signalSemaphoreCount = 1;
		}

		vkQueueSubmit(TransferQueue, 1, &transferSubmit, frameData.TransferFence);
		frameData.WaitFences.Add(frameData.TransferFence);
	}

	// Graphics
	if (bExecuteGraphics)
	{
		VkSubmitInfo graphicsSubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		graphicsSubmit.pCommandBuffers = frameData.SubmittedGraphicsCommandBuffers.GetData();
		graphicsSubmit.commandBufferCount = static_cast<uint32>(frameData.SubmittedGraphicsCommandBuffers.GetCount());

		if (bExecuteTransfers)
		{
			frameData.GraphicsWaitSemaphores.Add(frameData.TransferSignalSemaphore);
			frameData.GraphicsWaitDstMasks.Add(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
		}

		if (bExecutePresents)
		{
			for (int i = 0; i < frameData.PresentImageAcquiredSemaphores.GetCount(); ++i)
			{
				frameData.GraphicsWaitSemaphores.Add(frameData.PresentImageAcquiredSemaphores[i]);
				frameData.GraphicsWaitDstMasks.Add(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			}

			graphicsSubmit.pSignalSemaphores = &frameData.GraphicsSignalSemaphore;
			graphicsSubmit.signalSemaphoreCount = 1;
		}

		graphicsSubmit.pWaitSemaphores = frameData.GraphicsWaitSemaphores.GetData();
		graphicsSubmit.pWaitDstStageMask = frameData.GraphicsWaitDstMasks.GetData();
		graphicsSubmit.waitSemaphoreCount = static_cast<uint32>(frameData.GraphicsWaitSemaphores.GetCount());

		vkQueueSubmit(GraphicsQueue, 1, &graphicsSubmit, frameData.GraphicsFence);
		frameData.WaitFences.Add(frameData.GraphicsFence);
	}

	// Presents
	if (bExecutePresents)
	{
		VkPresentInfoKHR presents = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presents.pSwapchains = frameData.PresentSwapchains.GetData();
		presents.pImageIndices = frameData.PresentImageIndices.GetData();
		presents.swapchainCount = static_cast<uint32>(frameData.PresentSwapchains.GetCount());
		presents.pWaitSemaphores = &frameData.GraphicsSignalSemaphore;
		presents.waitSemaphoreCount = 1;

		vkQueuePresentKHR(PresentSupportQueue, &presents);
	}
}
