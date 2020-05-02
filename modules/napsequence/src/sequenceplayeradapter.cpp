#include "sequenceplayeradapter.h"
#include "sequenceplayer.h"

namespace nap
{
	std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc>& SequencePlayerAdapter::getFactoryMap()
	{
		static std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc> map;
		return map;
	}


	bool SequencePlayerAdapter::registerFactory(rttr::type type, SequencePlayerAdapterFactoryFunc factory)
	{
		auto& map = getFactoryMap();
		if (map.size() > 0)
		{
			auto it = map.find(type);
			assert(it == map.end()); // duplicate entry
			if (it == map.end())
			{
				map.emplace(type, factory);
				return true;
			}
		}
		else
		{
			map.emplace(type, factory);
			return true;
		}

		return false;
	}


	std::unique_ptr<SequencePlayerAdapter> SequencePlayerAdapter::invokeFactory(rttr::type type, SequenceTrack& track, SequencePlayerInput& input)
	{
		auto& map = getFactoryMap();

		auto it = map.find(type);
		assert(it != map.end()); // factory method not present
		if (it != map.end())
		{
			return it->second(track, input);
		}

		return nullptr;
	}
}
