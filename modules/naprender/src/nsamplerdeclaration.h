#pragma once

// External Includes
#include <vector>

#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

namespace nap
{
	class NAPAPI SamplerDeclaration
	{
		RTTI_ENABLE()

	public:
		enum class EType : uint8_t
		{
			Type_1D,
			Type_2D,
			Type_3D
		};

		SamplerDeclaration(const std::string& name, int binding, VkShaderStageFlagBits stage, EType type, int numArrayElements) :
			mName(name),
			mBinding(binding),
			mStage(stage),
			mType(type),
			mNumArrayElements(numArrayElements)
		{
		}

		std::string				mName;
		int						mBinding = -1;
		VkShaderStageFlagBits	mStage;
		EType					mType = EType::Type_2D;
		int						mNumArrayElements = 1;
	};

	using SamplerDeclarations = std::vector<SamplerDeclaration>;
}
