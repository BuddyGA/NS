#include "nsTexture.h"
#include "API_VK/nsVulkanFunctions.h"


NS_ENGINE_DEFINE_HANDLE(nsTextureID);



static nsLogCategory TextureLog("nsTextureLog", nsELogVerbosity::LV_DEBUG);

static NS_INLINE VkFormat ns_ToVkFormat(nsETextureFormat format) noexcept
{
	switch (format)
	{
		case nsETextureFormat::UNCOMPRESSED_RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
		case nsETextureFormat::UNCOMPRESSED_BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		case nsETextureFormat::UNCOMPRESSED_R: return VK_FORMAT_R8_UNORM;
		case nsETextureFormat::UNCOMPRESSED_RG: return VK_FORMAT_R8G8_UNORM;
		case nsETextureFormat::COMPRESSED_BC3_RGBA: return VK_FORMAT_BC3_UNORM_BLOCK;
		case nsETextureFormat::COMPRESSED_BC4_R: return VK_FORMAT_BC4_UNORM_BLOCK;
		case nsETextureFormat::COMPRESSED_BC5_RG: return VK_FORMAT_BC5_UNORM_BLOCK;
		case nsETextureFormat::COMPRESSED_BC6_H: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case nsETextureFormat::COMPRESSED_BC7_RGBA: return VK_FORMAT_BC7_UNORM_BLOCK;
		case nsETextureFormat::RENDER_TARGET_BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		case nsETextureFormat::RENDER_TARGET_RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
		case nsETextureFormat::DEPTH_STENCIL_D32: return VK_FORMAT_D32_SFLOAT;
		case nsETextureFormat::DEPTH_STENCIL_D24_S8: return VK_FORMAT_D24_UNORM_S8_UINT;
		default: break;
	}

	return VK_FORMAT_UNDEFINED;
}



nsTextureManager::nsTextureManager() noexcept
	: bInitialized(false)
	, FrameDatas()
	, FrameIndex(0)
{
	TextureNames.Reserve(64);
	TextureFlags.Reserve(64);
	TextureDatas.Reserve(64);
	TextureResources.Reserve(64);

	CachedImageMemoryBarriers.Reserve(64);
}


void nsTextureManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(TextureLog, "Initialize texture manager");

	nsTextureID defaultTextures[2];
	defaultTextures[0] = GetDefaultTexture2D_White();
	defaultTextures[1] = GetDefaultTexture2D_Black();
	BindTextures(defaultTextures, 2);

	nsVulkanShaderResourceLayout* defaultTextureShaderResourceLayout = nsVulkanShaderResourceLayout::GetDefault_TextureShaderResourceLayout();

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.TextureDescriptorSet = nsVulkan::CreateDescriptorSetDynamicIndexing(defaultTextureShaderResourceLayout, 0, NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT);
	}

	bInitialized = true;
}


nsTextureID nsTextureManager::AllocateTexture(nsName name) noexcept
{
	NS_Validate_IsMainThread();

	const int nameId = TextureNames.Add(name);
	const int flagId = TextureFlags.Add(TextureFlag_None);
	const int dataId = TextureDatas.Add();
	const int resourceId = TextureResources.Add();
	NS_Assert(nameId == flagId && flagId == dataId && dataId == resourceId);

	nsTextureData& data = TextureDatas[dataId];
	data.Width = 0;
	data.Height = 0;
	data.Format = nsETextureFormat::UNCOMPRESSED_RGBA;
	data.bIsRenderTarget = false;
	data.bIsDepth = false;
	data.bIsStencil = false;
	data.Mips.Clear();

	nsTextureResource& resource = TextureResources[resourceId];
	NS_Assert(resource.Texture == nullptr);
	NS_Assert(resource.TextureView == nullptr);
	NS_Assert(resource.SubresourceViews.IsEmpty());

	return nsTextureID(nameId);
}


void nsTextureManager::DeallocateTexture(nsTextureID texture) noexcept
{
	NS_Validate_IsMainThread();

	const int id = texture.Id;
	NS_LogDebug(TextureLog, "Deallocate texture [%s]", *TextureNames[id]);
	
	const uint32& flags = TextureFlags[texture.Id];
	NS_Assert(flags & TextureFlag_PendingDestroy);

	TextureNames.RemoveAt(id);
	TextureFlags.RemoveAt(id);
	TextureDatas.RemoveAt(id);

	nsTextureResource& resource = TextureResources[id];

	for (int i = 0; i < resource.SubresourceViews.GetCount(); ++i)
	{
		nsVulkan::DestroyTextureView(resource.SubresourceViews[i]);
	}

	resource.SubresourceViews.Clear();
	nsVulkan::DestroyTextureView(resource.TextureView);
	nsVulkan::DestroyTexture(resource.Texture);

	TextureResources.RemoveAt(id);
}


