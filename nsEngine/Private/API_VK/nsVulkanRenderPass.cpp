#include "nsVulkan.h"
#include "nsVulkanFunctions.h"



nsVulkanRenderPass::nsVulkanRenderPass() noexcept
	: RenderPass(VK_NULL_HANDLE)
	, DepthStencilAttachmentReference()
	, ResolveAttachmentReference()
	, AttachmentReferenceIndex(0)
	, ColorAttachmentCount(0)
	, bHasDepthStencilAttachment(false)
	, bHasResolveAttachment(false)
	, SubpassDependency()
	, Framebuffer(VK_NULL_HANDLE)
	, PrevDepthStencilView(nullptr)
	, PrevResolveView(nullptr)
{
	AttachmentTable.Reserve(4);
}


nsVulkanRenderPass::~nsVulkanRenderPass() noexcept
{
	nsVulkan::DestroyFramebuffer(Framebuffer);

	if (RenderPass)
	{
		NS_VK_PrintDestroyVulkanObject(RenderPass);
		vkDestroyRenderPass(nsVulkan::GetVkDevice(), RenderPass, nullptr);
	}
}


void nsVulkanRenderPass::Reset() noexcept
{
	AttachmentTable.Clear();
	ColorAttachmentReferences.Clear();
	AttachmentReferenceIndex = 0;
	ColorAttachmentCount = 0;
	bHasDepthStencilAttachment = false;
	bHasResolveAttachment = false;
}


