// Local Includes
#include "rendertarget.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTarget)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("SampleShading",			&nap::RenderTarget::mSampleShading,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",				&nap::RenderTarget::mRequestedSamples,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",				&nap::RenderTarget::mClearColor,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ColorTexture",			&nap::RenderTarget::mColorTexture,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace 
	{
		bool createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = colorFormat;
			colorAttachment.samples = samples;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = depthFormat;
			depthAttachment.samples = samples;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription colorAttachmentResolve{};
			colorAttachmentResolve.format = colorFormat;
			colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			
			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentResolveRef{};
			colorAttachmentResolveRef.attachment = 2;
			colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
			subpass.pResolveAttachments = &colorAttachmentResolveRef;

			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
			std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve};
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = dependencies.size();
			renderPassInfo.pDependencies = dependencies.data();

			if (!errorState.check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass"))
				return false;

			return true;
		}
	}


	static bool createColorResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, ImageData& outData, utility::ErrorState& errorState)
	{
		// Create image allocation struct
		VmaAllocationCreateInfo img_alloc_usage = {};
		img_alloc_usage.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_alloc_usage.flags = 0;

		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, colorFormat, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, img_alloc_usage, outData.mTextureImage, outData.mTextureAllocation, outData.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.mTextureImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, outData.mTextureView, errorState))
			return false;

		return true;
	}


	static bool createDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		// Create image allocation struct
		VmaAllocationCreateInfo img_alloc_usage = {};
		img_alloc_usage.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_alloc_usage.flags = 0;

		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, renderer.getDepthFormat(), sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, img_alloc_usage, outImage.mTextureImage, outImage.mTextureAllocation, outImage.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.mTextureImage, renderer.getDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mTextureView, errorState))
			return false;

		return true;
	}


	RenderTarget::RenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}

	RenderTarget::~RenderTarget()
	{
		if (mFramebuffer != nullptr)
			vkDestroyFramebuffer(mRenderService->getDevice(), mFramebuffer, nullptr);
	
		if (mRenderPass != nullptr)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());

	}

	bool RenderTarget::init(utility::ErrorState& errorState)
	{
		// Warn if requested number of samples is not matched by hardware
		if (!mRenderService->getRasterizationSamples(mRequestedSamples, mRasterizationSamples, errorState))
			nap::Logger::warn(errorState.toString().c_str());

		// Check if sample rate shading is enabled
		if (mSampleShading && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShading = false;
		}

		if (!errorState.check(mColorTexture->mUsage == ETextureUsage::RenderTarget, "The color texture used by a RenderTarget must have a usage of 'RenderTarget' set."))
			return false;

		if (!createRenderPass(mRenderService->getDevice(), mColorTexture->getVulkanFormat(), mRenderService->getDepthFormat(), mRasterizationSamples, mRenderPass, errorState))
			return false;

		glm::ivec2 size = mColorTexture->getSize();
		VkExtent2D framebuffer_size;
		framebuffer_size.width = size.x;
		framebuffer_size.height = size.y;

		// Create multi-sampled color attachment
		if (!createColorResource(*mRenderService, framebuffer_size, mColorTexture->getVulkanFormat(), mRasterizationSamples, mColorImage, errorState))
			return false;

		// Create multi sampled depth attachment
		if (!createDepthResource(*mRenderService, framebuffer_size, mRasterizationSamples, mDepthImage, errorState))
			return false;

		std::array<VkImageView, 3> attachments = 
		{
			mColorImage.mTextureView,
			mDepthImage.mTextureView,
			mColorTexture->getImageView(),
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = framebuffer_size.width;
		framebufferInfo.height = framebuffer_size.height;
		framebufferInfo.layers = 1;

		if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebufferInfo, nullptr, &mFramebuffer) == VK_SUCCESS, "Failed to create framebuffer"))
			return false;

		return true;
	}

	void RenderTarget::beginRendering()
	{
		glm::ivec2 size = mColorTexture->getSize();

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32_t)size.x, (uint32_t)size.y };

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = size.x;
		rect.extent.height = size.y;
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = size.y;
		viewport.width = size.x;
		viewport.height = -size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}


	void RenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}


	const glm::ivec2 RenderTarget::getSize() const
	{
		return mColorTexture->getSize();
	}


	RenderTexture2D& RenderTarget::getColorTexture()
	{
		return *mColorTexture;
	}


	VkFormat RenderTarget::getColorFormat() const
	{
		return mColorTexture->getVulkanFormat();
	}


	VkFormat RenderTarget::getDepthFormat() const
	{
		return mRenderService->getDepthFormat();
	}


	VkSampleCountFlagBits RenderTarget::getSampleCount() const
	{
		return mRasterizationSamples;
	}


	bool RenderTarget::getSampleShadingEnabled() const
	{
		return mSampleShading;
	}

} // nap