nsTextureID nsTextureManager::FindTexture(const nsName& name) const noexcept
{
	for (auto it = TextureNames.CreateConstIterator(); it; ++it)
	{
		if (*it == name)
		{
			const int id = it.GetIndex();

			// Ignore pending destroy
			if (TextureFlags[id] & TextureFlag_PendingDestroy)
			{
				continue;
			}

			return id;
		}
	}

	return nsTextureID::INVALID;
}


nsTextureID nsTextureManager::CreateTexture2D_Empty(nsName name) noexcept
{
	if (FindTexture(name) != nsTextureID::INVALID)
	{
		NS_LogWarning(TextureLog, "Fail to create texture2D. Texture with name [%s] already exists!", *name);
		return nsTextureID();
	}

	return AllocateTexture(name);
}


nsTextureID nsTextureManager::CreateTexture2D(nsName name, nsETextureFormat format, int width, int height) noexcept
{
	if (FindTexture(name) != nsTextureID::INVALID)
	{
		NS_LogWarning(TextureLog, "Fail to create texture2D. Texture with name [%s] already exists!", *name);
		return nsTextureID();
	}

	const nsTextureID texture = AllocateTexture(name);

	nsTextureData& data = TextureDatas[texture.Id];
	data.Width = width;
	data.Height = height;
	data.Format = format;
	
	nsTextureData::Mip& mip0 = data.Mips.Add();
	mip0.Width = width;
	mip0.Height = height;
	mip0.Pixels.Clear();

	data.bIsRenderTarget = false;
	data.bIsDepth = false;
	data.bIsStencil = false;

	return texture;
}


nsTextureID nsTextureManager::CreateRenderTarget(nsName name, nsETextureFormat format, int width, int height) noexcept
{
	if (FindTexture(name) != nsTextureID::INVALID)
	{
		NS_LogWarning(TextureLog, "Fail to create texture render target. Texture with name [%s] already exists!", *name);
		return nsTextureID();
	}

	NS_AssertV(format == nsETextureFormat::RENDER_TARGET_BGRA || format == nsETextureFormat::RENDER_TARGET_RGBA, "Invalid format for render target!");

	const nsTextureID renderTarget = AllocateTexture(name);

	nsTextureData& data = TextureDatas[renderTarget.Id];
	data.Width = width;
	data.Height = height;
	data.Format = format;

	nsTextureData::Mip& mip0 = data.Mips.Add();
	mip0.Width = width;
	mip0.Height = height;
	mip0.Pixels.Clear();

	data.bIsRenderTarget = true;
	data.bIsDepth = false;
	data.bIsStencil = false;

	TextureFlags[renderTarget.Id] |= (TextureFlag_Loaded | TextureFlag_AlwaysLoaded);

	nsTextureResource& resource = TextureResources[renderTarget.Id];
	resource.Texture = nsVulkan::CreateTextureRenderTarget(ns_ToVkFormat(format), static_cast<uint32>(width), static_cast<uint32>(height), *name);
	resource.TextureView = nsVulkan::CreateTextureView(resource.Texture, 0, 1, nsName::Format("%s_view", *name));
	
	return renderTarget;
}


nsTextureID nsTextureManager::CreateDepthStencil(nsName name, nsETextureFormat format, int width, int height) noexcept
{
	if (FindTexture(name) != nsTextureID::INVALID)
	{
		NS_LogWarning(TextureLog, "Fail to create texture depth stencil. Texture with name [%s] already exists!", *name);
		return nsTextureID();
	}

	NS_AssertV(format == nsETextureFormat::DEPTH_STENCIL_D32 || format == nsETextureFormat::DEPTH_STENCIL_D24_S8, "Invalid format for depth stencil!");

	const bool bHasStencil = (format == nsETextureFormat::DEPTH_STENCIL_D24_S8);
	const nsTextureID depthStencil = AllocateTexture(name);

	nsTextureData& data = TextureDatas[depthStencil.Id];
	data.Width = width;
	data.Height = height;
	data.Format = format;

	nsTextureData::Mip& mip0 = data.Mips.Add();
	mip0.Width = width;
	mip0.Height = height;
	mip0.Pixels.Clear();

	data.bIsRenderTarget = false;
	data.bIsDepth = true;
	data.bIsStencil = bHasStencil;

	TextureFlags[depthStencil.Id] |= (TextureFlag_Loaded | TextureFlag_AlwaysLoaded);

	nsTextureResource& resource = TextureResources[depthStencil.Id];
	resource.Texture = nsVulkan::CreateTextureDepthStencil(ns_ToVkFormat(format), static_cast<uint32>(width), static_cast<uint32>(height), bHasStencil, *name);
	resource.TextureView = nsVulkan::CreateTextureView(resource.Texture, 0, 1, nsName::Format("%s_view", *name));

	return depthStencil;
}


