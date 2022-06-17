#include "nsVulkan.h"
#include "nsVulkanFunctions.h"
#include "../ThirdParty/spirv_reflect.h"



// ===================================================================================================================================================== //
// VULKAN SHADER MODULE
// ===================================================================================================================================================== //
nsVulkanShader::nsVulkanShader(VkShaderStageFlagBits stage, const uint32* codes, uint64 codeSize, nsName debugName) noexcept
	: Stage(stage)
{
	NS_Assert(codes);
	NS_Assert(codeSize > 0);

	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.pCode = codes;
	createInfo.codeSize = codeSize;

	vkCreateShaderModule(nsVulkan::GetVkDevice(), &createInfo, nullptr, &Module);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_SHADER_MODULE, Module, debugName);

	NS_Assert(codeSize % 4 == 0);
	const int wordCount = static_cast<int>(codeSize / sizeof(uint32));
	ByteCodes.InsertAt(codes, wordCount, 0);
}


nsVulkanShader::~nsVulkanShader() noexcept
{
	NS_VK_PrintDestroyVulkanObject(Module);
	vkDestroyShaderModule(nsVulkan::GetVkDevice(), Module, nullptr);
}


const nsTArray<uint32>& nsVulkanShader::GetByteCodes() const noexcept
{
	return ByteCodes;
}




// ===================================================================================================================================================== //
// VULKAN SHADER RESOURCE LAYOUT
// ===================================================================================================================================================== //
NS_NODISCARD static NS_INLINE VkDescriptorType ns_ToVkDescriptorType(SpvReflectDescriptorType type) noexcept
{
	switch (type)
	{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		default: break;
	}

	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}



nsVulkanShaderResourceLayout::nsVulkanShaderResourceLayout(nsName debugName) noexcept
	: PipelineLayout(VK_NULL_HANDLE)
{
	DebugName = debugName;
}

/*
nsVulkanShaderResourceLayout::nsVulkanShaderResourceLayout(nsVulkanShader* vertexShader, nsVulkanShader* fragmentShader, nsName debugName) noexcept
	: PipelineLayout(VK_NULL_HANDLE)
{
	CreateDescriptorSetLayoutBindingsAndPushConstantFromShaderReflect(vertexShader);
	CreateDescriptorSetLayoutBindingsAndPushConstantFromShaderReflect(fragmentShader);
	Build(debugName);
}
*/

nsVulkanShaderResourceLayout::~nsVulkanShaderResourceLayout() noexcept
{
	for (int i = 0; i < DescriptorSetLayouts.GetCount(); ++i)
	{
		vkDestroyDescriptorSetLayout(nsVulkan::GetVkDevice(), DescriptorSetLayouts[i], nullptr);
	}

	NS_VK_PrintDestroyVulkanObject(PipelineLayout);
	vkDestroyPipelineLayout(nsVulkan::GetVkDevice(), PipelineLayout, nullptr);
}

