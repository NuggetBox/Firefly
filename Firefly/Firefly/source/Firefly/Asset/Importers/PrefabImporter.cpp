#include "FFpch.h"
#include "PrefabImporter.h"
#include "SerializationUtils.h"


namespace Firefly
{
	bool PrefabImporter::ImportPrefab(Ref<Prefab> aAsset)
	{
		simdjson::ondemand::parser parser;
		auto loadPath = std::filesystem::current_path().string() + "\\" + aAsset->GetPath().string();
		auto json = simdjson::padded_string::load(loadPath);
		simdjson::ondemand::document doc = parser.iterate(json);

		auto jsonEntities = doc["Entities"];

		std::vector<std::shared_ptr<Entity>> entities;

		auto jsonEntArray = jsonEntities.get_array();
		for (auto jsonEnt : jsonEntArray)
		{
			entities.push_back(SerializationUtils::DeserializeEntity(jsonEnt));
		}

		entities.front()->GetTransform().SetPosition(0, 0, 0);

		std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>();
		aAsset->Init(entities, doc["PrefabID"].get_uint64());

		return true;
	}
	uint64_t PrefabImporter::ImportID(const std::filesystem::path& aPath)
	{
		simdjson::ondemand::parser parser;
		auto loadPath = std::filesystem::current_path().string() + "\\" + aPath.string();
		auto json = simdjson::padded_string::load(loadPath);
		simdjson::ondemand::document doc = parser.iterate(json);

		return doc["PrefabID"].get_uint64();
	}
}
