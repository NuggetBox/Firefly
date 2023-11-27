#include "FFpch.h"
#include "PhysicsLayerHandler.h"
#include "nlohmann/json.hpp"
#include <Firefly/Core/Log/DebugLogger.h>

namespace Firefly
{
	void PhysicsLayerHandler::Initialize()
	{
		Load("FireflyEngine\\Physics\\PhysicsLayers.json");
	}

	uint32_t PhysicsLayerHandler::GetLayersToCollideWith(uint32_t aFilter)
	{
		return myBitLayerMap[aFilter];
	}

	void PhysicsLayerHandler::SetLayerActive(uint32_t aCurrentFilter, uint32_t aLayerToSet, bool aActive)
	{
		if (aActive)
		{
			myBitLayerMap[aCurrentFilter] |= aLayerToSet;
		}
		else
		{
			myBitLayerMap[aCurrentFilter] ^= aLayerToSet;
		}
	}

	bool PhysicsLayerHandler::GetLayerActive(uint32_t aCurrentFilter, uint32_t aLayertoCheck)
	{
		return myBitLayerMap[aCurrentFilter] & aLayertoCheck;
	}

	void PhysicsLayerHandler::Add(uint32_t aLayerBit, std::string aName, uint32_t aLayer)
	{
		myBitLayerMap[aLayerBit] = aLayer;
		myNameMap[aLayerBit] = aName;
	}

	std::vector<std::string> PhysicsLayerHandler::GetNamesOflayers()
	{
		std::vector<std::string> names;

		for (auto& [layer, name] : myNameMap)
		{
			names.emplace_back(name);
		}
		return names;
	}

	std::vector<uint32_t> PhysicsLayerHandler::GetAllLayers()
	{
		std::vector<uint32_t> layers;
		for (auto& [layer, name] : myNameMap)
		{
			layers.emplace_back(layer);
		}
		return layers;
	}

	void PhysicsLayerHandler::Remove(uint32_t aLayerBit)
	{
		myNameMap.erase(aLayerBit);
		myBitLayerMap.erase(aLayerBit);
		for (auto& layer : myBitLayerMap)
		{
			// we should never remove layer from default layer
			if (layer.first == 1)
			{
				continue; 
			}
			// if the bit exists.
			if (layer.second & aLayerBit)
			{
				// adds the bit from the layer.second.
				layer.second |= aLayerBit;
			}
		}
	}

	bool PhysicsLayerHandler::LayerExist(uint32_t aLayer)
	{
		return myBitLayerMap.contains(aLayer);
	}

	void PhysicsLayerHandler::Save(const std::filesystem::path& aPath)
	{
		nlohmann::json data;
		size_t index = 0;
		for (auto& layer : myBitLayerMap)
		{
			auto strId = std::to_string(index);
			data[strId]["Layer"] = layer.first;
			data[strId]["Layers"] = layer.second;
			data[strId]["Name"] = myNameMap[layer.first];
			index++;
		}

		std::ofstream fout(aPath);
		fout << data;
	}

	std::string PhysicsLayerHandler::GetNameOfLayer(uint32_t aFilter)
	{
		return myNameMap[aFilter];
	}

	void PhysicsLayerHandler::Load(const std::filesystem::path& aPath)
	{
		std::ifstream fin(aPath);
		if (!fin.good())
		{
			LOGINFO("not fine");
			return;
		}
		nlohmann::json data = nlohmann::json::parse(fin);

		// it will never be more than 32 physics layers.
		for (size_t i = 0; i < 32; ++i)
		{
			auto indexInString = std::to_string(i);
			if (!data.contains(indexInString))
			{
				break;
			}

			uint32_t filter = (uint32_t)data[indexInString]["Layer"];
			uint32_t filters = (uint32_t)data[indexInString]["Layers"];
			std::string name = (std::string)data[indexInString]["Name"];
			Add(filter, name, filters);
		}
	}
}