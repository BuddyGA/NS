#include "nsMaterial.h"
#include "nsShaderManager.h"
#include "nsTexture.h"
#include "API_VK/nsVulkanFunctions.h"


NS_ENGINE_DEFINE_HANDLE(nsMaterialID);



static nsLogCategory MaterialLog("nsMaterialLog", nsELogVerbosity::LV_DEBUG);

static void ns_GetVertexAttributeBindings(nsEMaterialSurfaceDomain surfaceDomain, nsTArrayInline<VkVertexInputBindingDescription, 4>& outVertexBindings, nsTArrayInline<VkVertexInputAttributeDescription, 8>& outVertexAttributes) noexcept
{
	outVertexBindings.Clear();
	outVertexAttributes.Clear();

	switch (surfaceDomain)
	{
		case nsEMaterialSurfaceDomain::PRIMITIVE_LINE:
		case nsEMaterialSurfaceDomain::PRIMITIVE_MESH:
		{
			outVertexBindings.Add({ 0, sizeof(nsVertexPrimitive), VK_VERTEX_INPUT_RATE_VERTEX });

			outVertexAttributes.Add({ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 }); // Position
			outVertexAttributes.Add({ 1, 0, VK_FORMAT_R8G8B8A8_UNORM, 16 }); // Color

			break;
		}

		case nsEMaterialSurfaceDomain::SURFACE:
		{
			outVertexBindings.Add({ 0, sizeof(nsVertexMeshPosition), VK_VERTEX_INPUT_RATE_VERTEX });
			outVertexBindings.Add({ 1, sizeof(nsVertexMeshAttribute), VK_VERTEX_INPUT_RATE_VERTEX });

			outVertexAttributes.Add({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }); // Position
			outVertexAttributes.Add({ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 }); // Normal;
			outVertexAttributes.Add({ 2, 1, VK_FORMAT_R32G32B32_SFLOAT, 12 }); // Tangent;
			outVertexAttributes.Add({ 3, 1, VK_FORMAT_R32G32_SFLOAT, 24 }); // TexCoord;

			break;
		}

		case nsEMaterialSurfaceDomain::PRIMITIVE_LINE_2D:
		case nsEMaterialSurfaceDomain::PRIMITIVE_MESH_2D:
		{
			outVertexBindings.Add({ 0, sizeof(nsVertexPrimitive2D), VK_VERTEX_INPUT_RATE_VERTEX });

			outVertexAttributes.Add({ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }); // Position
			outVertexAttributes.Add({ 1, 0, VK_FORMAT_R8G8B8A8_UNORM, 8 }); // Color

			break;
		}

		case nsEMaterialSurfaceDomain::GUI:
		{
			outVertexBindings.Add({ 0, sizeof(nsVertexGUI), VK_VERTEX_INPUT_RATE_VERTEX });

			outVertexAttributes.Add({ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }); // Position
			outVertexAttributes.Add({ 1, 0, VK_FORMAT_R32G32_SFLOAT, 8 }); // TexCoord
			outVertexAttributes.Add({ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, 16 }); // Color

			break;
		}

		case nsEMaterialSurfaceDomain::WIREFRAME:
		{
			outVertexBindings.Add({ 0, sizeof(nsVertexMeshPosition), VK_VERTEX_INPUT_RATE_VERTEX });

			outVertexAttributes.Add({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }); // Position

			break;
		}

		default:
			break;
	}
}



nsMaterialManager::nsMaterialManager() noexcept
	: bInitialized(false)
	, FrameDatas()
	, FrameIndex(0)
{
	MaterialNames.Reserve(256);
	MaterialFlags.Reserve(256);
	MaterialPipelineStates.Reserve(256);
	MaterialResources.Reserve(256);
	MaterialParameterTables.Reserve(256);
	MaterialUniformData.Reserve(NS_ENGINE_MATERIAL_UNIFORM_SIZE * 256);
}


void nsMaterialManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	NS_LogInfo(MaterialLog, "Initialize material manager");

	// Initialize default materials
	NS_LogInfo(MaterialLog, "Creating default materials...");

	const nsTextureID whiteTexture = nsTextureManager::Get().GetDefaultTexture2D_White();
	const nsTextureID blackTexture = nsTextureManager::Get().GetDefaultTexture2D_Black();
	nsShaderManager& shaderManager = nsShaderManager::Get();

	// Depth pass
	{
		// TODO: 
	}


	// Shadow pass
	{
		// TODO:
	}


	// Forward pass
	{
		nsMaterialPipelineState forwardState{};
		forwardState.RenderPass = nsVulkanRenderPass::GetDefault_Forward();
		forwardState.ShaderResourceLayout = GetDefaultShaderResourceLayout_Forward();
		forwardState.VertexShader = shaderManager.GetShaderModule("mesh");
		forwardState.SurfaceDomain = nsEMaterialSurfaceDomain::SURFACE;
		forwardState.BlendMode = nsEBlendMode::NONE;
		forwardState.CullMode = VK_CULL_MODE_BACK_BIT;
		forwardState.bEnableDepthTest = true;
		forwardState.bEnableDepthWrite = true;
		
		// Default phong
		{
			forwardState.FragmentShader = shaderManager.GetShaderModule("forward_phong");

			nsMaterialParameterLayout paramLayout{};
			paramLayout.AddParameterVector("base_color", 0, nsVector4(1.0f));
			paramLayout.AddParameterScalar("shininess", 4, 32.0f);
			paramLayout.AddParameterTexture("diffuse", whiteTexture);
			paramLayout.AddParameterTexture("specular", whiteTexture);

			DefaultPhong = CreateMaterial(NS_ENGINE_MATERIAL_DEFAULT_PHONG_NAME, forwardState, paramLayout);
		}

		// Default phong-checker
		{
			forwardState.FragmentShader = shaderManager.GetShaderModule("forward_phong_checker");

			DefaultPhongChecker = CreateMaterial(NS_ENGINE_MATERIAL_DEFAULT_PHONG_CHECKER_NAME, forwardState, nsMaterialParameterLayout());
		}

		// Default wireframe
		{
			forwardState.ShaderResourceLayout = GetDefaultShaderResourceLayout_Wireframe();
			forwardState.VertexShader = shaderManager.GetShaderModule("mesh_wireframe");
			forwardState.FragmentShader = shaderManager.GetShaderModule("color");
			forwardState.SurfaceDomain = nsEMaterialSurfaceDomain::WIREFRAME;
			forwardState.CullMode = VK_CULL_MODE_NONE;

			DefaultWireframe = CreateMaterial("mat_default_wireframe", forwardState, nsMaterialParameterLayout());
		}

		// Default primitive (mesh, line)
		{
			forwardState.ShaderResourceLayout = GetDefaultShaderResourceLayout_Primitive();
			forwardState.VertexShader = shaderManager.GetShaderModule("primitive");
			forwardState.FragmentShader = shaderManager.GetShaderModule("color");

			// mesh
			forwardState.SurfaceDomain = nsEMaterialSurfaceDomain::PRIMITIVE_MESH;
			forwardState.CullMode = VK_CULL_MODE_BACK_BIT;
			DefaultPrimitiveMesh = CreateMaterial("mat_default_primitive_mesh", forwardState, nsMaterialParameterLayout());

			// line
			forwardState.SurfaceDomain = nsEMaterialSurfaceDomain::PRIMITIVE_LINE;
			forwardState.CullMode = VK_CULL_MODE_NONE;
			DefaultPrimitiveLine = CreateMaterial("mat_default_primitive_line", forwardState, nsMaterialParameterLayout());
		}
	}


	// Final pass
	{
		nsMaterialPipelineState finalState{};
		finalState.RenderPass = nsVulkanRenderPass::GetDefault_Final();
		finalState.ShaderResourceLayout = GetDefaultShaderResourceLayout_GUI();
		finalState.CullMode = VK_CULL_MODE_BACK_BIT;


		// Default primitive 2D (mesh, line)
		{
			finalState.VertexShader = shaderManager.GetShaderModule("primitive_2d");
			finalState.FragmentShader = shaderManager.GetShaderModule("color");

			// mesh
			finalState.SurfaceDomain = nsEMaterialSurfaceDomain::PRIMITIVE_MESH_2D;
			DefaultPrimitiveMesh2D = CreateMaterial("mat_default_primitive_mesh_2d", finalState, nsMaterialParameterLayout());

			// line
			finalState.SurfaceDomain = nsEMaterialSurfaceDomain::PRIMITIVE_LINE_2D;
			DefaultPrimitiveLine2D = CreateMaterial("mat_default_primitive_line_2d", finalState, nsMaterialParameterLayout());
		}
		

		// Default GUI (surface, font)
		{
			nsMaterialParameterLayout paramLayout{};
			paramLayout.AddParameterTexture("texture", whiteTexture);

			finalState.VertexShader = shaderManager.GetShaderModule("gui");
			finalState.SurfaceDomain = nsEMaterialSurfaceDomain::GUI;
			finalState.BlendMode = nsEBlendMode::TRANSPARENCY;

			finalState.FragmentShader = shaderManager.GetShaderModule("texture_color");
			DefaultGUI = CreateMaterial(NS_ENGINE_MATERIAL_DEFAULT_GUI_NAME, finalState, paramLayout);

			finalState.FragmentShader = shaderManager.GetShaderModule("font");
			DefaultFont = CreateMaterial(NS_ENGINE_MATERIAL_DEFAULT_FONT_NAME, finalState, paramLayout);
		}


		// Default fullscreen
		{
			nsVulkanShaderResourceLayout* fullscreenShaderResourceLayout = nsVulkan::CreateShaderResourceLayout("srl_default_fullscreen");
			fullscreenShaderResourceLayout->AddDescriptorBindingTexture(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0, NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT, true);
			fullscreenShaderResourceLayout->AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int));
			fullscreenShaderResourceLayout->Build();

			finalState.ShaderResourceLayout = fullscreenShaderResourceLayout;
			finalState.VertexShader = shaderManager.GetShaderModule("fullscreen");
			finalState.FragmentShader = shaderManager.GetShaderModule("texture");
			finalState.SurfaceDomain = nsEMaterialSurfaceDomain::POST_PROCESS;
			finalState.BlendMode = nsEBlendMode::NONE;

			nsMaterialParameterLayout paramLayout{};
			paramLayout.AddParameterTexture("texture", blackTexture);

			DefaultFullscreen = CreateMaterial("mat_default_fullscreen", finalState, paramLayout);
		}
	}


	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		frame.StorageBuffer = nsVulkan::CreateStorageBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, NS_ENGINE_MATERIAL_UNIFORM_SIZE * 64, "material_storage_buffer");
	}

	bInitialized = true;
}


