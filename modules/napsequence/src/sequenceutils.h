#pragma once

// internal includes
#include "sequence.h"

namespace nap
{
	namespace sequenceutils
	{
		//////////////////////////////////////////////////////////////////////////

		/**
		 * generated a unique id
		 * @param objectIDs reference to collections of id's 
		 * @param baseID base id
		 */
		const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated");


		/**
		 * creates a default sequence
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @return a raw pointer to the newly created sequence
		 */
		Sequence* createEmptySequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
	}
}
