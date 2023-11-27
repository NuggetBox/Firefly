#pragma once
#include <nlohmann/json.hpp>
#include "simdjson/simdjson.h"

namespace Firefly
{
	class Component;
	class Entity;
	struct Variable;
	class Scene;

	class SerializationUtils
	{
	public:
		static void SerializeScene(nlohmann::json& aJson, std::weak_ptr<Scene> aScene);
		static void SerializeEntity(nlohmann::json& aJson, std::weak_ptr<Entity> aEntity, bool aIsPrefab = false);
		static void SerializePrefabRoot(nlohmann::json& aJson, std::weak_ptr<Entity> aEntity);
		static void SerializePrefabChild(nlohmann::json& aJson, std::weak_ptr<Entity> aEntity);
		static void SerializeParameter(nlohmann::json& aJson, const Variable& aParameter, bool aIsPrefab = false);

		static void SaveEntityAsPrefab(std::weak_ptr<Entity> aEntity, const std::filesystem::path& aPath);
		static void SerializeEntityAsPrefab(nlohmann::json& aJson, std::weak_ptr<Entity> aEntity);

		static std::shared_ptr<Entity> DeserializePrefabRoot(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt);
		static  std::shared_ptr<Entity> DeserializePrefabChild(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt);
		static  std::shared_ptr<Entity> DeserializeEntity(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonEnt);
		static  std::shared_ptr<Scene> DeserializeScene(simdjson::simdjson_result<simdjson::fallback::ondemand::value> aJsonScene);

	private:
		static void CollectMeshesAndAnimationsPaths(std::weak_ptr<Component> aComponent, std::vector<std::string>& aMeshes, std::vector<std::string>& aAnimatedMeshes, std::vector<std::string>& aAnimations);
	};
}