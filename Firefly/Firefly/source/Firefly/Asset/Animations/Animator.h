#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Asset.h"
#include "Utils/Math/Vector2.hpp"
#include "Firefly/Asset/Animations/AnimatorLayer.h"

class AnimatorWindow;
namespace Firefly
{
	class Animator : public Asset
	{
	public:
		__forceinline const std::unordered_map<uint64_t, AnimatorParameter>& GetParameters() const { return myParameters; }
		__forceinline const std::vector<AnimatorLayer>& GetLayers() const { return myLayers; }
		__forceinline std::vector<AnimatorLayer>& GetLayersMutable() { return myLayers; }
		__forceinline const AnimatorParameter& GetParameter(uint64_t aID) const { return myParameters.at(aID); }
		__forceinline uint64_t GetID() const { return myID; }
		 int GetLayerIndex(const std::string& aLayerName) const;

		static AssetType GetStaticType() { return AssetType::Animator; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }
	private:
		friend class ::AnimatorWindow;
		friend class AnimatorImporter;
		uint64_t myID;
		std::unordered_map<uint64_t, AnimatorParameter> myParameters;
		std::vector<AnimatorLayer> myLayers;


		
		

	};

}