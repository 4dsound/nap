// Local Includes
#include "renderservice.h"
#include "renderablemeshcomponent.h"
#include "rendercomponent.h"
#include "renderwindow.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"
#include "mesh.h"
#include "depthsorter.h"
#include "vertexbuffer.h"

// External Includes
#include <nap/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <rtti/factory.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>
#include <scene.h>
#include "boxmesh.h"
#include "meshfromfile.h"
#include "trianglemesh.h"
#include "shader.h"
#include "material.h"
#include "rendertexture2d.h"
#include "imagefromfile.h"
#include "image.h"
#include "texture2d.h"
#include "planemesh.h"
#include "spheremesh.h"
#include "descriptorsetcache.h"
#include "descriptorsetallocator.h"
#include "vk_mem_alloc.h"
#include "SDL_vulkan.h"
#include "sdlhelpers.h"
#include "glslang/Public/ShaderLang.h"

RTTI_BEGIN_CLASS(nap::RenderServiceConfiguration)
	RTTI_PROPERTY("Settings",	&nap::RenderServiceConfiguration::mSettings,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	/**
	*	@return the set of layers to be initialized with Vulkan
	*/
	const std::set<std::string>& getRequestedLayerNames()
	{
		static std::set<std::string> layers;
		if (layers.empty())
		{
			layers.emplace("VK_LAYER_NV_optimus");
			layers.emplace("VK_LAYER_LUNARG_standard_validation");
		}
		return layers;
	}


	/**
	* @return the set of required device extension names
	*/
	const std::set<std::string>& getRequestedDeviceExtensionNames()
	{
		static std::set<std::string> layers;
		if (layers.empty())
		{
			layers.emplace(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
		return layers;
	}

	/**
	* Callback that receives a debug message from Vulkan
	*/
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData)
	{
		nap::Logger::info("Validation Layer [%s]: %s", layerPrefix, msg);
		return VK_FALSE;
	}

	VkResult createDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	/**
	*	Sets up the vulkan messaging callback specified above
	*/
	bool setupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT& callback, utility::ErrorState& errorState)
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (!errorState.check(createDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) == VK_SUCCESS, "Unable to create debug report callback extension"))
			return false;

		return true;
	}


	bool getAvailableVulkanLayers(std::vector<std::string>& outLayers, utility::ErrorState& errorState)
	{
		// Figure out the amount of available layers
		// Layers are used for debugging / validation etc / profiling..
		unsigned int instance_layer_count = 0;
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL) == VK_SUCCESS, "Unable to query vulkan instance layer property count"))
			return false;

		std::vector<VkLayerProperties> instance_layers(instance_layer_count);
		if (!errorState.check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()) == VK_SUCCESS, "Unable to retrieve vulkan instance layer names"))
			return false;

		Logger::info("Found %d instance layers:", instance_layer_count);

		const std::set<std::string>& requested_layers = getRequestedLayerNames();
		outLayers.clear();
		for (int index = 0; index < instance_layers.size(); ++index)
		{
			VkLayerProperties& layer = instance_layers[index];
			Logger::info("%d: %s: %s", index, layer.layerName, layer.description);

			if (requested_layers.find(std::string(layer.layerName)) != requested_layers.end())
				outLayers.emplace_back(layer.layerName);
		}

		// Print the ones we're enabling
		Logger::info("");
		for (const auto& layer : outLayers)
			Logger::info("Applying layer: %s", layer.c_str());

		return true;
	}


	bool getAvailableVulkanExtensions(SDL_Window* window, std::vector<std::string>& outExtensions, utility::ErrorState& errorState)
	{
		// Figure out the amount of extensions vulkan needs to interface with the os windowing system 
		// This is necessary because vulkan is a platform agnostic API and needs to know how to interface with the windowing system
		unsigned int ext_count = 0;
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr) == SDL_TRUE, "Unable to query the number of Vulkan instance extensions"))
			return false;

		// Use the amount of extensions queried before to retrieve the names of the extensions
		std::vector<const char*> ext_names(ext_count);
		if (!errorState.check(SDL_Vulkan_GetInstanceExtensions(window, &ext_count, ext_names.data()) == SDL_TRUE, "Unable to query the number of Vulkan instance extension names"))
			return false;

		// Display names
		Logger::info("Found %d Vulkan instance extensions:", ext_count);
		for (unsigned int i = 0; i < ext_count; i++)
		{
			Logger::info("%d: %s", i, ext_names[i]);
			outExtensions.emplace_back(ext_names[i]);
		}

		// Add debug display extension, we need this to relay debug messages
		outExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		return true;
	}


	/**
	* Creates a vulkan instance using all the available instance extensions and layers
	* @return if the instance was created successfully
	*/
	bool createVulkanInstance(const std::vector<std::string>& layerNames, const std::vector<std::string>& extensionNames, VkInstance& outInstance, utility::ErrorState& errorState)
	{
		// Copy layers
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());

		// Copy extensions
		std::vector<const char*> ext_names;
		for (const auto& ext : extensionNames)
			ext_names.emplace_back(ext.c_str());

		// Get the suppoerted vulkan instance version
		unsigned int api_version;
		vkEnumerateInstanceVersion(&api_version);

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = "NAP";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "NAP";
		app_info.engineVersion = 1;
		app_info.apiVersion = api_version;	// Note: this is the *requested* version, which is the version the installed vulkan driver supports. If the device itself does not support this version, a lower version is returned. See selectGPU

											// initialize the VkInstanceCreateInfo structure
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledExtensionCount = static_cast<uint32_t>(ext_names.size());
		inst_info.ppEnabledExtensionNames = ext_names.data();
		inst_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		inst_info.ppEnabledLayerNames = layer_names.data();

		// Create vulkan runtime instance
		Logger::info("Initializing Vulkan instance");
		VkResult res = vkCreateInstance(&inst_info, NULL, &outInstance);
		switch (res)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			errorState.fail("Unable to create vulkan instance, cannot find a compatible Vulkan driver");
			return false;
		default:
			errorState.fail("Unable to create Vulkan instance: unknown error");
			return false;
		}

		return true;
	}


	/**
	* Allows the user to select a GPU (physical device)
	* @return if query, selection and assignment was successful
	* @param outDevice the selected physical device (gpu)
	* @param outQueueFamilyIndex queue command family that can handle graphics commands
	*/
	bool selectGPU(VkInstance instance, VkPhysicalDevice& outDevice, uint32_t& outDeviceVersion, unsigned int& outQueueFamilyIndex, utility::ErrorState& errorState)
	{
		// Get number of available physical devices, needs to be at least 1
		unsigned int physical_device_count(0);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
		if (!errorState.check(physical_device_count != 0, "No physical devices found"))
			return false;

		// Now get the devices
		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// Show device information
		Logger::info("Found %d GPUs:", physical_device_count);

		std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_devices.size());
		for (int index = 0; index < physical_devices.size(); ++index)
		{
			VkPhysicalDevice physical_device = physical_devices[index];
			VkPhysicalDeviceProperties& properties = physical_device_properties[index];
			vkGetPhysicalDeviceProperties(physical_device, &properties);

			Logger::info("%d: %s (%d.%d)", index, properties.deviceName, VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion));
		}

		VkPhysicalDevice selected_device = physical_devices[0];

		// Find the number queues this device supports, we want to make sure that we have a queue that supports graphics commands
		unsigned int family_queue_count(0);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, nullptr);
		if (!errorState.check(family_queue_count != 0, "Device has no family of queues associated with it"))
			return false;

		// Extract the properties of all the queue families
		std::vector<VkQueueFamilyProperties> queue_properties(family_queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(selected_device, &family_queue_count, queue_properties.data());

		// Make sure the family of commands contains an option to issue graphical commands.
		unsigned int queue_node_index = -1;
		for (unsigned int i = 0; i < family_queue_count; i++)
		{
			if (queue_properties[i].queueCount > 0 && queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queue_node_index = i;
				break;
			}
		}

		if (!errorState.check(queue_node_index >= 0, "Unable to find graphics command queue on device"))
			return false;

		// Set the output variables
		outDevice = selected_device;
		outDeviceVersion = physical_device_properties[0].apiVersion;	// The actual version of the device, which may be different from what we requested in the ApplicationInfo
		outQueueFamilyIndex = queue_node_index;
		return true;
	}


	/**
	*	Creates a logical device
	*/
	bool createLogicalDevice(VkPhysicalDevice& physicalDevice, unsigned int queueFamilyIndex, const std::vector<std::string>& layerNames, VkDevice& outDevice, utility::ErrorState& errorState)
	{
		// Copy layer names
		std::vector<const char*> layer_names;
		for (const auto& layer : layerNames)
			layer_names.emplace_back(layer.c_str());


		// Get the number of available extensions for our graphics card
		uint32_t device_property_count(0);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, NULL) == VK_SUCCESS, "Unable to acquire device extension property count"))
			return false;

		Logger::info("Found %d device extensions", device_property_count);

		// Acquire their actual names
		std::vector<VkExtensionProperties> device_properties(device_property_count);
		if (!errorState.check(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &device_property_count, device_properties.data()) == VK_SUCCESS, "Unable to acquire device extension property names"))
			return false;

		// Match names against requested extension
		std::vector<const char*> device_property_names;
		const std::set<std::string>& required_extension_names = getRequestedDeviceExtensionNames();
		for (int index = 0; index < device_properties.size(); ++index)
		{
			const VkExtensionProperties& ext_property = device_properties[index];

			Logger::info("%d: %s", index, ext_property.extensionName);

			auto it = required_extension_names.find(std::string(ext_property.extensionName));
			if (it != required_extension_names.end())
				device_property_names.emplace_back(ext_property.extensionName);

		}

		if (!errorState.check(required_extension_names.size() == device_property_names.size(), "Unable to find all required extensions"))
			return false;

		for (const auto& name : device_property_names)
			Logger::info("Applying device extension %s", name);

		// Create queue information structure used by device based on the previously fetched queue information from the physical device
		// We create one command processing queue for graphics
		VkDeviceQueueCreateInfo queue_create_info;
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queueFamilyIndex;
		queue_create_info.queueCount = 1;
		std::vector<float> queue_prio = { 1.0f };
		queue_create_info.pQueuePriorities = queue_prio.data();
		queue_create_info.pNext = NULL;
		queue_create_info.flags = NULL;

		// Device creation information
		VkDeviceCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = 1;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.ppEnabledLayerNames = layer_names.data();
		create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		create_info.ppEnabledExtensionNames = device_property_names.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_property_names.size());
		create_info.pNext = NULL;
		create_info.pEnabledFeatures = NULL;
		create_info.flags = NULL;

		// Finally we're ready to create a new device
		if (!errorState.check(vkCreateDevice(physicalDevice, &create_info, nullptr, &outDevice) == VK_SUCCESS, "Failed to create logical device"))
			return false;

		return true;
	}

	bool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, unsigned int graphicsQueueIndex, VkCommandPool& commandPool)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicsQueueIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		return vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS;
	}

	static bool findDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outFormat)
	{
		std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				outFormat = format;
				return true;
			}
		}

		return false;
	}

