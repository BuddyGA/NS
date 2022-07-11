#pragma once

#include "nsTextureTypes.h"



enum class nsEMaterialSurfaceDomain : uint8
{
	NONE = 0,
	PRIMITIVE_LINE,
	PRIMITIVE_LINE_2D,
	PRIMITIVE_MESH,
	PRIMITIVE_MESH_2D,
	MESH,
	GUI,
	POST_PROCESS,
	WIREFRAME
};



struct nsMaterialParameterLayout
{
	struct Scalar
	{
		float Value;
		int ScalarIndex;
	};

	struct Vector
	{
		nsVector4 Value;
		int ScalarIndex;
	};

private:
	nsTMap<nsName, Scalar> Scalars;
	nsTMap<nsName, Vector> Vectors;
	nsTMap<nsName, nsTextureID> Textures;


public:
	nsMaterialParameterLayout() noexcept = default;

	NS_INLINE void AddParameterScalar(nsName name, int scalarIndex, float defaultValue) noexcept
	{
		Scalar& s = Scalars[name];
		s.Value = defaultValue;
		s.ScalarIndex = scalarIndex;
	}

	NS_INLINE void AddParameterVector(nsName name, int scalarIndex, nsVector4 defaultValue) noexcept
	{
		Vector& v = Vectors[name];
		v.Value = defaultValue;
		v.ScalarIndex = scalarIndex;
	}

	NS_INLINE void AddParameterTexture(nsName name, nsTextureID defaultValue) noexcept
	{
		Textures[name] = defaultValue;
	}

	NS_NODISCARD_INLINE const nsTMap<nsName, Scalar>& GetScalars() const noexcept
	{
		return Scalars;
	}

	NS_NODISCARD_INLINE const nsTMap<nsName, Vector>& GetVectors() const noexcept
	{
		return Vectors;
	}

	NS_NODISCARD_INLINE const nsTMap<nsName, nsTextureID>& GetTextures() const noexcept
	{
		return Textures;
	}

	NS_NODISCARD_INLINE bool IsEmpty() const noexcept
	{
		return Scalars.GetCount() == 0 && Vectors.GetCount() == 0 && Textures.GetCount() == 0;
	}

};



struct nsMaterialParameterTable
{
	nsTMap<nsName, int> Scalars;
	nsTMap<nsName, int> Vectors;
	nsTMap<nsName, nsTextureID> Textures;
};



struct nsMaterialPipelineState
{
	const nsVulkanRenderPass* RenderPass;
	const nsVulkanShaderResourceLayout* ShaderResourceLayout;
	nsVulkanShader* VertexShader;
	nsVulkanShader* FragmentShader;
	nsEMaterialSurfaceDomain SurfaceDomain;
	nsEBlendMode BlendMode;
	VkCullModeFlagBits CullMode;
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
};



struct nsMaterialResource
{
	const nsVulkanShaderResourceLayout* ShaderResourceLayout;
	nsVulkanShaderPipeline* ShaderPipeline;
};



class NS_ENGINE_API nsMaterialManager
{
private:
	bool bInitialized;

	struct Frame
	{
		nsVulkanBuffer* StorageBuffer;
		nsTArrayFreeList<VkDescriptorSet> MaterialDescriptorSets;
		nsTArrayFreeList<uint32> MaterialDirtyFlags;
		nsTArrayInline<nsMaterialID, 32> MaterialToBinds;
		nsTArrayInline<nsMaterialID, 16> MaterialToDestroys;
	};

	Frame FrameDatas[NS_ENGINE_FRAME_BUFFERING];
	int FrameIndex;


	enum Flag
	{
		MaterialFlag_None				= (0),
		MaterialFlag_Default			= (1 << 0),
		MaterialFlag_Instance			= (1 << 1),
		MaterialFlag_PendingDestroy		= (1 << 2),
	};

	nsTArrayFreeList<nsName> MaterialNames;
	nsTArrayFreeList<uint32> MaterialFlags;
	nsTArrayFreeList<nsMaterialPipelineState> MaterialPipelineStates;
	nsTArrayFreeList<nsMaterialResource> MaterialResources;
	nsTArrayFreeList<nsMaterialParameterTable> MaterialParameterTables;
	nsTArray<float> MaterialUniformData;

	nsMaterialID DefaultPhong;
	nsMaterialID DefaultPhongChecker;
	nsMaterialID DefaultWireframe;
	nsMaterialID DefaultPrimitiveMesh;
	nsMaterialID DefaultPrimitiveMesh_IgnoreDepth;
	nsMaterialID DefaultPrimitiveLine;
	nsMaterialID DefaultPrimitiveLine_IgnoreDepth;
	nsMaterialID DefaultPrimitiveMesh2D;
	nsMaterialID DefaultPrimitiveLine2D;
	nsMaterialID DefaultGUI;
	nsMaterialID DefaultFont;
	nsMaterialID DefaultFullscreen;

	
public:
	void Initialize() noexcept;

private:
	NS_NODISCARD int Internal_CreateMaterial(nsName name) noexcept;

public:
	NS_NODISCARD const nsVulkanShaderResourceLayout* GetDefaultShaderResourceLayout_Forward() noexcept;
	NS_NODISCARD const nsVulkanShaderResourceLayout* GetDefaultShaderResourceLayout_Primitive() noexcept;
	NS_NODISCARD const nsVulkanShaderResourceLayout* GetDefaultShaderResourceLayout_GUI() noexcept;
	NS_NODISCARD const nsVulkanShaderResourceLayout* GetDefaultShaderResourceLayout_Wireframe() noexcept;