/*
void nsVulkanShaderResourceLayout::CreateDescriptorSetLayoutBindingsAndPushConstantFromShaderReflect(const nsVulkanShader* shader) noexcept
{
	const nsTArray<uint32>& shaderByteCodes = shader->GetByteCodes();

	SpvReflectShaderModule reflModule{};
	SpvReflectResult result = spvReflectCreateShaderModule(sizeof(uint32) * shaderByteCodes.GetCount(), shaderByteCodes.GetData(), &reflModule);
	NS_Assert(result == SPV_REFLECT_RESULT_SUCCESS);

	// Descriptor sets
	uint32 descriptorCount = 0;
	result = spvReflectEnumerateDescriptorSets(&reflModule, &descriptorCount, nullptr);
	NS_Assert(result == SPV_REFLECT_RESULT_SUCCESS);
	NS_Assert(descriptorCount <= 8);

	SpvReflectDescriptorSet* reflDescriptorSets[8];
	result = spvReflectEnumerateDescriptorSets(&reflModule, &descriptorCount, reflDescriptorSets);
	NS_Assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (uint32 i = 0; i < descriptorCount; ++i)
	{
		const SpvReflectDescriptorSet* reflDescriptorSet = reflDescriptorSets[i];
		const uint32 setId = reflDescriptorSet->set;
		const int bindingCount = static_cast<int>(reflDescriptorSet->binding_count);
		NS_Assert(bindingCount <= 8);

		nsTArrayInline<VkDescriptorSetLayoutBinding, 8>& layoutBindings = DescriptorSetLayoutBindings[setId];
		layoutBindings.Resize(bindingCount);

		for (int j = 0; j < bindingCount; ++j)
		{
			const SpvReflectDescriptorBinding* reflBinding = reflDescriptorSet->bindings[j];
			const uint32 bindingId = reflBinding->binding;

			VkDescriptorSetLayoutBinding& layout = layoutBindings[j];
			layout.binding = bindingId;
			layout.descriptorType = ns_ToVkDescriptorType(reflBinding->descriptor_type);
			layout.descriptorCount = reflBinding->count;
			layout.stageFlags |= shader->GetShaderStage();
			layout.pImmutableSamplers = nullptr;
		}
	}

	// Push constants
	uint32 pushConstantCount = 0;
	result = spvReflectEnumeratePushConstantBlocks(&reflModule, &pushConstantCount, nullptr);
	NS_Assert(result == SPV_REFLECT_RESULT_SUCCESS);
	NS_Assert(pushConstantCount <= 4);

	SpvReflectBlockVariable* reflPushConstantBlocks[4];
	result = spvReflectEnumeratePushConstantBlocks(&reflModule, &pushConstantCount, reflPushConstantBlocks);
	NS_Assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (uint32 i = 0; i < pushConstantCount; ++i)
	{
		const SpvReflectBlockVariable* reflPushConstantBlock = reflPushConstantBlocks[i];
		VkPushConstantRange& pushConstant = PushConstants.Add();
		pushConstant.offset = reflPushConstantBlock->offset;
		pushConstant.size = reflPushConstantBlock->size;
		pushConstant.stageFlags |= shader->GetShaderStage();
	}

	spvReflectDestroyShaderModule(&reflModule);
}
*/


void nsVulkanShaderResourceLayout::Reset() noexcept
{
	nsVulkan::WaitDeviceIdle();

	DescriptorSetLayoutBindings.Clear();
	DescriptorBindingInfos.Clear();

	for (int i = 0; i < DescriptorSetLayouts.GetCount(); ++i)
	{
		vkDestroyDescriptorSetLayout(nsVulkan::GetVkDevice(), DescriptorSetLayouts[i], nullptr);
	}

	DescriptorSetLayouts.Clear();

	if (PipelineLayout)
	{
		vkDestroyPipelineLayout(nsVulkan::GetVkDevice(), PipelineLayout, nullptr);
		PipelineLayout = VK_NULL_HANDLE;
	}
}


void nsVulkanShaderResourceLayout::AddDescriptorBindingUniform(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicOffset, bool bIsDynamicIndexing) noexcept
{
	DescriptorBindingInfos[descriptorSlot].Resize(bindingSlot + 1);

	DescriptorBindingInfo& info = DescriptorBindingInfos[descriptorSlot][bindingSlot];
	info.ShaderStages = shaderStages;
	info.ArrayCount = arrayCount;
	info.Type = bIsDynamicOffset ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	info.bIsDynamicIndexing = bIsDynamicIndexing;
}


void nsVulkanShaderResourceLayout::AddDescriptorBindingStorage(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicOffset, bool bIsDynamicIndexing) noexcept
{
	DescriptorBindingInfos[descriptorSlot].Resize(bindingSlot + 1);

	DescriptorBindingInfo& info = DescriptorBindingInfos[descriptorSlot][bindingSlot];
	info.ShaderStages = shaderStages;
	info.ArrayCount = arrayCount;
	info.Type = bIsDynamicOffset ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	info.bIsDynamicIndexing = bIsDynamicIndexing;
}


