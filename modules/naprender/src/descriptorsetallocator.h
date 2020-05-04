#pragma once

// Internal includes

// External Includes
#include <unordered_map>
#include "vulkan/vulkan_core.h"

namespace nap
{
	struct DescriptorPool;

	/**
	 * Allocates DescriptorSets from a pool. Each pool is bound to a specific combination of UBOs and samplers,
	 * as allocates resources for each UBO and sampler as well as the DescriptorSet itself. Any shader that has 
	 * the same amount of UBOs and samplers can therefore allocate from the same pool.
	 * So, we maintain a map that holds a key that is a combination of sampler/UBO count. As a value, there is a 
	 * list of pools, because pools are preallocated with a fixed size of sets to allocate from. If the pool is
	 * full, we need to allocate a new pool.
	 * So essentially we're managing a pool of DescriptorSetPools here.
	 */
	class DescriptorSetAllocator
	{
	public:
		DescriptorSetAllocator(VkDevice device);
		~DescriptorSetAllocator();

		/**
		 * Allocate a DescriptorSetLayout that is compatible with VkDescriptorSetLayout (same amount of UBOs and samplers).
		 */
		VkDescriptorSet allocate(VkDescriptorSetLayout layout, int numUBODescriptors, int numSamplerDescriptors);

	private:
		using DescriptorPoolMap = std::unordered_map<uint64_t, std::vector<DescriptorPool>>;

		VkDevice					mDevice;
		DescriptorPoolMap			mDescriptorPools;
	};

} // nap



