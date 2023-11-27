#pragma once
#include "Firefly/Rendering/Renderer.h"

#include <Utils/Math/Sphere.hpp>

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Rendering/Material/InternalMaterial.h"
#include "Firefly/Core/Core.h"
#include "Utils/Math/AABB3D.hpp"

namespace Firefly
{
	class Mesh;
	class MaterialAsset;
	class MeshComponent : public Component
	{
	public:
		MeshComponent();
		~MeshComponent() = default;

		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		static std::string GetFactoryName() { return "MeshComponent"; }
		static Ref<Component> Create() { return CreateRef<MeshComponent>(); }
		const std::vector<Ref<MaterialAsset>>& GetMaterials() { return myMaterials; }
		void SetMesh(std::string aMeshPath);
		void SetIsRender(const bool aBool) { myIsRender = aBool; }
		const Ref<Mesh>& GetMesh() { return myMesh; }
		std::string GetMeshPath() { return myMeshPath; }
		bool GetIsRender() { return myIsRender; }

		const Utils::Sphere<float>& GetBoundingSphere() const { return myModelInfo.BoundingSphere; }
		void SetShouldOutline(bool aShouldOutline = true);

		void LoadAssets();

		void SetMaterialPath(std::string_view aPath);

		void CreateBoundingSphere();
		//void SetMaterial(const std::string& aPath);

	private:
		void LoadMesh();
		void LoadMaterials();
		std::string myMeshPath;
		std::vector<std::string> myMaterialPaths;
		std::vector<Ref<MaterialAsset>> myMaterials;
		bool myIsRender{ true };
		Ref<Mesh> myMesh;
		MeshSubmitInfo myModelInfo;
		bool myShouldResizeMaterials;
		float myInitialRadius;

		bool myCreateBoundingOnce = true;
	};
}