void nsVulkanShaderResourceLayout::AddDescriptorBindingTexture(VkShaderStageFlags shaderStages, int descriptorSlot, int bindingSlot, int arrayCount, bool bIsDynamicIndexing) noexcept
{
	DescriptorBindingInfos[descriptorSlot].Resize(bindingSlot + 1);

	DescriptorBindingInfo& info = DescriptorBindingInfos[descriptorSlot][bindingSlot];
	info.ShaderStages = shaderStages;
	info.ArrayCount = arrayCount;
	info.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	info.bIsDynamicIndexing = bIsDynamicIndexing;
}


void nsVulkanShaderResourceLayout::AddPushConstant(VkShaderStageFlags shaderStages, uint32 offset, uint32 size) noexcept
{
	VkPushConstantRange& pcr = PushConstants.Add();
	pcr.stageFlags = shaderStages;
	pcr.offset = offset;
	pcr.size = size;
}


void nsVulkanShaderResourceLayout::Build()
{
	/*
	for (int i = 0; i < DescriptorSetLayoutBindings.GetCount(); ++i)
	{
		const nsTArrayInline<VkDescriptorSetLayoutBinding, 8>& layoutBindings = DescriptorSetLayoutBindings.GetValueByIndex(i);
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		dslCreateInfo.flags = 0;
		dslCreateInfo.pBindings = layoutBindings.GetData();
		dslCreateInfo.bindingCount = static_cast<uint32>(layoutBindings.GetCount());

		VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
		vkCreateDescriptorSetLayout(nsVulkan::GetVkDevice(), &dslCreateInfo, nullptr, &dsl);

		DescriptorSetLayouts.Add(dsl);
	}
	*/

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[8] = {};
	VkDescriptorBindingFlags descriptorBindingFlags[8] = {};

	for (int i = 0; i < DescriptorBindingInfos.GetCount(); ++i)
	{
		const nsTArrayInline<DescriptorBindingInfo, 8>& bindings = DescriptorBindingInfos.GetValueByIndex(i);
		const int bindingCount = bindings.GetCount();
		NS_Assert(bindingCount <= 8);

		for (int j = 0; j < bindingCount; ++j)
		{
			const DescriptorBindingInfo& info = bindings[j];
			VkDescriptorSetLayoutBinding& layout = descriptorSetLayoutBindings[j];
			layout.stageFlags = info.ShaderStages;
			layout.binding = static_cast<uint32>(j);
			layout.descriptorType = info.Type;
			layout.descriptorCount = static_cast<uint32>(info.ArrayCount);
			layout.pImmutableSamplers = nullptr;

			VkDescriptorBindingFlags& flags = descriptorBindingFlags[j];
			flags = info.bIsDynamicIndexing ? VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT : 0;
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;
		descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32>(bindingCount);

		VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlagsCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags;
		descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32>(bindingCount);

		descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;

		VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
		vkCreateDescriptorSetLayout(nsVulkan::GetVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &dsl);
		DescriptorSetLayouts.Add(dsl);
	}

	VkPipelineLayoutCreateInfo plCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	plCreateInfo.pSetLayouts = DescriptorSetLayouts.GetData();
	plCreateInfo.setLayoutCount = static_cast<uint32>(DescriptorSetLayouts.GetCount());
	plCreateInfo.pPushConstantRanges = PushConstants.GetData();
	plCreateInfo.pushConstantRangeCount = static_cast<uint32>(PushConstants.GetCount());

	vkCreatePipelineLayout(nsVulkan::GetVkDevice(), &plCreateInfo, nullptr, &PipelineLayout);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, PipelineLayout, DebugName);
}


nsVulkanShaderResourceLayout* nsVulkanShaderResourceLayout::GetDefault_TextureShaderResourceLayout() noexcept
{
	static nsVulkanShaderResourceLayout* _default = nullptr;

	if (_default == nullptr)
	{
		_default = ns_CreateObject<nsVulkanShaderResourceLayout>("srl_def_textures");
		_default->AddDescriptorBindingTexture(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 0, NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT, true);
		_default->Build();
	}

	return _default;
}
