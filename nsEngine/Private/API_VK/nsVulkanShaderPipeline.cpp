#include "nsVulkan.h"
#include "nsVulkanFunctions.h"



nsVulkanShaderPipeline::nsVulkanShaderPipeline(VkPipelineCache pipelineCache, const nsVulkanPipelineState& pipelineState, nsName debugName) noexcept
	: PipelineState(pipelineState)
	, Pipeline(VK_NULL_HANDLE)
{
	NS_Assert(pipelineState.RenderPass);
	NS_Assert(pipelineState.ShaderResourceLayout);

	// Vertex input state
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputStateCreateInfo.pVertexBindingDescriptions = PipelineState.VertexInputBindings.GetData();
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32>(PipelineState.VertexInputBindings.GetCount());
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = PipelineState.VertexInputAttributes.GetData();
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(PipelineState.VertexInputAttributes.GetCount());


	// Shader stages
	nsTArrayInline<VkPipelineShaderStageCreateInfo, 5> shaderStageCreateInfos;
	{
		VkPipelineShaderStageCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		info.pName = "main";

		if (PipelineState.VertexShader)
		{
			info.stage = VK_SHADER_STAGE_VERTEX_BIT;
			info.module = PipelineState.VertexShader->GetVkShaderModule();
			shaderStageCreateInfos.Add(info);
		}

		if (PipelineState.FragmentShader)
		{
			info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			info.module = PipelineState.FragmentShader->GetVkShaderModule();
			shaderStageCreateInfos.Add(info);
		}
	}


	// Input assembly state
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
	inputAssemblyCreateInfo.topology = PipelineState.Topology;


	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationStateCreateInfo.polygonMode = PipelineState.PolygonMode;
	rasterizationStateCreateInfo.frontFace = PipelineState.FrontFace;
	rasterizationStateCreateInfo.cullMode = PipelineState.CullMode;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	rasterizationStateCreateInfo.depthBiasEnable = PipelineState.bEnableDepthBias;
	rasterizationStateCreateInfo.depthBiasConstantFactor = PipelineState.DepthBiasConstant;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = PipelineState.DepthBiasSlope;
	rasterizationStateCreateInfo.depthClampEnable = PipelineState.bEnableDepthClamp;
	rasterizationStateCreateInfo.depthBiasClamp = PipelineState.DepthBiasClamp;


	// Multisample state
	VkPipelineMultisampleStateCreateInfo multiSampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multiSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multiSampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	multiSampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multiSampleStateCreateInfo.minSampleShading = VK_FALSE;
	multiSampleStateCreateInfo.pSampleMask = nullptr;


	// Color blend state
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;

	switch (PipelineState.BlendMode)
	{
		case nsEBlendMode::ADDITIVE:
		{
			NS_ValidateV(0, "Not implemented yet!");
			break;
		}

		case nsEBlendMode::MULTIPLY:
		{
			NS_ValidateV(0, "Not implemented yet!");
			break;
		}

		case nsEBlendMode::TRANSPARENCY:
		{
			colorBlendAttachmentState.blendEnable = VK_TRUE;
			colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

			break;
		}

		default:
			break;
	}

	const nsVulkanRenderPass* vulkanRenderPass = PipelineState.RenderPass;
	const uint32 colorBlendAttachmentCount = vulkanRenderPass->GetColorAttachmentCount();
	NS_Assert(colorBlendAttachmentCount <= 8);
	nsTArrayInline<VkPipelineColorBlendAttachmentState, 8> colorBlendAttachmentStates;

	for (uint32 i = 0; i < colorBlendAttachmentCount; ++i)
	{
		colorBlendAttachmentStates.Add(colorBlendAttachmentState);
	}

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentStates.GetData();
	colorBlendStateCreateInfo.attachmentCount = static_cast<uint32>(colorBlendAttachmentStates.GetCount());


	// Depth stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilStateCreateInfo.depthTestEnable = PipelineState.bEnableDepthTest;
	depthStencilStateCreateInfo.depthWriteEnable = PipelineState.bEnableDepthWrite;
	depthStencilStateCreateInfo.depthBoundsTestEnable = PipelineState.bEnableDepthBound;
	depthStencilStateCreateInfo.minDepthBounds = PipelineState.MinDepthBound;
	depthStencilStateCreateInfo.maxDepthBounds = PipelineState.MaxDepthBound;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateCreateInfo.stencilTestEnable = PipelineState.bEnableStencilTest;
	depthStencilStateCreateInfo.front.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.front.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;


	// Dynamic states
	VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;
	dynamicStateCreateInfo.dynamicStateCount = 2;


	// Viewport state
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;


	// Tessellation state
	VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineCreateInfo.renderPass = vulkanRenderPass->GetVkRenderPass();
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.layout = PipelineState.ShaderResourceLayout->GetVkPipelineLayout();
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pStages = shaderStageCreateInfos.GetData();
	pipelineCreateInfo.stageCount = static_cast<uint32>(shaderStageCreateInfos.GetCount());
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multiSampleStateCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pTessellationState = &tessellationStateCreateInfo;

	vkCreateGraphicsPipelines(nsVulkan::GetVkDevice(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &Pipeline);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_PIPELINE, Pipeline, debugName);
}


nsVulkanShaderPipeline::~nsVulkanShaderPipeline() noexcept
{
	NS_VK_PrintDestroyVulkanObject(Pipeline);
	vkDestroyPipeline(nsVulkan::GetVkDevice(), Pipeline, nullptr);
}