void nsTextureManager::DestroyTexture(nsTextureID& texture) noexcept
{
	if (texture.IsValid())
	{
		NS_Assert(IsTextureValid(texture));

		uint32& flags = TextureFlags[texture.Id];

		if (flags & TextureFlag_PendingDestroy)
		{
			NS_LogWarning(TextureLog, "Ignoring destroy texture [%s] that has already marked pending destroy!", *TextureNames[texture.Id]);
		}
		else
		{
			flags |= TextureFlag_PendingDestroy;
			NS_LogDebug(TextureLog, "Marked texture [%s] as pending destroy", *TextureNames[texture.Id]);
		}

		FrameDatas[FrameIndex].TextureToDestroys.AddUnique(texture);
		texture = nsTextureID::INVALID;
	}
}


void nsTextureManager::UpdateTextureMipData(nsTextureID texture, int mipIndex, const uint8* pixelData, int pixelDataSize) noexcept
{
	NS_Assert(IsTextureValid(texture));

	nsTextureData& data = TextureDatas[texture.Id];
	NS_AssertV(mipIndex >= 0 && mipIndex < data.Mips.GetCount(), "Invalid mip index!");
	NS_Assert(pixelData);
	NS_Assert(pixelDataSize > 0);
	NS_AssertV(!data.bIsRenderTarget && !data.bIsDepth && !data.bIsStencil, "Cannot update mip data for render target/depth-stencil!");

	nsTextureData::Mip& mip = data.Mips[mipIndex];
	mip.Pixels.Clear();
	mip.Pixels.InsertAt(pixelData, pixelDataSize, 0);

	TextureFlags[texture.Id] |= TextureFlag_Dirty;
}


nsTextureID nsTextureManager::GetDefaultTexture2D_White() noexcept
{
	static nsTextureID _white;

	if (_white == nsTextureID::INVALID)
	{
		const int WIDTH = 32;
		const int HEIGHT = 32;
		const int PIXEL_SIZE = WIDTH * HEIGHT * 4;

		nsTArrayInline<uint8, PIXEL_SIZE> pixels;
		pixels.Resize(PIXEL_SIZE);
		nsPlatform::Memory_Set(pixels.GetData(), 255, PIXEL_SIZE);

		_white = CreateTexture2D(NS_ENGINE_TEXTURE_DEFAULT_WHITE_NAME, nsETextureFormat::UNCOMPRESSED_RGBA, WIDTH, HEIGHT);
		TextureFlags[_white.Id] |= TextureFlag_AlwaysLoaded;
		UpdateTextureMipData(_white, 0, pixels.GetData(), PIXEL_SIZE);
	}

	return _white;
}


nsTextureID nsTextureManager::GetDefaultTexture2D_Black() noexcept
{
	static nsTextureID _black;

	if (_black == nsTextureID::INVALID)
	{
		const int WIDTH = 32;
		const int HEIGHT = 32;
		const int COLOR_COUNT = WIDTH * HEIGHT;

		nsTArrayInline<nsColor, COLOR_COUNT> pixels;
		pixels.Resize(COLOR_COUNT);

		for (int i = 0; i < COLOR_COUNT; ++i)
		{
			pixels[i] = nsColor::BLACK;
		}

		_black = CreateTexture2D(NS_ENGINE_TEXTURE_DEFAULT_BLACK_NAME, nsETextureFormat::UNCOMPRESSED_RGBA, WIDTH, HEIGHT);
		TextureFlags[_black.Id] |= TextureFlag_AlwaysLoaded;
		UpdateTextureMipData(_black, 0, reinterpret_cast<uint8*>(pixels.GetData()), sizeof(nsColor) * COLOR_COUNT);
	}

	return _black;
}


