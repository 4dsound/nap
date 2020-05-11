// local includes
#include "sequenceutils.h"
#include "sequence.h"
#include "sequencetrackevent.h"
#include "sequencetrackcurve.h"

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace sequenceutils
	{
		const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID)
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}

		Sequence* createEmptySequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs)
		{
			// create the sequence
			std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
			sequence->mID = sequenceutils::generateUniqueID(objectIDs);
			sequence->mDuration = 1.0;

			//
			Sequence* returnPtr = sequence.get();

			// move ownership
			createdObjects.emplace_back(std::move(sequence));

			// finally return
			return returnPtr;
		}
	}
}
