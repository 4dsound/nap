#include "parameterblender.h"
#include <mathutils.h>
#include <nap/logger.h>

RTTI_DEFINE_BASE(nap::BaseParameterBlender)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterFloatBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterDoubleBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterVec2Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterVec3Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Contains all the available parameter blenders
	 */
	static std::unordered_map<rtti::TypeInfo, rtti::TypeInfo> sBlendMap
	{
		{ RTTI_OF(ParameterFloat),			RTTI_OF(ParameterFloatBlender)  },
		{ RTTI_OF(ParameterDouble),			RTTI_OF(ParameterDoubleBlender) },
		{ RTTI_OF(ParameterVec2),			RTTI_OF(ParameterVec2Blender)	},
		{ RTTI_OF(ParameterVec3),			RTTI_OF(ParameterVec3Blender)	}
	};


	std::unique_ptr<BaseParameterBlender> NAPAPI getParameterBlender(Parameter& param)
	{
		// Find in map
		auto it = sBlendMap.find(param.get_type());
		if (it == sBlendMap.end())
			return nullptr;

		// Create blender
		nap::BaseParameterBlender* blender = it->second.create<BaseParameterBlender>({ param });
		return std::unique_ptr<BaseParameterBlender>(blender);
	}


	bool registerParameterBlender(rtti::TypeInfo inParameterType, rtti::TypeInfo inBlenderType)
	{
		if (!inParameterType.is_derived_from(RTTI_OF(nap::Parameter)))
		{
			nap::Logger::warn("Unable to register parameter blender, parameter %s: not derived from %s", inParameterType.get_name().to_string().c_str(),
				RTTI_OF(Parameter).get_name().to_string().c_str());
			return false;
		}

		if (!inBlenderType.is_derived_from(RTTI_OF(nap::BaseParameterBlender)))
		{
			nap::Logger::warn("Unable to register parameter blender, blender %s: not derived from %s", inBlenderType.get_name().to_string().c_str(),
				RTTI_OF(BaseParameterBlender).get_name().to_string().c_str());
			return false;
		}

		auto it = sBlendMap.emplace(std::make_pair(inParameterType, inBlenderType));
		if (!(it.second))
		{
			nap::Logger::warn("Unable to register parameter blender, parameter %s: duplicate entry", 
				inParameterType.get_name().to_string().c_str());
			return false;
		}
		return true;
	}


	BaseParameterBlender::BaseParameterBlender(Parameter& parameter) : mParameter(&parameter)
	{ }


	void BaseParameterBlender::blend(float value)
	{
		assert(mTarget != nullptr);
		onBlend(math::clamp<float>(value, 0.0f, 1.0f));
	}


	void BaseParameterBlender::setTarget(const Parameter* target)
	{
		// Ensure target is derived from parameter
		if (!target->get_type().is_derived_from(mParameter->get_type()))
		{
			assert(false);
			return;
		}

		// Update target and call derived classes
		mTarget = target;
		onTargetSet();
	}


	void BaseParameterBlender::clearTarget()
	{
		mTarget = nullptr;
	}


	void BaseParameterBlender::sync()
	{
		onSync();
	}


	bool BaseParameterBlender::hasTarget() const
	{
		return mTarget != nullptr;
	}


	const nap::Parameter& BaseParameterBlender::getTarget() const
	{
		assert(mTarget != nullptr); 
		return *mTarget;
	}


	const nap::Parameter& BaseParameterBlender::getParameter() const
	{
		assert(mParameter != nullptr); 
		return *mParameter;
	}
}