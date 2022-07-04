#pragma once

#include "nsVulkan.h"



enum class nsETextureFormat : uint8
{
	UNCOMPRESSED_RGBA = 0,
	UNCOMPRESSED_BGRA,
	UNCOMPRESSED_R,
	UNCOMPRESSED_RG,

	COMPRESSED_BC3_RGBA,
	COMPRESSED_BC4_R,
	COMPRESSED_BC5_RG,
	COMPRESSED_BC6_H,
	COMPRESSED_BC7_RGBA,

	RENDER_TARGET_BGRA,
	RENDER_TARGET_RGBA,

	DEPTH_STENCIL_D32,
	DEPTH_STENCIL_D24_S8,
};



struct nsTextureData
{
	struct Mip
	{
		int Width;
		int Height;
		nsTArray<uint8> Pixels;


		friend NS_INLINE void operator|(nsStream& stream, Mip& mip) noexcept
		{
			stream | mip.Width;
			stream | mip.Height;
			stream | mip.Pixels;
		}

	};


	int Width;
	int Height;
	nsETextureFormat Format;
	nsTArrayInline<Mip, NS_ENGINE_TEXTURE_MAX_MIP> Mips;
	bool bIsRenderTarget;
	bool bIsDepth;
	bool bIsStencil;


	friend NS_INLINE void operator|(nsStream& stream, nsTextureData& textureData) noexcept
	{
		stream | textureData.Width;
		stream | textureData.Height;
		stream | textureData.Format;
		stream | textureData.Mips;
	}

};



struct nsTextureResource
{
	nsVulkanTexture* Texture;
	nsVulkanTextureView* TextureView;
	nsTArrayInline<nsVulkanTextureView*, NS_ENGINE_TEXTURE_MAX_MIP> SubresourceViews;
};



class NS_ENGINE_API nsTextureManager
{
	NS_DECLARE_SINGLETON(nsTextureManager)

private:
	bool bInitialized;

	struct Frame
	{
		VkDescriptorSet TextureDescriptorSet;
		nsTArray<nsTextureID> TextureToBinds;
		nsTArray<nsTextureID> TextureToDestroys;
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;


	enum Flag
	{
		TextureFlag_None				= (0),
		TextureFlag_Dirty				= (1 << 0),
		TextureFlag_PendingLoad			= (1 << 1),
		TextureFlag_Loaded				= (1 << 2),
		TextureFlag_AlwaysLoaded		= (1 << 3),
		TextureFlag_PendingDestroy		= (1 << 4),
	};

	nsTArrayFreeList<nsName> TextureNames;
	nsTArrayFreeList<uint32> TextureFlags;
	nsTArrayFreeList<nsTextureData> TextureDatas;
	nsTArrayFreeList<nsTextureResource> TextureResources;

	nsTArray<VkImageMemoryBarrier> CachedImageMemoryBarriers;


public:
	void Initialize() noexcept;

private:
	nsTextureID AllocateTexture(nsName name) noexcept;
	void DeallocateTexture(nsTextureID texture) noexcept;

public:
	// Find valid texture (not marked as pending destroy) by name
	NS_NODISCARD nsTextureID FindTexture(const nsName& name) const noexcept;

	// Create texture 2D
	NS_NODISCARD nsTextureID CreateTexture2D_Empty(nsName name) noexcept;
	NS_NODISCARD nsTextureID CreateTexture2D(nsName name, nsETextureFormat format, int width, int height) noexcept;

	// Create render target
	NS_NODISCARD nsTextureID CreateRenderTarget(nsName name, nsETextureFormat format, int width, int height) noexcept;

	// Create depth-stencil
	NS_NODISCARD nsTextureID CreateDepthStencil(nsName name, nsETextureFormat format, int width, int height) noexcept;

	// Destroy texture
	void DestroyTexture(nsTextureID& texture) noexcept;

	// Update texture mip data. Only valid for texture2D
	void UpdateTextureMipData(nsTextureID texture, int mipIndex, const uint8* pixelData, int pixelDataSize) noexcept;

	// Get default texture 2D white (32x32)
	NS_NODISCARD nsTextureID GetDefaultTexture2D_White() noexcept;

	// Get default texture 2D black (32x32)
	NS_NODISCARD nsTextureID GetDefaultTexture2D_Black() noexcept;


	// Get current frame descriptor set (bindless). This must always bind at set=0
	NS_NODISCARD_INLINE VkDescriptorSet GetDescriptorSet() const noexcept
	{
		return FrameDatas[FrameIndex].TextureDescriptorSet;
	}


	NS_NODISCARD_INLINE bool IsTextureValid(nsTextureID texture) const noexcept
	{
		return texture.IsValid() && TextureFlags.IsValid(texture.Id);
	}


	NS_INLINE int AddTextureMipLevel(nsTextureID texture, int width, int height) noexcept
	{
		NS_Assert(IsTextureValid(texture));
		NS_Assert(width > 0 && height > 0);

		nsTextureData& data = TextureDatas[texture.Id];
		NS_AssertV(!data.bIsRenderTarget && !data.bIsDepth && !data.bIsStencil, "Cannot add mip level to render target/depth-stencil!");

		const int mipIndex = data.Mips.GetCount();

		nsTextureData::Mip& mip = data.Mips.Add();
		mip.Width = width;
		mip.Height = height;
		mip.Pixels.Clear();

		TextureResources[texture.Id].SubresourceViews.Add();

		return mipIndex;
	}


	NS_NODISCARD_INLINE const nsVulkanTextureView* GetTextureView(nsTextureID texture) const noexcept
	{
		NS_Assert(IsTextureValid(texture));
		return TextureResources[texture.Id].TextureView;
	}


	NS_NODISCARD_INLINE const nsVulkanTextureView* GetTextureSubresourceView(nsTextureID texture, int mipIndex) const noexcept
	{
		NS_Assert(IsTextureValid(texture));
		const nsTextureResource& resource = TextureResources[texture.Id];
		NS_AssertV(mipIndex >= 0 && mipIndex < resource.SubresourceViews.GetCount(), "Invalid mip index!");

		return resource.SubresourceViews[mipIndex];
	}


	NS_NODISCARD_INLINE nsPointInt GetTextureDimension(nsTextureID texture) const noexcept
	{
		NS_Assert(IsTextureValid(texture));

		const nsTextureData& data = TextureDatas[texture.Id];
		return nsPointInt(data.Width, data.Height);
	}


	NS_NODISCARD_INLINE nsTextureData& GetTextureData(nsTextureID texture) noexcept
	{
		NS_Assert(IsTextureValid(texture));

		nsTextureData& data = TextureDatas[texture.Id];
		NS_Assert(!data.bIsRenderTarget && !data.bIsDepth && !data.bIsStencil);

		return data;
	}


	NS_NODISCARD_INLINE const nsTextureData& GetTextureData(nsTextureID texture) const noexcept
	{
		NS_Assert(IsTextureValid(texture));

		const nsTextureData& data = TextureDatas[texture.Id];
		NS_Assert(!data.bIsRenderTarget && !data.bIsDepth && !data.bIsStencil);

		return data;
	}


	NS_NODISCARD_INLINE nsName GetTextureName(nsTextureID texture) const noexcept
	{
		NS_Assert(IsTextureValid(texture));
		return TextureNames[texture.Id];
	}


	void BeginFrame(int frameIndex) noexcept;
	void BindTextures(const nsTextureID* textures, int count) noexcept;
	void UpdateRenderResources() noexcept;

};
