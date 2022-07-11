#pragma once

#include "nsVulkan.h"



NS_ENGINE_DECLARE_HANDLE(nsTextureID, nsTextureManager)



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
