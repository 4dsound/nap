#pragma once

// External Includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents a group of parameters that can be blended over time by a nap::ParameterBlendComponent
	 * All given parameters need to be part of the root group, either as a direct sibling or 
	 * contained within a group that is part of the root group.
	 */
	class NAPAPI ParameterBlendGroup : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~ParameterBlendGroup();

		/**
		 * Ensures that every parameter specified in the 'Parameters' property is part of the 'RootGroup'.
		 * Either as a direct sibling or contained within a group that is part of the root group.
		 * @param errorState contains the error message when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<ResourcePtr<Parameter>> mParameters;						///< Property: 'Parameters' list of all parameters considered to blend
		nap::ResourcePtr<ParameterGroup> mRootGroup = nullptr;					///< Property: 'RootGroup' group all the blend parameters belong to
	};
}
