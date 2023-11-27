#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>

namespace Firefly
{
	// its all about the bits!

	// this is a runtime type of Enum system.
	class PhysicsLayerHandler
	{
	public:
		static void Initialize();

		static uint32_t GetLayersToCollideWith(uint32_t aFilter);

		static std::string GetNameOfLayer(uint32_t aFilter);

		static void SetLayerActive(uint32_t aCurrentFilter, uint32_t aFilterToSet, bool aActive);
		static bool GetLayerActive(uint32_t aCurrentFilter, uint32_t aFiltertoCheck);
		static void Add(uint32_t aFilterBit, std::string aName, uint32_t aFilter);
		static void Remove(uint32_t aFilterBit);

		static bool LayerExist(uint32_t aLayer);

		static std::vector<std::string> GetNamesOflayers();
		static std::vector<uint32_t> GetAllLayers();


		static void Save(const std::filesystem::path& aPath);

		static void Load(const std::filesystem::path& aPath);


	private:
		// layers is reprisented by bits in a bit set, so i find the filtered layer from the map.
		// example 1 0 0 0 0 0 0  might be filtered with 1 1 1 1 1 1 0 0 
		// OBS this map should not have more than one bit set in the first uint32_t i the map.
		static inline std::unordered_map<uint32_t, uint32_t> myBitLayerMap;

		// same as above but this holds the name of the bit for a more human way of reading the filter.
		static inline std::unordered_map<uint32_t, std::string> myNameMap;
	};
}