	NS_NODISCARD nsMaterialID FindMaterial(const nsName& name) const noexcept;
	NS_NODISCARD nsMaterialID CreateMaterial_Empty(nsName name) noexcept;
	NS_NODISCARD nsMaterialID CreateMaterial(nsName name, nsMaterialPipelineState pipelineState, const nsMaterialParameterLayout& parameterLayout) noexcept;
	NS_NODISCARD nsMaterialID CreateMaterialInstance(nsName name, nsMaterialID parentMaterial) noexcept;
	void DestroyMaterial(nsMaterialID& material) noexcept;
	void SetMaterialParameterScalarValue(nsMaterialID material, nsName paramName, float value) noexcept;
	void SetMaterialParameterVectorValue(nsMaterialID material, nsName paramName, nsVector4 value) noexcept;
	void SetMaterialParameterTextureValue(nsMaterialID material, nsName paramName, nsTextureID value) noexcept;
	NS_NODISCARD float GetMaterialParameterScalarValue(nsMaterialID material, nsName paramName) const noexcept;
	NS_NODISCARD nsVector4 GetMaterialParameterVectorValue(nsMaterialID material, nsName paramName) const noexcept;
	NS_NODISCARD nsTextureID GetMaterialParameterTextureValue(nsMaterialID material, nsName paramName) const noexcept;

	void BeginFrame(int frameIndex) noexcept;
	void BindMaterials(const nsMaterialID* materials, int count) noexcept;
	void UpdateRenderResources() noexcept;


	NS_NODISCARD_INLINE bool IsMaterialValid(nsMaterialID material) const noexcept
	{
		if (!material.IsValid())
		{
			return false;
		}

		if (!MaterialFlags.IsValid(material.Id))
		{
			return false;
		}

		return (MaterialFlags[material.Id] & MaterialFlag_PendingDestroy) == 0;
	}


	NS_NODISCARD_INLINE bool IsMaterialDefault(nsMaterialID material) const noexcept
	{
		NS_Assert(IsMaterialValid(material));
		return (MaterialFlags[material.Id] & MaterialFlag_Default);
	}


	NS_NODISCARD_INLINE bool IsMaterialInstance(nsMaterialID material) const noexcept
	{
		NS_Assert(IsMaterialValid(material));
		return (MaterialFlags[material.Id] & MaterialFlag_Instance);
	}


	NS_NODISCARD_INLINE nsName GetMaterialName(nsMaterialID material) const noexcept
	{
		NS_Assert(IsMaterialValid(material));
		return MaterialNames[material.Id];
	}


	NS_NODISCARD_INLINE const nsMaterialResource& GetMaterialResource(nsMaterialID material) const noexcept
	{
		NS_Assert(IsMaterialValid(material));
		return MaterialResources[material.Id];
	}


	NS_NODISCARD_INLINE VkDescriptorSet GetMaterialDescriptorSet(nsMaterialID material) const noexcept
	{
		NS_Assert(IsMaterialValid(material));
		return FrameDatas[FrameIndex].MaterialDescriptorSets[material.Id];
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_Phong() const noexcept
	{
		return DefaultPhong;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PhongChecker() const noexcept
	{
		return DefaultPhongChecker;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_Wireframe() const noexcept
	{
		return DefaultWireframe;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveMesh() const noexcept
	{
		return DefaultPrimitiveMesh;
	}

	NS_NODISCARD_INLINE const nsMaterialResource& GetDefaultMaterialResource_PrimitiveMesh() const noexcept
	{
		return MaterialResources[DefaultPrimitiveMesh.Id];
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveMesh_IgnoreDepth() const noexcept
	{
		return DefaultPrimitiveMesh_IgnoreDepth;
	}

	NS_NODISCARD_INLINE const nsMaterialResource& GetDefaultMaterialResource_PrimitiveMesh_IgnoreDepth() const noexcept
	{
		return MaterialResources[DefaultPrimitiveMesh_IgnoreDepth.Id];
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveLine() const noexcept
	{
		return DefaultPrimitiveLine;
	}

	NS_NODISCARD_INLINE const nsMaterialResource& GetDefaultMaterialResource_PrimitiveLine() const noexcept
	{
		return MaterialResources[DefaultPrimitiveLine.Id];
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveLine_IgnoreDepth() const noexcept
	{
		return DefaultPrimitiveLine_IgnoreDepth;
	}

	NS_NODISCARD_INLINE const nsMaterialResource& GetDefaultMaterialResource_PrimitiveLine_IgnoreDepth() const noexcept
	{
		return MaterialResources[DefaultPrimitiveLine_IgnoreDepth.Id];
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveMesh_2D() const noexcept
	{
		return DefaultPrimitiveMesh2D;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_PrimitiveLine_2D() const noexcept
	{
		return DefaultPrimitiveLine2D;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_GUI() const noexcept
	{
		return DefaultGUI;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_Font() const noexcept
	{
		return DefaultFont;
	}


	NS_NODISCARD_INLINE nsMaterialID GetDefaultMaterial_Fullscreen() const noexcept
	{
		return DefaultFullscreen;
	}


	NS_DECLARE_SINGLETON(nsMaterialManager)

};