//////////////////////////////////////////////////////////////////////////

	RenderService::RenderService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	// Register all object creation functions
	void RenderService::registerObjectCreators(rtti::Factory& factory)
	{
	}


	void RenderService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
	}


	std::shared_ptr<GLWindow> RenderService::addWindow(RenderWindow& window, utility::ErrorState& errorState)
	{
		// Get settings
		RenderWindowSettings window_settings;
		window_settings.width = window.mWidth;
		window_settings.height = window.mHeight;
		window_settings.borderless = window.mBorderless;
		window_settings.resizable = window.mResizable;
		window_settings.title = window.mTitle;
		window_settings.sync = window.mSync;
		window_settings.highdpi = mSettings.mEnableHighDPIMode;

#if 0
			// The primary window always exists. This is necessary to initialize openGL, and we need a window and an associated GL context for creating
			// resources before a window resource becomes available. The first RenderWindow that is created will share the primary window with the Renderer.
			// The settings that are passed here are applied to the primary window.
			// Because of the nature of the real-time editing system, when Windows are edited, new Windows will be created before old Windows are destroyed.
			// We need to make sure that an edit to a RenderWindow that was previously created as primary window will not create a new window, but share the 
			// existing primary window. We do this by testing if the IDs are the same.
			if (mPrimaryWindow.unique() || mPrimaryWindowID == inID)
			{
				mPrimaryWindowID = inID;
				mPrimaryWindow->applySettings(window_settings);
				return mPrimaryWindow;
			}
#endif

		// Construct and return new window
		std::shared_ptr<GLWindow> new_window = std::make_shared<GLWindow>();
		if (!new_window->init(window_settings, *this, errorState))
			return nullptr;

		mWindows.push_back(&window);

		// After window creation, make sure the primary window stays active, so that render resource creation always goes to that context
		//getPrimaryWindow().makeCurrent();

		return new_window;
	}


	void RenderService::removeWindow(RenderWindow& window)
	{
		WindowList::iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val == &window;
		});

		assert(pos != mWindows.end());
		mWindows.erase(pos);
	}


	RenderWindow* RenderService::findWindow(void* nativeWindow) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val->getWindow()->getNativeWindow() == nativeWindow;
		});

		if (pos != mWindows.end())
			return *pos;
		return nullptr;
	}


	RenderWindow* RenderService::findWindow(uint id) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val)
		{
			return val->getNumber() == id;
		});

		if (pos != mWindows.end())
			return *pos;
		return nullptr;
	}

	void RenderService::addEvent(WindowEventPtr windowEvent)
	{
		nap::Window* window = findWindow(windowEvent->mWindow);
		assert(window != nullptr);
		window->addEvent(std::move(windowEvent));
	}

	void createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkRenderPass& renderPass)
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = colorFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	VkPrimitiveTopology getTopology(const IMesh& inMesh)
	{
		switch (inMesh.getMeshInstance().getDrawMode())
		{
		case EDrawMode::POINTS:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case EDrawMode::LINES:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case EDrawMode::LINE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case EDrawMode::TRIANGLES:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case EDrawMode::TRIANGLE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case EDrawMode::TRIANGLE_FAN:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		default:
		{
			assert(false);
			return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}
		}
	}

	VkPipelineDepthStencilStateCreateInfo getDepthStencilCreateInfo(MaterialInstance& materialInstance)
	{
		VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_FALSE;
		depth_stencil.depthWriteEnable = VK_FALSE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.stencilTestEnable = VK_FALSE;

		EDepthMode depth_mode = materialInstance.getDepthMode();

		// If the depth mode is inherited from the blend mode, determine the correct depth mode to use
		if (depth_mode == EDepthMode::InheritFromBlendMode)
		{
			if (materialInstance.getBlendMode() == EBlendMode::Opaque)
				depth_mode = EDepthMode::ReadWrite;
			else
				depth_mode = EDepthMode::ReadOnly;
		}

		switch (depth_mode)
		{
		case EDepthMode::ReadWrite:
		{
			depth_stencil.depthTestEnable = VK_TRUE;
			depth_stencil.depthWriteEnable = VK_TRUE;
			break;
		}
		case EDepthMode::ReadOnly:
		{
			depth_stencil.depthTestEnable = VK_TRUE;
			depth_stencil.depthWriteEnable = VK_FALSE;
			break;
		}
		case EDepthMode::WriteOnly:
		{
			depth_stencil.depthTestEnable = VK_FALSE;
			depth_stencil.depthWriteEnable = VK_TRUE;
			break;
		}
		case EDepthMode::NoReadWrite:
		{
			depth_stencil.depthTestEnable = VK_FALSE;
			depth_stencil.depthWriteEnable = VK_FALSE;
			break;
		}
		default:
			assert(false);
		}

		return depth_stencil;
	}


	VkPipelineColorBlendAttachmentState getColorBlendAttachmentState(MaterialInstance& materialInstance)
	{
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

		EBlendMode blend_mode = materialInstance.getBlendMode();

		switch (blend_mode)
		{
		case EBlendMode::Opaque:
		{
			color_blend_attachment_state.blendEnable = VK_FALSE;
			break;
		}
		case EBlendMode::AlphaBlend:
		{
			color_blend_attachment_state.blendEnable = VK_TRUE;
			color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		}
		case EBlendMode::Additive:
		{
			color_blend_attachment_state.blendEnable = VK_TRUE;
			color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			break;
		}
		}

		return color_blend_attachment_state;
	}


	bool createGraphicsPipeline(VkDevice device, MaterialInstance& materialInstance, const IMesh& mesh, VkRenderPass renderPass, VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline, utility::ErrorState& errorState)
	{
		Material& material = materialInstance.getMaterial();

		const Shader& shader = material.getShader();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		uint32_t shader_attribute_binding = 0;
		for (auto& kvp : shader.getAttributes())
		{
			const VertexAttributeDeclaration* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader().mVertPath.c_str(), material.mID.c_str()))
				return false;

			const VertexAttributeBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(vertex_buffer != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return false;

			if (!errorState.check(shader_vertex_attribute->mFormat == vertex_buffer->getFormat(), "Shader vertex attribute format does not match mesh attribute format for attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return false;

			bindingDescriptions.push_back({ shader_attribute_binding, (uint32_t)getVertexSize(shader_vertex_attribute->mFormat), VK_VERTEX_INPUT_RATE_VERTEX });
			attributeDescriptions.push_back({ (uint32_t)shader_vertex_attribute->mLocation, shader_attribute_binding, shader_vertex_attribute->mFormat, 0 });

			shader_attribute_binding++;
		}

		VkShaderModule vertShaderModule = shader.getVertexModule();
		VkShaderModule fragShaderModule = shader.getFragmentModule();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = (int)bindingDescriptions.size();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = getTopology(mesh);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamic_states[2] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.dynamicStateCount = 2;
		dynamic_state_create_info.pDynamicStates = dynamic_states;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = getDepthStencilCreateInfo(materialInstance);
		VkPipelineColorBlendAttachmentState colorBlendAttachment = getColorBlendAttachmentState(materialInstance);

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkDescriptorSetLayout set_layout = material.getShader().getDescriptorSetLayout();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &set_layout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (!errorState.check(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout"))
			return false;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.pDynamicState = &dynamic_state_create_info;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (!errorState.check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) == VK_SUCCESS, "Failed to create graphics pipeline"))
			return false;

		return true;
	}

	VkRenderPass RenderService::getOrCreateRenderPass(ERenderTargetFormat format)
	{
		VkRenderPass render_pass = nullptr;
		switch (format)
		{
			case ERenderTargetFormat::RGBA8:
			{
				if (mRenderPassRGBA8 == nullptr)
					createRenderPass(mDevice, VK_FORMAT_B8G8R8A8_SRGB, mDepthFormat, mRenderPassRGBA8);

				render_pass = mRenderPassRGBA8;
			}
			break;

			case ERenderTargetFormat::R8:
			{
				if (mRenderPassR8 == nullptr)
					createRenderPass(mDevice, VK_FORMAT_R8_SRGB, mDepthFormat, mRenderPassR8);

				render_pass = mRenderPassR8;
			}
			break;

			case ERenderTargetFormat::Depth:
			{
				if (mRenderPassDepth == nullptr)
					createRenderPass(mDevice, VK_FORMAT_D24_UNORM_S8_UINT, mDepthFormat, mRenderPassDepth);

				render_pass = mRenderPassDepth;
			}
			break;
		}

		assert(render_pass != nullptr);

		return render_pass;
	}

	nap::RenderableMesh RenderService::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		VkRenderPass render_pass = getOrCreateRenderPass(materialInstance.getMaterial().getShader().mOutputFormat);

		VkPipelineLayout layout;
		VkPipeline pipeline;
		if (!createGraphicsPipeline(mDevice, materialInstance, mesh, render_pass, layout, pipeline, errorState))
			return RenderableMesh();

		return RenderableMesh(mesh, materialInstance, layout, pipeline);
	}


	void RenderService::recreatePipeline(RenderableMesh& renderableMesh, VkPipelineLayout& layout, VkPipeline& pipeline)
	{
		VkRenderPass render_pass = getOrCreateRenderPass(renderableMesh.getMaterialInstance().getMaterial().getShader().mOutputFormat);

		VkPipeline old_pipeline = renderableMesh.getPipeline();

		utility::ErrorState errorState;
		if (!createGraphicsPipeline(mDevice, renderableMesh.getMaterialInstance(), renderableMesh.getMesh(), render_pass, layout, pipeline, errorState))
			return;

		mPipelinesToDestroy.push_back({ mCurrentFrameIndex, old_pipeline });
	}


	void RenderService::advanceToFrame(int frameIndex)
	{
		mCurrentFrameIndex = frameIndex;

		// Destroy pipelines for the new frame
		mPipelinesToDestroy.erase(std::remove_if(mPipelinesToDestroy.begin(), mPipelinesToDestroy.end(), [this, frameIndex](PipelineToDestroy& pipelineToDestroy)
		{
			if (pipelineToDestroy.mFrameIndex == mCurrentFrameIndex)
			{
				vkDestroyPipeline(mDevice, pipelineToDestroy.mPipeline, nullptr);
				return true;
			}

			return false;
		}), mPipelinesToDestroy.end());

		for (auto& kvp : mDescriptorSetCaches)
			kvp.second->release(mCurrentFrameIndex);
	}


	void RenderService::processEvents()
	{
		for (const auto& window : mWindows)
		{
			window->processEvents();
		}
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera)
	{
		renderObjects(renderTarget, camera, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const SortFunction& sortFunction)
	{
		// Get all render-able components
		// Only gather renderable components that can be rendered using the given caera
		std::vector<nap::RenderableComponentInstance*> render_comps;
		std::vector<nap::RenderableComponentInstance*> entity_render_comps;
		for (Scene* scene : mSceneService->getScenes())
		{
			for (EntityInstance* entity : scene->getEntities())
			{
				entity_render_comps.clear();
				entity->getComponentsOfType<nap::RenderableComponentInstance>(entity_render_comps);
				for (const auto& comp : entity_render_comps)
				{
					if (comp->isSupported(camera))
						render_comps.emplace_back(comp);
				}
			}
		}

		// Render these objects
		renderObjects(renderTarget, camera, render_comps, sortFunction);
	}


	void RenderService::sortObjects(std::vector<RenderableComponentInstance*>& comps, const CameraComponentInstance& camera)
	{
		// Split into front to back and back to front meshes
		std::vector<nap::RenderableComponentInstance*> front_to_back;
		front_to_back.reserve(comps.size());
		std::vector<nap::RenderableComponentInstance*> back_to_front;
		back_to_front.reserve(comps.size());

		for (nap::RenderableComponentInstance* component : comps)
		{
			nap::RenderableMeshComponentInstance* renderable_mesh = rtti_cast<RenderableMeshComponentInstance>(component);
			if (renderable_mesh != nullptr)
			{
				nap::RenderableMeshComponentInstance* renderable_mesh = static_cast<RenderableMeshComponentInstance*>(component);
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.emplace_back(component);
				else
					front_to_back.emplace_back(component);
			}
			else
			{
				front_to_back.emplace_back(component);
			}
		}

		// Sort front to back and render those first
		DepthSorter front_to_back_sorter(DepthSorter::EMode::FrontToBack, camera.getViewMatrix());
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

		// Then sort back to front and render these
		DepthSorter back_to_front_sorter(DepthSorter::EMode::BackToFront, camera.getViewMatrix());
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

		// concatinate both in to the output
		comps.clear();
		comps.insert(comps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
		comps.insert(comps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
	}


	// Renders all available objects to a specific renderTarget.
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps)
	{
		renderObjects(renderTarget, camera, comps, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction)
	{
		assert(mCurrentCommandBuffer != nullptr);	// BeginRendering is not called if this assert is fired	

		// Sort objects to render
		std::vector<RenderableComponentInstance*> components_to_render = comps;
		sortFunction(components_to_render, camera);

		// Before we render, we always set aspect ratio. This avoids overly complex
		// responding to various changes in render target sizes.
		camera.setRenderTargetSize(renderTarget.getSize());

		// Make sure we update our render state associated with the current context
		//updateRenderState();

		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getProjectionMatrix();

		// Extract view matrix
		glm::mat4x4 view_matrix = camera.getViewMatrix();

		// Draw components only when camera is supported
		for (auto& comp : components_to_render)
		{
			if (!comp->isSupported(camera))
			{
				nap::Logger::warn("unable to render component: %s, unsupported camera %s",
					comp->mID.c_str(), camera.get_type().get_name().to_string().c_str());
				continue;
			}
			comp->draw(renderTarget, mCurrentCommandBuffer, view_matrix, projection_matrix);
		}
	}


	bool RenderService::initEmptyTexture(nap::utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = 16;
		settings.mHeight = 16;
		settings.mChannels = ESurfaceChannels::RGBA;
		settings.mDataType = ESurfaceDataType::BYTE;
		mEmptyTexture = std::make_unique<Texture2D>(getCore());
		if (!mEmptyTexture->init(settings, false, errorState))
			return false;

		std::vector<uint8_t> empty_texture_data;
		empty_texture_data.resize(settings.getSizeInBytes());

		mEmptyTexture->update(empty_texture_data.data(), settings.mWidth, settings.mHeight, settings.getPitch(), settings.mChannels);

		return true;
	}


	// Set the currently active renderer
	bool RenderService::init(nap::utility::ErrorState& errorState)
	{
		// Get handle to scene service
		mSceneService = getCore().getService<SceneService>();
		assert(mSceneService != nullptr);

		if (!errorState.check(SDL::initVideo(), "Failed to init SDL"))
			return false;

		if (!errorState.check(ShInitialize() != 0, "Failed to initialize shader compiler"))
			return false;

		// Store render settings, used for initialization and global window creation
		mSettings = getConfiguration<RenderServiceConfiguration>()->mSettings;

		// Get available vulkan extensions, necessary for interfacing with native window
		// SDL takes care of this call and returns, next to the default VK_KHR_surface a platform specific extension
		// When initializing the vulkan instance these extensions have to be enabled in order to create a valid
		// surface later on.
		std::vector<std::string> found_extensions;
		SDL_Window* dummy_window = SDL_CreateWindow("Dummy", 0, 0, 32, 32, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
		bool got_extensions = getAvailableVulkanExtensions(dummy_window, found_extensions, errorState);
		SDL_DestroyWindow(dummy_window);

		if (!got_extensions)
			return false;

		// Get available vulkan layer extensions, notify when not all could be found
		std::vector<std::string> found_layers;
		if (!getAvailableVulkanLayers(found_layers, errorState))
			return false;

		// Warn when not all requested layers could be found
		if (!errorState.check(found_layers.size() == getRequestedLayerNames().size(), "Not all requested layers were found"))
			return false;

		// Create Vulkan Instance
		if (!createVulkanInstance(found_layers, found_extensions, mInstance, errorState))
			return false;

		// Vulkan messaging callback
		setupDebugCallback(mInstance, mDebugCallback, errorState);

		// Select GPU after succsessful creation of a vulkan instance
		if (!selectGPU(mInstance, mPhysicalDevice, mPhysicalDeviceVersion, mGraphicsQueueIndex, errorState))
			return false;

		// Create a logical device that interfaces with the physical device
		if (!createLogicalDevice(mPhysicalDevice, mGraphicsQueueIndex, found_layers, mDevice, errorState))
			return false;

		if (!errorState.check(createCommandPool(mPhysicalDevice, mDevice, mGraphicsQueueIndex, mCommandPool), "Failed to create commandpool"))
			return false;

		if (!errorState.check(findDepthFormat(mPhysicalDevice, mDepthFormat), "Unable to find depth format"))
			return false;

		vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

		mDescriptorSetAllocator = std::make_unique<DescriptorSetAllocator>(mDevice);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = mPhysicalDevice;
		allocatorInfo.device = mDevice;

		if (!errorState.check(vmaCreateAllocator(&allocatorInfo, &mVulkanAllocator) == VK_SUCCESS, "Failed to create Vulkan Memory Allocator"))
			return false;

		if (!initEmptyTexture(errorState))
			return false;

		return true;
	}


	bool RenderService::beginRendering(RenderWindow& renderWindow)
	{
		assert(mCurrentCommandBuffer == nullptr);
		assert(mCurrentRenderWindow == nullptr);

		mCurrentCommandBuffer = renderWindow.makeActive();
		if (mCurrentCommandBuffer == nullptr)
			return false;

		mCurrentRenderWindow = &renderWindow;
		int frame_index = renderWindow.getWindow()->getCurrentFrameIndex();
		advanceToFrame(frame_index);

		for (Texture2D* texture : mTexturesToUpdate)
			texture->upload(mCurrentCommandBuffer);

		mTexturesToUpdate.clear();
		renderWindow.beginRenderPass();

		return true;
	}

	void RenderService::endRendering()
	{
		assert(mCurrentCommandBuffer != nullptr);
		assert(mCurrentRenderWindow != nullptr);

		mCurrentRenderWindow->swap();

		mCurrentCommandBuffer = nullptr;
		mCurrentRenderWindow = nullptr;
	}

	void RenderService::preUpdate(double deltaTime)
	{
		//	getPrimaryWindow().makeCurrent();
	}


	void RenderService::update(double deltaTime)
	{
		processEvents();
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
		SDL::shutdownVideo();
	}


	DescriptorSetCache& RenderService::getOrCreateDescriptorSetCache(VkDescriptorSetLayout layout)
	{
		DescriptorSetCacheMap::iterator pos = mDescriptorSetCaches.find(layout);
		if (pos != mDescriptorSetCaches.end())
			return *pos->second;

		std::unique_ptr<DescriptorSetCache> allocator = std::make_unique<DescriptorSetCache>(*this, layout, *mDescriptorSetAllocator);
		auto inserted = mDescriptorSetCaches.insert(std::make_pair(layout, std::move(allocator)));
		return *inserted.first->second;
	}


	void RenderService::requestTextureUpdate(Texture2D& texture)
	{
		mTexturesToUpdate.insert(&texture);
	}

} // Renderservice