void nsTextureManager::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];

	frame.TextureToBinds.Clear();

	// Cleanup pending destroy textures
	for (int i = 0; i < frame.TextureToDestroys.GetCount(); ++i)
	{
		DeallocateTexture(frame.TextureToDestroys[i]);
	}

	frame.TextureToDestroys.Clear();
}


void nsTextureManager::BindTextures(const nsTextureID* textures, int count) noexcept
{
	if (textures == nullptr || count <= 0)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];

	for (int i = 0; i < count; ++i)
	{
		NS_Assert(IsTextureValid(textures[i]));

		const int id = textures[i].Id;
		uint32& flags = TextureFlags[id];
		NS_AssertV(!(flags & TextureFlag_PendingDestroy), "Cannot bind texture that has marked pending destroy!");

		if (flags & TextureFlag_Dirty)
		{
			flags |= TextureFlag_PendingLoad;

			nsTextureResource& resource = TextureResources[id];
			const nsTextureData& data = TextureDatas[id];

			// For texture 2D we only create resource when needed (binding)
			if (resource.Texture == nullptr)
			{
				const nsName name = TextureNames[id];
				resource.Texture = nsVulkan::CreateTexture2D(ns_ToVkFormat(data.Format), static_cast<uint32>(data.Width), static_cast<uint32>(data.Height), static_cast<uint32>(data.Mips.GetCount()), *name);
				resource.TextureView = nsVulkan::CreateTextureView(resource.Texture, 0, static_cast<uint32>(data.Mips.GetCount()), nsName::Format("%s_view", *name));

				const int mipCount = data.Mips.GetCount();
				resource.SubresourceViews.Resize(mipCount);

				for (int m = 0; m < mipCount; ++m)
				{
					resource.SubresourceViews[m] = nsVulkan::CreateTextureView(resource.Texture, m, 1, nsName::Format("%s_view_%i", *name, m));
				}
			}
			// For render target/depth-stencil, resource must already created
			else
			{
				NS_Assert(resource.Texture);
				NS_Assert(resource.TextureView);
			}
		}

		frame.TextureToBinds.AddUnique(textures[i]);
	}
}


