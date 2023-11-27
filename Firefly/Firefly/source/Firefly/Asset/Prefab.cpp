#include "FFpch.h"
#include "Prefab.h"

namespace Firefly
{

	void Prefab::Init(std::vector<std::shared_ptr<Entity>> someEntities, uint64_t aPrefabID)
	{
		myEntities = someEntities;
		myPrefabID = aPrefabID;
	}

}