void nsVulkanRenderPass::AddRenderTargetAttachment(int attachmentId, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) noexcept
{
	NS_Assert(!AttachmentTable.Exists(attachmentId));

	VkAttachmentDescription& attachment = AttachmentTable.Add(attachmentId);
	attachment.format = format;
	attachment.samples = sampleCount;
	attachment.initialLayout = inputLayout;
	attachment.finalLayout = outputLayout;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	ColorAttachmentReferences.Add({ static_cast<uint32>(AttachmentReferenceIndex++), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	ColorAttachmentCount++;
}


void nsVulkanRenderPass::SetDepthStencilAttachment(int attachmentId, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) noexcept
{
	NS_Assert(!AttachmentTable.Exists(attachmentId));
	NS_Assert(!bHasDepthStencilAttachment);

	VkAttachmentDescription& attachment = AttachmentTable.Add(attachmentId);
	attachment.format = format;
	attachment.samples = sampleCount;
	attachment.initialLayout = inputLayout;
	attachment.finalLayout = outputLayout;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = stencilLoadOp;
	attachment.stencilStoreOp = stencilStoreOp;

	DepthStencilAttachmentReference = { static_cast<uint32>(AttachmentReferenceIndex++), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	bHasDepthStencilAttachment = true;
}


void nsVulkanRenderPass::SetResolveAttachment(int attachmentId, VkFormat format, VkImageLayout inputLayout, VkImageLayout outputLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) noexcept
{
	NS_Assert(!AttachmentTable.Exists(attachmentId));
	NS_Assert(!bHasResolveAttachment);

	VkAttachmentDescription& attachment = AttachmentTable.Add(attachmentId);
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = inputLayout;
	attachment.finalLayout = outputLayout;
	attachment.loadOp = loadOp;
	attachment.storeOp = storeOp;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	ResolveAttachmentReference = { static_cast<uint32>(AttachmentReferenceIndex++), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	bHasResolveAttachment = true;
}


void nsVulkanRenderPass::SetSubpassDependency(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess) noexcept
{
	SubpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = srcStage;
	SubpassDependency.srcAccessMask = srcAccess;
	SubpassDependency.dstStageMask = dstStage;
	SubpassDependency.dstAccessMask = dstAccess;
}


void nsVulkanRenderPass::Build(nsName debugName) noexcept
{
	nsVulkan::DestroyFramebuffer(Framebuffer);

	if (RenderPass)
	{
		vkDestroyRenderPass(nsVulkan::GetVkDevice(), RenderPass, nullptr);
		RenderPass = VK_NULL_HANDLE;
	}

	const nsTArray<VkAttachmentDescription>& attachmentDescriptions = AttachmentTable.GetValues();

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.pColorAttachments = ColorAttachmentReferences.GetData();
	subpassDescription.colorAttachmentCount = static_cast<uint32>(ColorAttachmentReferences.GetCount());

	if (bHasDepthStencilAttachment)
	{
		subpassDescription.pDepthStencilAttachment = &DepthStencilAttachmentReference;
	}

	if (bHasResolveAttachment)
	{
		subpassDescription.pResolveAttachments = &ResolveAttachmentReference;
	}


	VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.pAttachments = attachmentDescriptions.GetData();
	renderPassCreateInfo.attachmentCount = static_cast<uint32>(attachmentDescriptions.GetCount());
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pDependencies = &SubpassDependency;
	renderPassCreateInfo.dependencyCount = 1;

	vkCreateRenderPass(nsVulkan::GetVkDevice(), &renderPassCreateInfo, nullptr, &RenderPass);
	NS_VK_SetDebugName(nsVulkan::GetVkDevice(), VK_OBJECT_TYPE_RENDER_PASS, RenderPass, debugName);
}


void nsVulkanRenderPass::BeginRenderPass(VkCommandBuffer commandBuffer, const nsVulkanTextureView** renderTargetViews, int renderTargetCount, const nsVulkanTextureView* depthStencilView, const nsVulkanTextureView* resolveView, nsPointInt framebufferDimension, nsLinearColor clearColor, float clearDepth, uint8 clearStencil) noexcept
{
	NS_Assert(renderTargetCount == ColorAttachmentCount);

	const int prevRenderTargetCount = PrevRenderTargetViews.GetCount();

	bool bShouldResizeFramebuffer = (Framebuffer == VK_NULL_HANDLE) || 
		FramebufferDimension != framebufferDimension || 
		(prevRenderTargetCount != renderTargetCount) || 
		(PrevDepthStencilView != depthStencilView) || 
		(PrevResolveView != resolveView);

	VkImageView attachments[8];
	uint32 attachmentCount = 0;
	PrevRenderTargetViews.Resize(renderTargetCount);

	for (int i = 0; i < renderTargetCount; ++i)
	{
		const nsVulkanTextureView* rtv = renderTargetViews[i];
		NS_Assert(rtv);

		if (!bShouldResizeFramebuffer && PrevRenderTargetViews[i] != rtv)
		{
			bShouldResizeFramebuffer = true;
		}

		attachments[attachmentCount++] = rtv->GetVkImageView();
		PrevRenderTargetViews[i] = rtv;
	}

	if (bHasDepthStencilAttachment)
	{
		NS_Assert(depthStencilView);
		attachments[attachmentCount++] = depthStencilView->GetVkImageView();
		PrevDepthStencilView = depthStencilView;
	}

	if (bHasResolveAttachment)
	{
		NS_Assert(resolveView);
		attachments[attachmentCount++] = resolveView->GetVkImageView();
		PrevResolveView = resolveView;
	}

	NS_Assert(attachmentCount <= 8);

	const uint32 width = static_cast<uint32>(framebufferDimension.X);
	const uint32 height = static_cast<uint32>(framebufferDimension.Y);

	if (bShouldResizeFramebuffer)
	{
		nsVulkan::DestroyFramebuffer(Framebuffer);
		Framebuffer = nsVulkan::CreateFramebuffer(RenderPass, attachments, attachmentCount, width, height);
		FramebufferDimension = framebufferDimension;
	}

	VkClearValue clearValues[8];
	uint32 clearValueCount = 0;

	for (int i = 0; i < ColorAttachmentCount; ++i)
	{
		nsPlatform::Memory_Copy(&clearValues[clearValueCount++].color, &clearColor, sizeof(float) * 4);
	}

	if (bHasDepthStencilAttachment)
	{
		VkClearValue& clearValue = clearValues[clearValueCount++];
		clearValue.depthStencil.depth = clearDepth;
		clearValue.depthStencil.stencil = clearStencil;
	}

	if (bHasResolveAttachment)
	{
		VkClearValue& clearValue = clearValues[clearValueCount++];
		nsPlatform::Memory_Copy(&clearValue.color, &clearColor, sizeof(float) * 4);
	}

	NS_Assert(clearValueCount <= 8);

	VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	beginInfo.renderPass = RenderPass;
	beginInfo.framebuffer = Framebuffer;
	beginInfo.pClearValues = clearValues;
	beginInfo.clearValueCount = clearValueCount;
	beginInfo.renderArea.offset = { 0, 0 };
	beginInfo.renderArea.extent = { width, height };

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void nsVulkanRenderPass::EndRenderPass(VkCommandBuffer commandBuffer) noexcept
{
	vkCmdEndRenderPass(commandBuffer);
}


nsVulkanRenderPass* nsVulkanRenderPass::CreateDefault_Forward() noexcept
{
	nsVulkanRenderPass* renderPassForward = ns_CreateObject<nsVulkanRenderPass>();
	renderPassForward->AddRenderTargetAttachment(0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	renderPassForward->SetDepthStencilAttachment(1, VK_FORMAT_D24_UNORM_S8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	renderPassForward->SetSubpassDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);
	renderPassForward->Build("renderpass_default_forward");

	return renderPassForward;
}


nsVulkanRenderPass* nsVulkanRenderPass::CreateDefault_Final() noexcept
{
	nsVulkanRenderPass* renderPassFinal = ns_CreateObject<nsVulkanRenderPass>();
	renderPassFinal->AddRenderTargetAttachment(0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	renderPassFinal->SetSubpassDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	renderPassFinal->Build("renderpass_default_final");

	return renderPassFinal;
}