int nsMaterialManager::Internal_CreateMaterial(nsName name) noexcept
{
	const int nameId = MaterialNames.Add(name);
	const int flagId = MaterialFlags.Add(MaterialFlag_Default);
	const int stateId = MaterialPipelineStates.Add();
	const int resId = MaterialResources.Add();
	const int paramId = MaterialParameterTables.Add();
	NS_Assert(nameId == flagId && flagId == stateId && stateId == resId && resId == paramId);

	for (int i = 0; i < NS_ENGINE_FRAME_BUFFERING; ++i)
	{
		Frame& frame = FrameDatas[i];
		const int descriptorId = frame.MaterialDescriptorSets.Add(VK_NULL_HANDLE);
		const int dirtyId = frame.MaterialDirtyFlags.Add(1);
		NS_Assert(nameId == descriptorId && descriptorId == dirtyId);
	}

	return nameId;
}


const nsVulkanShaderResourceLayout* nsMaterialManager::GetDefaultShaderResourceLayout_Forward() noexcept
{
	static nsVulkanShaderResourceLayout* _default;

	if (_default == nullptr)
	{
		_default = nsVulkan::CreateShaderResourceLayout("srl_default_forward");

		// Texture (Dynamic Indexing) [Set = 0, Binding = 0]
		_default->AddDescriptorBindingTexture(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0, NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT, true);

		// Environment [Set = 1, Binding = 0]
		_default->AddDescriptorBindingUniform(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0, 1, false, false);

		// TODO: Light UBO [Set = 2, Binding = 0]

		// Camera view UBO [Set = 2, Binding = 0]
		_default->AddDescriptorBindingUniform(VK_SHADER_STAGE_VERTEX_BIT, 2, 0, 1, false, false);

		// Material UBO [Set = 3, Binding = 0]
		_default->AddDescriptorBindingStorage(VK_SHADER_STAGE_FRAGMENT_BIT, 3, 0, 1, false, false);

		// Push constant (world transform)
		_default->AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(nsMatrix4));

		_default->Build();
	}
	
	return _default;
}


const nsVulkanShaderResourceLayout* nsMaterialManager::GetDefaultShaderResourceLayout_Primitive() noexcept
{
	static nsVulkanShaderResourceLayout* _default;

	if (_default == nullptr)
	{
		_default = nsVulkan::CreateShaderResourceLayout("srl_default_primitive");

		// Camera view UBO [Set = 0, Binding = 0]
		_default->AddDescriptorBindingUniform(VK_SHADER_STAGE_VERTEX_BIT, 0, 0, 1, false, false);

		_default->Build();
	}

	return _default;
}


