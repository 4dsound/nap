// Local Includes
#include "renderabletextcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "indexbuffer.h"
#include "uniforminstances.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::renderabletextcomponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponent)
	RTTI_PROPERTY("Text",				&nap::RenderableTextComponent::mText,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Font",				&nap::RenderableTextComponent::mFont,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GlyphUniform",		&nap::RenderableTextComponent::mGlyphUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableTextComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderabletextcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	RenderableTextComponentInstance::RenderableTextComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore())
	{
	}

	bool RenderableTextComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableTextComponent* resource = getComponent<RenderableTextComponent>();
		
		// Extract font
		mFont = &(resource->mFont->getFontInstance());

		// Extract glyph uniform (texture slot in shader)
		mGlyphUniformName = resource->mGlyphUniform;

		// Fetch transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();

		// Create material instance
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// Ensure the uniform to set the glyph is available on the source material
		mGlyphUniform = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(mGlyphUniformName);
		if (!errorState.check(mGlyphUniform != nullptr,
		 	"%s: Unable to bind font character, can't find 2DSampler uniform: %s in material: %s", this->mID.c_str(), 
			mGlyphUniformName.c_str() , mMaterialInstance.getMaterial().mID.c_str()))
		 	return false;

		// Setup the plane, 1x1 with lower left corner at origin {0, 0}
		mPlane.mRows	= 1;
		mPlane.mColumns = 1;
		mPlane.mPosition = { 0.5f, 0.5f };
		mPlane.mSize = { 1.0f, 1.0f };
		if (!mPlane.setup(errorState))
			return false;

		// Make sure we can write to it often 
		mPlane.getMeshInstance().setUsage(EMeshDataUsage::DynamicWrite);

		// Update the uv coordinates
		Vec3VertexAttribute* uv_attr = mPlane.getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		if (!errorState.check(uv_attr != nullptr, "%s: unable to find uv vertex attribute on plane", mID.c_str()))
			return false;

		// Flip uv y axis (text is rendered flipped)
		uv_attr->getData()[0].y = 1.0f;
		uv_attr->getData()[1].y = 1.0f;
		uv_attr->getData()[2].y = 0.0f;
		uv_attr->getData()[3].y = 0.0f;

		// Initialize it on the GPU
		if (!mPlane.getMeshInstance().init(errorState))
			return false;

		// Get position attribute buffer, we will update the vertex positions of this plane
		mPositionAttr = mPlane.getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		if (!errorState.check(mPositionAttr != nullptr, "%s: unable to get plane vertex attribute handle", mID.c_str()))
			return false;

		// Construct render-able mesh (TODO: Make a factory or something similar to create and verify render-able meshes!
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		mRenderableMesh = render_service->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Set text, needs to succeed on initialization
		if (!setText(resource->mText, errorState))
			return false;

		return true;
	}


	const nap::FontInstance& RenderableTextComponentInstance::getFont() const
	{
		assert(mFont != nullptr);
		return *mFont;
	}


	bool RenderableTextComponentInstance::setText(const std::string& text, utility::ErrorState& error)
	{
		// Clear Glyph handles
		mGlyphs.clear();
		mGlyphs.reserve(text.size());

		// Get or create a Glyph for every letter in the text
		bool success(true);
		for (const auto& letter : text)
		{
			// Fetch glyph.
			RenderableGlyph* glyph = getRenderableGlyph(mFont->getGlyphIndex(letter), error);
			if (!error.check(glyph != nullptr, "%s: unsupported character: %d, %s", mID.c_str(), letter, error.toString().c_str()))
			{
				success = false;
				continue;
			}
			// Store handle
			mGlyphs.emplace_back(glyph);
		}
		mText = text;
		mFont->getBoundingBox(mText, mTextBounds);	
		return success;
	}


	nap::MaterialInstance& RenderableTextComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}


	void RenderableTextComponentInstance::draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Get the parent material and set uniform values if present
		UniformStructInstance* nap_uniform = mMaterialInstance.getMaterial().findUniform(napStructUniform);
		UniformMat4Instance* model_uniform = nullptr;
		if (nap_uniform != nullptr)
		{
			// Set projection uniform in shader
			UniformMat4Instance* projection_uniform = nap_uniform->getOrCreateUniform<UniformMat4Instance>(projectionMatrixUniform);
			if (projection_uniform != nullptr)
				projection_uniform->setValue(projectionMatrix);

			// Set view uniform in shader
			UniformMat4Instance* view_uniform = nap_uniform->getOrCreateUniform<UniformMat4Instance>(viewMatrixUniform);
			if (view_uniform != nullptr)
				view_uniform->setValue(viewMatrix);

			// Set model matrix uniform in shader
			model_uniform = nap_uniform->getOrCreateUniform<UniformMat4Instance>(modelMatrixUniform);
		}

		// Get vertex position data (that we update in the loop
		std::vector<glm::vec3>& pos_data = mPositionAttr->getData();

		// Get plane to draw
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();

		// Get render service
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();

		// Fetch index buffer (holding drawing order
		const IndexBuffer& index_buffer = mesh_instance.getGPUMesh().getIndexBuffer(0);

		utility::ErrorState error_state;

		// Location of active letter
		float x = 0.0f;
		float y = 0.0f;

		// Get pipeline
		RenderService::Pipeline pipeline = render_service->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		
		// Draw individual glyphs
		for (auto& render_glyph : mGlyphs)
		{
			// Don't draw empty glyphs (spaces)
			if (render_glyph->empty())
			{
				x += render_glyph->getHorizontalAdvance();
				continue;
			}

			// Get width and height of character to draw
			float w = render_glyph->getSize().x;
			float h = render_glyph->getSize().y;

			// Compute x and y position
			float xpos = x + render_glyph->getOffsetLeft();
			float ypos = y - (h - render_glyph->getOffsetTop());
			glm::vec3 position(xpos, ypos, 0.0f);
			
			// Compute scale
			glm::vec3 scale(w, h, 1.0f);

			// Compute matrix
			glm::mat4 plane_loc = glm::translate(glm::mat4(), position);
			plane_loc = glm::scale(plane_loc, scale);

			// Set model matrix
			if (model_uniform != nullptr)
				model_uniform->setValue(modelMatrix * plane_loc);

			// Set glyph
			mGlyphUniform->setTexture(render_glyph->getTexture());

			// Get new descriptor set
			VkDescriptorSet descriptor_set = mMaterialInstance.update();

			// Bind pipeline
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

			// Set viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = renderTarget.getSize().x;
			viewport.height = renderTarget.getSize().y;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			// Bind our desciptor set
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

			// Bind vertex buffers
			const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
			const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
			vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

			// TODO: Compute scissor correctly based on 2D text location, saves a lot of drawing overhead!
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getSize().x;
			rect.extent.height = renderTarget.getSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);

			// Draw geometry
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

			// Update x
			x += render_glyph->getHorizontalAdvance();
		}
	}


	const math::Rect& RenderableTextComponentInstance::getBoundingBox() const
	{ 
		return mTextBounds;
	}
}
