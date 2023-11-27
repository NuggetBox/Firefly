#pragma once
#include <Utils/Math/Vector2.hpp>
#include <Utils/Math/Vector3.hpp>
#include <Utils/Math/Vector4.hpp>

#include "Firefly/Asset/Texture/Texture2D.h"
#include "Firefly/Core/DXHelpers.h"
#include <Firefly/Rendering/Material/InternalMaterial.h>


namespace Firefly
{
	
	class MaterialAsset : public Asset
	{
		friend class MaterialEditorWindow;
	public:
		void Init(const InternalMaterial& aInternalMaterial);
		bool operator==(MaterialAsset& other)
		{
			return myMatID == other.myMatID;
		}
		bool operator<(MaterialAsset& other)
		{
			return myMatID < other.myMatID;
		}
		template<class T>
		T& MaterialValue(const std::string& aName);
		void Bind(ID3D11DeviceContext* aContext);
		void BindWithPipeline(ID3D11DeviceContext* aContext);
		void UnBind(ID3D11DeviceContext* aContext);
		void UnbindWithPipeline(ID3D11DeviceContext* aContext);
		void AssignID(size_t aId);
		size_t GetID() { return myMatID; }
		InternalMaterial& GetInfo() { return myMatInfo; }

		static AssetType GetStaticType() { return AssetType::Material; }
		inline AssetType GetAssetType() const override { return GetStaticType(); }

		void SetShouldBlend(const bool aBlend);

	private:
		size_t myMatID;
		InternalMaterial myMatInfo = {};
	};
	template<class T>
	inline T& MaterialAsset::MaterialValue(const std::string& aName)
	{
		size_t byteOffset = 0;
		for (auto & varible : myMatInfo.MaterialData.varibles)
		{
			if (aName == varible.Name)
			{
				break;
			}

			byteOffset += SizeOfReflectedValue(varible.VariableType);
		}
		return *(T*)&myMatInfo.MaterialData.data[byteOffset];
	}
}