const nsVulkanShaderResourceLayout* nsMaterialManager::GetDefaultShaderResourceLayout_GUI() noexcept
{
	static nsVulkanShaderResourceLayout* _default;

	if (_default == nullptr)
	{
		_default = nsVulkan::CreateShaderResourceLayout("srl_default_gui");

		// Texture (Dynamic Indexing) [Set = 0, Binding = 0]
		_default->AddDescriptorBindingTexture(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0, NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT, true);

		// Push constant (vec2: Scale, vec2: Translate)
		_default->AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(nsVector4));

		// Push constant (int: TextureId)
		_default->AddPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(nsVector4), sizeof(int));

		_default->Build();
	}

	return _default;
}


const nsVulkanShaderResourceLayout* nsMaterialManager::GetDefaultShaderResourceLayout_Wireframe() noexcept
{
	static nsVulkanShaderResourceLayout* _default;

	if (_default == nullptr)
	{
		_default = nsVulkan::CreateShaderResourceLayout("srl_default_wireframe");

		// Camera view UBO [Set = 0, Binding = 0]
		_default->AddDescriptorBindingUniform(VK_SHADER_STAGE_VERTEX_BIT, 0, 0, 1, false, false);

		// Push constant (world transform)
		_default->AddPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(nsMatrix4));

		_default->Build();
	}

	return _default;
}


nsMaterialID nsMaterialManager::FindMaterial(const nsName& name) const noexcept
{
	for (auto it = MaterialNames.CreateConstIterator(); it; ++it)
	{
		if ((*it) == name)
		{
			return nsMaterialID(it.GetIndex());
		}
	}

	return nsMaterialID::INVALID;
}


nsMaterialID nsMaterialManager::CreateMaterial_Empty(nsName name) noexcept
{
	if (FindMaterial(name) != nsMaterialID::INVALID)
	{
		NS_LogWarning(MaterialLog, "Material with name [%s] already exists!", *name);
		return nsMaterialID::INVALID;
	}

	return Internal_CreateMaterial(name);
}


