#pragma once
#include <string>
#include "Firefly/Asset/Asset.h"
#include "Firefly/ComponentSystem/Entity.h"

namespace Firefly
{
	class Prefab : public Asset
	{
	public:
		Prefab() = default;
		~Prefab() = default;
		static AssetType GetStaticType() { return AssetType::Prefab; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		void Init(std::vector<std::shared_ptr<Entity>> someEntities, uint64_t myPrefabID);

		const std::vector<std::shared_ptr<Entity>>& GetEntities() const { return myEntities; }
		uint64_t GetPrefabID() const { return myPrefabID; }

	private:
		std::vector<std::shared_ptr<Entity>>myEntities;
		uint64_t myPrefabID = 0;


	};
}