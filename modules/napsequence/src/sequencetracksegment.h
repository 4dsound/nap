#pragma once

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <fcurve.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceTrackSegment
	 * Base class for track segments. Holds information about the start time of the segment in the track. And the duration.
	 */
	class NAPAPI SequenceTrackSegment : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * init evaluates the data of the segment
		 * @param errorState contains information about eventual failure of evaluation
		 * @return true if data valid
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	public:
		double mStartTime = 0.0; ///< Property: 'Start time' start time of segment in track
		double mDuration = 1.0; ///< Property: 'Duration' duration of segment
	};
}