void nsTextureManager::UpdateRenderResources() noexcept
{
	Frame& frame = FrameDatas[FrameIndex];

	uint64 stagingBufferSize = 0;
	nsTArrayInline<int, 32> textureToLoads;

	if (frame.TextureToBinds.GetCount() > 0)
	{
		nsTArrayInline<VkWriteDescriptorSet, 32> TextureWriteDescriptorSets;
		nsTArrayInline<VkDescriptorImageInfo, 32> TextureDescriptorImageInfos;
		VkDescriptorSet textureDescriptorSet = frame.TextureDescriptorSet;

		for (int i = 0; i < frame.TextureToBinds.GetCount(); ++i)
		{
			const int id = frame.TextureToBinds[i].Id;
			const nsTextureResource& resource = TextureResources[id];

			VkDescriptorImageInfo& descriptorImageInfo = TextureDescriptorImageInfos.Add();
			descriptorImageInfo.imageView = resource.TextureView->GetVkImageView();
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorImageInfo.sampler = nsVulkan::GetDefaultSampler();

			VkWriteDescriptorSet& write = TextureWriteDescriptorSets.Add();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstSet = textureDescriptorSet;
			write.dstBinding = 0;
			write.descriptorCount = 1;
			write.dstArrayElement = static_cast<uint32>(id);
			write.pImageInfo = &descriptorImageInfo;

			// Should staging/load to GPU
			uint32& flags = TextureFlags[id];

			if (!(flags & TextureFlag_PendingLoad))
			{
				continue;
			}

			flags &= ~TextureFlag_Dirty;
			flags &= ~TextureFlag_PendingLoad;
			flags |= TextureFlag_Loaded;

			const nsTextureData& data = TextureDatas[id];

			// Ignore staging for render target/depth-stencil
			if (data.bIsRenderTarget || data.bIsDepth || data.bIsStencil)
			{
				continue;
			}

			for (int m = 0; m < data.Mips.GetCount(); ++m)
			{
				stagingBufferSize += data.Mips[m].Pixels.GetCount();
			}

			textureToLoads.Add(id);
			NS_LogDebug(TextureLog, "Load texture [%s] to GPU", *TextureNames[id]);
		}

		vkUpdateDescriptorSets(nsVulkan::GetVkDevice(), static_cast<uint32>(TextureWriteDescriptorSets.GetCount()), TextureWriteDescriptorSets.GetData(), 0, nullptr);
	}


	if (stagingBufferSize == 0)
	{
		return;
	}

	VkCommandBuffer commandBuffer = nsVulkan::AllocateGraphicsCommandBuffer();
	NS_VK_BeginCommand(commandBuffer);

	// Copy all mip datas to staging buffer
	nsVulkanBuffer* stagingBuffer = nsVulkan::CreateStagingBuffer(stagingBufferSize, "texture_staging_buffer");
	{
		uint8* stagingMap = reinterpret_cast<uint8*>(stagingBuffer->MapMemory());
		uint64 stagingOffset = 0;

		for (int i = 0; i < textureToLoads.GetCount(); ++i)
		{
			const int id = textureToLoads[i];
			const nsTextureData& data = TextureDatas[id];

			for (int m = 0; m < data.Mips.GetCount(); ++m)
			{
				const nsTextureData::Mip& mip = data.Mips[m];
				const uint64 pixelSize = static_cast<uint64>(mip.Pixels.GetCount());
				nsPlatform::Memory_Copy(stagingMap + stagingOffset, mip.Pixels.GetData(), pixelSize);
				NS_Assert(stagingOffset + pixelSize <= stagingBufferSize);
				stagingOffset += pixelSize;
			}
		}

		stagingBuffer->UnmapMemory();
	}


	// Transition texture layout to TRANSFER
	{
		CachedImageMemoryBarriers.Clear();

		for (int i = 0; i < textureToLoads.GetCount(); ++i)
		{
			const int id = textureToLoads[i];
			nsVulkan::SetupTextureBarrier_TransferDest(TextureResources[id].Texture, CachedImageMemoryBarriers);
		}

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr, 0, nullptr,
			static_cast<uint32>(CachedImageMemoryBarriers.GetCount()), CachedImageMemoryBarriers.GetData()
		);
	}


	// Copy staging buffer to texture regions
	{
		uint64 stagingOffset = 0;
		nsTArrayInline<VkBufferImageCopy, NS_ENGINE_TEXTURE_MAX_MIP> copyRegions;

		for (int i = 0; i < textureToLoads.GetCount(); ++i)
		{
			const int id = textureToLoads[i];
			const nsVulkanTexture* texture = TextureResources[id].Texture;
			const nsTextureData& data = TextureDatas[id];
			copyRegions.Clear();

			for (int m = 0; m < data.Mips.GetCount(); ++m)
			{
				const nsTextureData::Mip& mip = data.Mips[m];
				VkBufferImageCopy& copy = copyRegions.Add();
				copy.bufferOffset = stagingOffset;
				copy.bufferRowLength = 0;
				copy.bufferImageHeight = 0;
				copy.imageOffset = { 0, 0, 0 };
				copy.imageExtent = { static_cast<uint32>(mip.Width), static_cast<uint32>(mip.Height), 1 };
				copy.imageSubresource.aspectMask = texture->GetAspectFlags();
				copy.imageSubresource.baseArrayLayer = 0;
				copy.imageSubresource.layerCount = 1;
				copy.imageSubresource.mipLevel = m;

				const uint64 pixelSize = static_cast<uint64>(mip.Pixels.GetCount());
				NS_Assert(stagingOffset + pixelSize <= stagingBufferSize);
				stagingOffset += pixelSize;
			}

			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer->GetVkBuffer(), texture->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32>(copyRegions.GetCount()), copyRegions.GetData());
		}
	}


	// Transition texture layout to SHADER_READ
	{
		CachedImageMemoryBarriers.Clear();

		for (int i = 0; i < textureToLoads.GetCount(); ++i)
		{
			const int id = textureToLoads[i];
			nsVulkan::SetupTextureBarrier_ShaderRead(TextureResources[id].Texture, CachedImageMemoryBarriers);
		}

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
			0, nullptr, 0, nullptr,
			static_cast<uint32>(CachedImageMemoryBarriers.GetCount()), CachedImageMemoryBarriers.GetData()
		);
	}

	NS_VK_EndCommand(commandBuffer);

	nsVulkan::SubmitGraphicsCommandBuffer(&commandBuffer, 1);
	nsVulkan::DestroyBuffer(stagingBuffer);
}