nsMaterialID nsMaterialManager::CreateMaterial(nsName name, nsMaterialPipelineState pipelineState, const nsMaterialParameterLayout& parameterLayout) noexcept
{
	if (FindMaterial(name) != nsMaterialID::INVALID)
	{
		NS_LogWarning(MaterialLog, "Material with name [%s] already exists!", *name);
		return nsMaterialID::INVALID;
	}

	const int id = Internal_CreateMaterial(name);
	MaterialPipelineStates[id] = pipelineState;

	const nsEMaterialSurfaceDomain surfaceDomain = pipelineState.SurfaceDomain;

	// Setup shader resource layout and pipeline
	nsMaterialResource& resource = MaterialResources[id];
	//resource.ShaderResourceLayout = nsVulkan::CreateShaderResourceLayout(pipelineState.VertexShader, pipelineState.FragmentShader, nsName::Format("%s.sh_res_lyt", *name));
	resource.ShaderResourceLayout = pipelineState.ShaderResourceLayout;
	
	nsVulkanPipelineState pso{};
	pso.RenderPass = pipelineState.RenderPass;
	pso.ShaderResourceLayout = resource.ShaderResourceLayout;
	ns_GetVertexAttributeBindings(surfaceDomain, pso.VertexInputBindings, pso.VertexInputAttributes);

	pso.VertexShader = pipelineState.VertexShader;
	pso.FragmentShader = pipelineState.FragmentShader;
	pso.FrontFace = VK_FRONT_FACE_CLOCKWISE;
	pso.CullMode = pipelineState.CullMode;
	pso.BlendMode = pipelineState.BlendMode;
	pso.DepthBiasConstant = pipelineState.DepthBiasConstant;
	pso.DepthBiasSlope = pipelineState.DepthBiasSlope;
	pso.DepthBiasClamp = pipelineState.DepthBiasClamp;
	pso.MinDepthBound = pipelineState.MinDepthBound;
	pso.MaxDepthBound = pipelineState.MaxDepthBound;
	pso.bEnableDepthTest = pipelineState.bEnableDepthTest;
	pso.bEnableDepthWrite = pipelineState.bEnableDepthWrite;
	pso.bEnableDepthBias = pipelineState.bEnableDepthBias;
	pso.bEnableDepthClamp = pipelineState.bEnableDepthClamp;
	pso.bEnableDepthBound = pipelineState.bEnableDepthBound;

	switch (surfaceDomain)
	{
		case nsEMaterialSurfaceDomain::PRIMITIVE_LINE:
		case nsEMaterialSurfaceDomain::PRIMITIVE_LINE_2D:
		{
			pso.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			pso.PolygonMode = VK_POLYGON_MODE_LINE;
			break;
		}

		case nsEMaterialSurfaceDomain::PRIMITIVE_MESH:
		case nsEMaterialSurfaceDomain::PRIMITIVE_MESH_2D:
		case nsEMaterialSurfaceDomain::SURFACE:
		case nsEMaterialSurfaceDomain::GUI:
		case nsEMaterialSurfaceDomain::POST_PROCESS:
		{
			pso.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pso.PolygonMode = VK_POLYGON_MODE_FILL;
			break;
		}

		case nsEMaterialSurfaceDomain::WIREFRAME:
		{
			pso.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pso.PolygonMode = VK_POLYGON_MODE_LINE;
			break;
		}

		default:
			break;
	}

	resource.ShaderPipeline = nsVulkan::CreateShaderPipeline(pso, nsName::Format("%s.pso", *name));


	// Setup parameters
	MaterialUniformData.Resize((id + 1) * NS_ENGINE_MATERIAL_UNIFORM_SIZE);
	nsMaterialParameterTable& parameterTable = MaterialParameterTables[id];

	if (!parameterLayout.IsEmpty())
	{
		const auto& scalarTable = parameterLayout.GetScalars();
		for (int i = 0; i < scalarTable.GetCount(); ++i)
		{
			const nsName& name = scalarTable.GetKeyByIndex(i);
			const auto& param = scalarTable.GetValueByIndex(i);
			parameterTable.Scalars.Add(name, param.ScalarIndex);
			nsPlatform::Memory_Copy(&MaterialUniformData[id] + param.ScalarIndex, &param.Value, sizeof(float));
		}

		const auto& vectorTable = parameterLayout.GetVectors();
		for (int i = 0; i < vectorTable.GetCount(); ++i)
		{
			const nsName& name = vectorTable.GetKeyByIndex(i);
			const auto& param = vectorTable.GetValueByIndex(i);
			parameterTable.Vectors.Add(name, param.ScalarIndex);
			nsPlatform::Memory_Copy(&MaterialUniformData[id] + param.ScalarIndex, &param.Value, sizeof(nsVector4));
		}

		const auto& textureTable = parameterLayout.GetTextures();
		for (int i = 0; i < textureTable.GetCount(); ++i)
		{
			parameterTable.Textures.Add(textureTable.GetKeyByIndex(i), textureTable.GetValueByIndex(i));
		}
	}


	NS_LogInfo(MaterialLog, "Create material default [%s]", *name);

	return nsMaterialID(id);
}


nsMaterialID nsMaterialManager::CreateMaterialInstance(nsName name, nsMaterialID parentMaterial) noexcept
{
	NS_Assert(IsMaterialValid(parentMaterial));

	const int id = Internal_CreateMaterial(name);
	MaterialFlags[id] = MaterialFlag_Instance;
	MaterialPipelineStates[id] = MaterialPipelineStates[parentMaterial.Id];
	MaterialResources[id] = MaterialResources[parentMaterial.Id];
	MaterialParameterTables[id] = MaterialParameterTables[parentMaterial.Id];

	NS_LogInfo(MaterialLog, "Create material instance [%s]", *name);

	return nsMaterialID(id);
}


void nsMaterialManager::DestroyMaterial(nsMaterialID& material) noexcept
{
	NS_ValidateV(0, "Not implemented yet!");
}


void nsMaterialManager::SetMaterialParameterScalarValue(nsMaterialID material, nsName paramName, float value) noexcept
{
	NS_ValidateV(0, "Not implemented yet!");
}


void nsMaterialManager::SetMaterialParameterVectorValue(nsMaterialID material, nsName paramName, nsVector4 value) noexcept
{
	NS_ValidateV(0, "Not implemented yet!");
}


void nsMaterialManager::SetMaterialParameterTextureValue(nsMaterialID material, nsName paramName, nsTextureID value) noexcept
{
	NS_Assert(IsMaterialValid(material));

	nsMaterialParameterTable& paramTable = MaterialParameterTables[material.Id];
	nsTextureID* currentValue = paramTable.Textures.GetValueByKey(paramName);

	if (currentValue)
	{
		*currentValue = value;
	}
	else
	{
		NS_LogWarning(MaterialLog, "Fail to set material parameter texture value. Material [%s] texture parameter [%s] not found!", *MaterialNames[material.Id], *paramName);
	}
}


