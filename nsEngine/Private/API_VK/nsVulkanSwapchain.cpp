#include "nsVulkan.h"
#include "nsVulkanFunctions.h"



nsVulkanSwapchain::nsVulkanSwapchain(nsWindowHandle windowHandle) noexcept
	: Surface(VK_NULL_HANDLE)
	, SurfaceCapabilities()
	, SurfaceFormat()
	, RequestedPresentMode(VK_PRESENT_MODE_FIFO_KHR)
	, PresentMode()
	, Swapchain(VK_NULL_HANDLE)
	, BackbufferIndex(0)
	, ImageAcquiredSemaphoreIndex(0)
	, LastAcquireImageResult(VK_SUCCESS)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = windowHandle;
	vkCreateWin32SurfaceKHR(nsVulkan::GetVkInstance(), &createInfo, nullptr, &Surface);
#endif // VK_USE_PLATFORM_WIN32_KHR

	if (!nsVulkan::CheckPresentSupport(Surface))
	{
		NS_LogError(VulkanLog, "Device does not support presentation!");
		NS_Abort();
	}

	const nsTArrayInline<VkSurfaceFormatKHR, 16> availableSurfaceFormats = nsVulkan::GetSurfaceFormats(Surface);
	VkFormat requiredFormat = VK_FORMAT_B8G8R8A8_UNORM;
	bool bFoundRequiredFormat = false;

	for (int i = 0; i < availableSurfaceFormats.GetCount(); ++i)
	{
		const VkSurfaceFormatKHR& check = availableSurfaceFormats[i];

		if (check.format == requiredFormat)
		{
			SurfaceFormat = check;
			bFoundRequiredFormat = true;
			break;
		}
	}

	if (!bFoundRequiredFormat)
	{
		NS_LogError(VulkanLog, "Required surface format [VK_FORMAT_B8G8R8A8_UNORM] not supported!");
		NS_Abort();
	}

	SetVsync(true);

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		VkSemaphore semaphore = nsVulkan::CreateObjectSemaphore();
		NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_SEMAPHORE, semaphore, nsName::Format("vk_image_acquired_semaphore_%i", i));

		ImageAcquiredSemaphores.Add(semaphore);
	}
}


nsVulkanSwapchain::~nsVulkanSwapchain() noexcept
{
	nsVulkan::WaitDeviceIdle();

	for (int i = 0; i < ImageAcquiredSemaphores.GetCount(); ++i)
	{
		nsVulkan::DestroyObjectSemaphore(ImageAcquiredSemaphores[i]);
	}

	for (int i = 0; i < BackbufferViews.GetCount(); ++i)
	{
		nsVulkan::DestroyTextureView(BackbufferViews[i]);
	}

	if (Swapchain)
	{
		vkDestroySwapchainKHR(nsVulkan::GetVkDevice(), Swapchain, nullptr);
	}

	if (Surface)
	{
		vkDestroySurfaceKHR(nsVulkan::GetVkInstance(), Surface, nullptr);
	}
}


void nsVulkanSwapchain::SetVsync(bool bEnabled) noexcept
{
	const VkPresentModeKHR vsyncMode = bEnabled ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
	const nsTArrayInline<VkPresentModeKHR, 16> availablePresentModes = nsVulkan::GetPresentModes(Surface);
	bool bFoundPresentMode = false;

	for (int i = 0; i < availablePresentModes.GetCount(); ++i)
	{
		if (availablePresentModes[i] == vsyncMode)
		{
			RequestedPresentMode = vsyncMode;
			bFoundPresentMode = true;
			break;
		}
	}

	if (!bFoundPresentMode)
	{
		NS_LogWarning(VulkanLog, "Requested present mode not found. Fallback to default present mode VK_PRESENT_MODE_FIFO_KHR!");
		RequestedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	}
}


void nsVulkanSwapchain::Resize() noexcept
{
	const uint32 prevWidth = SurfaceCapabilities.currentExtent.width;
	const uint32 prevHeight = SurfaceCapabilities.currentExtent.height;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(nsVulkan::GetVkPhysicalDevice(), Surface, &SurfaceCapabilities);
	const uint32 width = SurfaceCapabilities.currentExtent.width;
	const uint32 height = SurfaceCapabilities.currentExtent.height;

	if (width <= 1 || height <= 1)
	{
		return;
	}

	const bool bShouldResize = (Swapchain == VK_NULL_HANDLE) || (prevWidth != width) || (prevHeight != height) || (RequestedPresentMode != PresentMode) || (LastAcquireImageResult != VK_SUCCESS);

	if (!bShouldResize)
	{
		return;
	}

	if (Swapchain)
	{
		nsVulkan::WaitDeviceIdle();

		for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
		{
			nsVulkan::DestroyTextureView(BackbufferViews[i]);
		}

		BackbufferViews.Clear();
	}

	PresentMode = RequestedPresentMode;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.flags = 0;
	createInfo.pNext = nullptr;
	createInfo.surface = Surface;
	createInfo.clipped = VK_FALSE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = PresentMode;
	createInfo.preTransform = SurfaceCapabilities.currentTransform;
	createInfo.minImageCount = NS_ENGINE_FRAME_BUFFERING;
	createInfo.imageArrayLayers = 1;
	createInfo.imageFormat = SurfaceFormat.format;
	createInfo.imageColorSpace = SurfaceFormat.colorSpace;
	createInfo.imageExtent = SurfaceCapabilities.currentExtent;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.oldSwapchain = Swapchain;

	VkDevice vkDevice = nsVulkan::GetVkDevice();
	vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &Swapchain);

	if (createInfo.oldSwapchain)
	{
		vkDestroySwapchainKHR(vkDevice, createInfo.oldSwapchain, nullptr);
	}

	VkImage swapchainImages[8];
	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(vkDevice, Swapchain, &swapchainImageCount, nullptr);
	NS_Assert(swapchainImageCount <= 8);
	vkGetSwapchainImagesKHR(vkDevice, Swapchain, &swapchainImageCount, swapchainImages);
	NS_Validate(swapchainImageCount == NS_ENGINE_FRAME_BUFFERING);

	for (uint32 i = 0; i < swapchainImageCount; ++i)
	{
		BackbufferViews.Add(nsVulkan::CreateBackbufferView(swapchainImages[i], SurfaceFormat.format, nsName::Format("backbuffer_view_%i", i)));
	}
}


bool nsVulkanSwapchain::AcquireNextBackbuffer(int& outBackbufferIndex) noexcept
{
	ImageAcquiredSemaphoreIndex = (ImageAcquiredSemaphoreIndex + 1) % NS_ENGINE_FRAME_BUFFERING;
	LastAcquireImageResult = vkAcquireNextImageKHR(nsVulkan::GetVkDevice(), Swapchain, UINT64_MAX, ImageAcquiredSemaphores[ImageAcquiredSemaphoreIndex], VK_NULL_HANDLE, &BackbufferIndex);
	outBackbufferIndex = BackbufferIndex;

	return LastAcquireImageResult == VK_SUCCESS;
}


void nsVulkanSwapchain::Present() noexcept
{
	nsVulkan::SubmitPresent(Swapchain, BackbufferIndex, ImageAcquiredSemaphores[ImageAcquiredSemaphoreIndex]);
}
