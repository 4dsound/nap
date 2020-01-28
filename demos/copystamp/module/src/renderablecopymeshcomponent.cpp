#include "renderablecopymeshcomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>
#include "nindexbuffer.h"

// nap::renderablecopymeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableCopyMeshComponent)
	RTTI_PROPERTY("Orient",				&nap::RenderableCopyMeshComponent::mOrient,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",				&nap::RenderableCopyMeshComponent::mScale,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationSpeed",		&nap::RenderableCopyMeshComponent::mRotationSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomScale",		&nap::RenderableCopyMeshComponent::mRandomScale,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomRotation",		&nap::RenderableCopyMeshComponent::mRandomRotation,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableCopyMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorUniform",		&nap::RenderableCopyMeshComponent::mColorUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Camera",				&nap::RenderableCopyMeshComponent::mCamera,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetMesh",			&nap::RenderableCopyMeshComponent::mTargetMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CopyMeshes",			&nap::RenderableCopyMeshComponent::mCopyMeshes,					nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderablecopymeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableCopyMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderableCopyMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	/**
	 * Initializes this component. For this component to work a reference mesh + at least one mesh to copy onto it is needed.
	 * It also makes sure various uniforms (such as color) are present in the material. These uniforms are set when onRender() is called.
	 * But most importantly: it creates a valid RenderableMesh for every mesh to copy and caches it internally.
	 * The renderable mesh represents the coupling between a mesh and material. When valid, the mesh can be rendered with the material.
	 */
	bool RenderableCopyMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableCopyMeshComponent* resource = getComponent<RenderableCopyMeshComponent>();

		// Fetch transform, used to offset the copied meshes
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if(!errorState.check(mTransform != nullptr, 
			"%s: unable to find transform component", resource->mID.c_str()))
			return false;

		// Initialize our material instance based on values in the resource
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// Get handle to color uniform, which we set in the draw call
		mColorUniform = &mMaterialInstance.getOrCreateUniform("UBO").getOrCreateUniform<UniformVec3Instance>("meshColor");

		// Get handle to matrices, which we set in the draw call
		mProjectionUniform = &mMaterialInstance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>("projectionMatrix");
		mViewUniform = &mMaterialInstance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>("viewMatrix");
		mModelUniform = &mMaterialInstance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>("modelMatrix");

		// Ensure there's at least 1 mesh to copy
		if (!errorState.check(!(resource->mCopyMeshes.empty()), 
			"no meshes found to copy: %s", resource->mID.c_str()))
			return false;

		// Fetch render service
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<RenderService>();

		// Iterate over the meshes to copy
		// Create a valid mesh / material combination based on our referenced material and the meshes to copy
		// If a renderable mesh turns out to be invalid it means that the material / mesh combination isn't valid, ie:
		// There are required vertex attributes in the shader that the mesh doesn't have.
		for(auto& mesh : resource->mCopyMeshes)
		{
			RenderableMesh render_mesh = render_service->createRenderableMesh(*mesh, mMaterialInstance, errorState);
			if (!errorState.check(render_mesh.isValid(), "%s, mesh: %s can't be copied", resource->mID.c_str(), mesh->mID.c_str()))
				return false;

			// Store renderable mesh
			mCopyMeshes.emplace_back(render_mesh);
		}

		// Store handle to target mesh
		mTargetMesh = resource->mTargetMesh.get();

		// Ensure the reference mesh has vertices (position attribute).
		mTargetVertices = mTargetMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		if (!errorState.check(mTargetVertices != nullptr, "%s: unable to find target vertex position attribute", resource->mID.c_str()))
			return false;

		// Ensure the reference mesh has normals.
		mTargetNormals = mTargetMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		if (!errorState.check(mTargetNormals != nullptr, "%s: unable to find target normal attribute", resource->mID.c_str()))
			return false;

		// Copy over parameters
		mOrient = resource->mOrient;
		mScale	= resource->mScale;
		mRandomScale = resource->mRandomScale;
		mRandomRotation = resource->mRandomRotation;
		mRotationSpeed = resource->mRotationSpeed;

		// Add the colors that are randomly picked for every mesh that is drawn
		mColors.emplace_back(RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>());
			
		return true;
	}


	void RenderableCopyMeshComponentInstance::update(double deltaTime)
	{
		mTime += (deltaTime * (double)mRotationSpeed);
	}


	nap::MaterialInstance& RenderableCopyMeshComponentInstance::getMaterial()
	{
		return mMaterialInstance;
	}

	void renderMesh(RenderableMesh& renderableMesh, opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer)
	{
		MaterialInstance& mat_instance = renderableMesh.getMaterialInstance();
		VkDescriptorSet descriptor_set = mat_instance.update();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderableMesh.getPipeline());

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = renderTarget.getSize().x;
		viewport.height = renderTarget.getSize().y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Gather draw info
		MeshInstance& mesh_instance = renderableMesh.getMesh().getMeshInstance();
		opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		Material& material = mat_instance.getMaterial();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderableMesh.getPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);

		const std::vector<VkBuffer>& vertexBuffers = renderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = renderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = renderTarget.getSize().x;
		rect.extent.height = renderTarget.getSize().y;
		vkCmdSetScissor(commandBuffer, 0, 1, &rect);

		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}

	/**
	 * Called by the render service when the app wants to draw this component.
	 * A randomly selected mesh is rendered at the position of every vertex in the reference mesh.
	 * You can change the meshes that are copied and the reference mesh in the JSON file.
	 */
	void RenderableCopyMeshComponentInstance::onDraw(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get global transform
		const glm::mat4x4& model_matrix = mTransform->getGlobalTransform();

		// Set non changing uniforms
		mViewUniform->setValue(viewMatrix);
		mProjectionUniform->setValue(projectionMatrix);
		mColorUniform->setValue({ 1.0,0.0,0.0 });

		// Get points to copy onto
		std::vector<glm::vec3>& pos_data = mTargetVertices->getData();
		std::vector<glm::vec3>& nor_data = mTargetNormals->getData();

		// Fix seed for subsequent random calls
		math::setRandomSeed(mSeed);

		// Get randomization scale for various effects
		float rand_scale = math::clamp<float>(mRandomScale, 0.0f, 1.0f);
		float rand_rotat = math::clamp<float>(mRandomRotation, 0.0f, 1.0f);
		int max_rand_color = static_cast<int>(mColors.size()) - 1;

		// Get camera location
		glm::vec3 cam_pos = math::extractPosition(mCamera->getGlobalTransform());

		// Iterate over every point, fetch random mesh, construct custom object matrix, set uniforms and render.
		for (auto i = 0; i < pos_data.size(); i++)
		{
			// Pick random mesh number
			int mesh_idx = math::random<int>(0, mCopyMeshes.size() - 1);
			
			// Pick random color for mesh and push to GPU
			glm::vec3 color = mColors[math::random<int>(0, max_rand_color)].toVec3();
			mColorUniform->setValue(color);

			// Get the mesh to stamp onto this point and bind
			RenderableMesh& render_mesh = mCopyMeshes[mesh_idx];

			// Get data of that mesh
			MeshInstance& mesh_instance = render_mesh.getMesh().getMeshInstance();

			// GPU mesh representation of mesh to copy
			opengl::GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

			// Calculate model matrix
			glm::mat4x4 object_loc = glm::translate(model_matrix, pos_data[i]);

			// Orient towards camera using normal or rotate based on time
			float ftime = math::random<float>(1.0f - rand_rotat, 1.0f) * (float)mTime;
			if (mOrient)
			{
				glm::vec3 cam_normal = glm::normalize(cam_pos - pos_data[i]);
				glm::vec3 nor_normal = glm::normalize(nor_data[i]);
				glm::vec3 rot_normal = glm::cross(nor_normal, cam_normal);
				float rot_value = glm::acos(glm::dot(cam_normal, nor_normal));
				object_loc = glm::rotate(object_loc, rot_value, rot_normal);
			}
			else
			{
				object_loc = glm::rotate(object_loc, ftime, {0,1,0});
			}

			// Add scale, set as value and push
			float fscale = math::random<float>(1.0f - rand_scale, 1.0f) * mScale;
			mModelUniform->setValue(glm::scale(object_loc, { fscale, fscale, fscale }));

			renderMesh(render_mesh, renderTarget, commandBuffer);
		}
	}

}