float nsMaterialManager::GetMaterialParameterScalarValue(nsMaterialID material, nsName paramName) const noexcept
{
	return 0.0f;
}


nsVector4 nsMaterialManager::GetMaterialParameterVectorValue(nsMaterialID material, nsName paramName) const noexcept
{
	return nsVector4();
}


nsTextureID nsMaterialManager::GetMaterialParameterTextureValue(nsMaterialID material, nsName paramName) const noexcept
{
	NS_Assert(IsMaterialValid(material));

	return MaterialParameterTables[material.Id].Textures[paramName];
}


void nsMaterialManager::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	Frame& frame = FrameDatas[FrameIndex];
	frame.MaterialToBinds.Clear();
	frame.MaterialToDestroys.Clear();
}


void nsMaterialManager::BindMaterials(const nsMaterialID* materials, int count) noexcept
{
	if (materials == nullptr || count <= 0)
	{
		return;
	}

	Frame& frame = FrameDatas[FrameIndex];
	nsTArrayInline<nsTextureID, 32> bindTextures;

	for (int i = 0; i < count; ++i)
	{
		NS_Assert(IsMaterialValid(materials[i]));

		const int id = materials[i].Id;
		const nsMaterialParameterTable& paramTable = MaterialParameterTables[id];

		for (int j = 0; j < paramTable.Textures.GetCount(); ++j)
		{
			bindTextures.Add(paramTable.Textures.GetValueByIndex(j));
		}

		frame.MaterialToBinds.AddUnique(materials[i]);
	}

	nsTextureManager::Get().BindTextures(bindTextures.GetData(), bindTextures.GetCount());
}


void nsMaterialManager::Update() noexcept
{
	Frame& frame = FrameDatas[FrameIndex];

	if (frame.MaterialToBinds.GetCount() > 0)
	{
		const uint64 newStorageBufferSize = NS_ENGINE_MATERIAL_UNIFORM_SIZE * frame.MaterialToBinds.GetCount();
		frame.StorageBuffer->Resize(newStorageBufferSize);

		uint8* storageMap = reinterpret_cast<uint8*>(frame.StorageBuffer->MapMemory());
		uint64 storageOffset = 0;

		nsTArrayInline<VkWriteDescriptorSet, 32> writeDescriptorSets;
		nsTArrayInline<VkDescriptorBufferInfo, 32> bufferInfos;

		for (int i = 0; i < frame.MaterialToBinds.GetCount(); ++i)
		{
			const int id = frame.MaterialToBinds[i].Id;
			const nsMaterialPipelineState& pso = MaterialPipelineStates[id];

			if (pso.SurfaceDomain != nsEMaterialSurfaceDomain::SURFACE)
			{
				continue;
			}

			// Copy uniform data
			float* uniformData = &MaterialUniformData[id * NS_ENGINE_MATERIAL_UNIFORM_SIZE];
			nsPlatform::Memory_Copy(storageMap + storageOffset, uniformData, NS_ENGINE_MATERIAL_UNIFORM_SIZE);
			storageOffset += NS_ENGINE_MATERIAL_UNIFORM_SIZE;

			// Update descriptor set
			const nsMaterialResource& resource = MaterialResources[id];
			VkDescriptorSet& descriptorSet = frame.MaterialDescriptorSets[id];

			if (descriptorSet == VK_NULL_HANDLE)
			{
				descriptorSet = nsVulkan::CreateDescriptorSet(resource.ShaderResourceLayout, 3);
			}

			VkDescriptorBufferInfo& bufferInfo = bufferInfos.Add();
			bufferInfo.buffer = frame.StorageBuffer->GetVkBuffer();
			bufferInfo.offset = storageOffset;
			bufferInfo.range = NS_ENGINE_MATERIAL_UNIFORM_SIZE;

			VkWriteDescriptorSet& write = writeDescriptorSets.Add();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.dstSet = descriptorSet;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.pBufferInfo = &bufferInfo;
		}

		frame.StorageBuffer->UnmapMemory();

		vkUpdateDescriptorSets(nsVulkan::GetVkDevice(), static_cast<uint32>(writeDescriptorSets.GetCount()), writeDescriptorSets.GetData(), 0, nullptr);
	}


	// TODO: Cleanup pending destroy materials
	for (int i = 0; i < frame.MaterialToDestroys.GetCount(); ++i)
	{
		NS_AssertV(0, "Not implemented yet!");
	}
}
