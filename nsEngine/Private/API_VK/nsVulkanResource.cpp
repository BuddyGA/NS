#include "nsVulkan.h"
#include "nsVulkanFunctions.h"



// ======================================================================================================================================== //
// VULKAN BUFFER
// ======================================================================================================================================== //
nsVulkanBuffer::nsVulkanBuffer(VmaMemoryUsage memoryUsage, VkBufferUsageFlags usageFlags, VkDeviceSize size, nsName debugName) noexcept
	: Info()
	, MemoryUsage(memoryUsage)
	, Buffer(VK_NULL_HANDLE)
	, Allocation(VK_NULL_HANDLE)
	, MapPtr(nullptr)
	, DebugName(debugName)
{
	Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	Info.usage = usageFlags;

	if (MemoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		Info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	Resize(size);
}


nsVulkanBuffer::~nsVulkanBuffer() noexcept
{
	if (Buffer)
	{
		NS_Assert(Allocation);
		NS_VK_PrintDestroyVulkanObject(Buffer);
		vmaDestroyBuffer(nsVulkan::GetVmaAllocator(), Buffer, Allocation);
	}
}


void nsVulkanBuffer::Resize(VkDeviceSize newSize) noexcept
{
	if (Info.size >= newSize)
	{
		return;
	}

	NS_LogDebug(VulkanLog, "Resize buffer [%s] [PrevSize: %u, NewSize: %u]", *DebugName, Info.size, newSize);

	Info.size = newSize;

	if (Buffer)
	{
		NS_Assert(Allocation);
		NS_VK_PrintDestroyVulkanObject(Buffer);
		vmaDestroyBuffer(nsVulkan::GetVmaAllocator(), Buffer, Allocation);
	}

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = MemoryUsage;

	vmaCreateBuffer(nsVulkan::GetVmaAllocator(), &Info, &allocationCreateInfo, &Buffer, &Allocation, nullptr);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_BUFFER, Buffer, DebugName);
}


void* nsVulkanBuffer::MapMemory() noexcept
{
	if (MapPtr == nullptr)
	{
		VmaAllocator allocator = nsVulkan::GetVmaAllocator();
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(allocator, Allocation, &allocationInfo);

		VkMemoryPropertyFlags memoryPropertyFlags;
		vmaGetMemoryTypeProperties(allocator, allocationInfo.memoryType, &memoryPropertyFlags);
		NS_ValidateV(memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, "Memory not mappable!");

		vmaMapMemory(allocator, Allocation, &MapPtr);
	}

	return MapPtr;
}


void nsVulkanBuffer::UnmapMemory() noexcept
{
	if (MapPtr)
	{
		vmaUnmapMemory(nsVulkan::GetVmaAllocator(), Allocation);
		MapPtr = nullptr;
	}
}


VkDeviceSize nsVulkanBuffer::GetMemorySize() noexcept
{
	return Info.size;
}



// ======================================================================================================================================== //
// VULKAN TEXTURE
// ======================================================================================================================================== //
nsVulkanTexture::nsVulkanTexture(VkImageUsageFlags usageFlags, VkImageAspectFlags aspectFlags, VkFormat format, uint32 width, uint32 height, uint32 mipLevels, nsName debugName) noexcept
	: Info()
	, Image(VK_NULL_HANDLE)
	, Allocation(VK_NULL_HANDLE)
	, AspectFlags(aspectFlags)
{
	Info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	Info.arrayLayers = 1;
	Info.extent = { width, height, 1 };
	Info.format = format;
	Info.imageType = VK_IMAGE_TYPE_2D;
	Info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Info.mipLevels = mipLevels;
	Info.samples = VK_SAMPLE_COUNT_1_BIT;
	Info.tiling = VK_IMAGE_TILING_OPTIMAL;
	Info.usage = usageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(nsVulkan::GetVmaAllocator(), &Info, &allocCreateInfo, &Image, &Allocation, nullptr);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_IMAGE, Image, debugName);
}


nsVulkanTexture::~nsVulkanTexture() noexcept
{
	NS_VK_PrintDestroyVulkanObject(Image);
	vmaDestroyImage(nsVulkan::GetVmaAllocator(), Image, Allocation);
}


VkDeviceSize nsVulkanTexture::GetMemorySize() const noexcept
{
	NS_Assert(Allocation);

	VmaAllocationInfo allocationInfo{};
	vmaGetAllocationInfo(nsVulkan::GetVmaAllocator(), Allocation, &allocationInfo);

	return allocationInfo.size;
}



// ======================================================================================================================================== //
// VULKAN TEXTURE VIEW
// ======================================================================================================================================== //
nsVulkanTextureView::nsVulkanTextureView(const nsVulkanTexture* texture, uint32 baseMip, uint32 mipCount, nsName debugName) noexcept
	: Info()
	, View(VK_NULL_HANDLE)
{
	Info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	Info.components.r = VK_COMPONENT_SWIZZLE_R;
	Info.components.g = VK_COMPONENT_SWIZZLE_G;
	Info.components.b = VK_COMPONENT_SWIZZLE_B;
	Info.components.a = VK_COMPONENT_SWIZZLE_A;
	Info.format = texture->GetFormat();
	Info.image = texture->GetVkImage();
	Info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	Info.subresourceRange.aspectMask = texture->GetAspectFlags();
	Info.subresourceRange.baseArrayLayer = 0;
	Info.subresourceRange.layerCount = 1;
	Info.subresourceRange.baseMipLevel = baseMip;
	Info.subresourceRange.levelCount = mipCount;

	vkCreateImageView(nsVulkan::GetVkDevice(), &Info, nullptr, &View);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, View, debugName);
}


nsVulkanTextureView::nsVulkanTextureView(VkImage swapchainImage, VkFormat format, nsName debugName) noexcept
	: Info()
	, View(VK_NULL_HANDLE)
{
	Info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	Info.components.r = VK_COMPONENT_SWIZZLE_R;
	Info.components.g = VK_COMPONENT_SWIZZLE_G;
	Info.components.b = VK_COMPONENT_SWIZZLE_B;
	Info.components.a = VK_COMPONENT_SWIZZLE_A;
	Info.format = format;
	Info.image = swapchainImage;
	Info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	Info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Info.subresourceRange.baseArrayLayer = 0;
	Info.subresourceRange.layerCount = 1;
	Info.subresourceRange.baseMipLevel = 0;
	Info.subresourceRange.levelCount = 1;

	vkCreateImageView(nsVulkan::GetVkDevice(), &Info, nullptr, &View);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, View, debugName);
}


nsVulkanTextureView::~nsVulkanTextureView() noexcept
{
	NS_VK_PrintDestroyVulkanObject(View);
	vkDestroyImageView(nsVulkan::GetVkDevice(), View, nullptr);
}
