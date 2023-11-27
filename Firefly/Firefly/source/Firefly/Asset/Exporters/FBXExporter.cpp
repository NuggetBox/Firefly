#include "FFpch.h"
#include "FBXExporter.h"
#include "Firefly/Asset/Mesh/Mesh.h"
#include "Firefly/ComponentSystem/Scene.h"
#include "FBXExporter/Exporter.h"
#include "Utils/Math/Transform.h"
namespace Firefly
{
	void FBXExporter::ExportScene(Ref<Scene> aScene, const std::filesystem::path& aExportPath)
	{
		
	}
	void FBXExporter::ExportMeshPackage(std::vector<std::pair<Ref<Mesh>, Utils::Transform>> aMeshes, const std::filesystem::path& aExportPath)
	{
		TGA::FBX::Scene scene;

		for (auto& [mesh, tranform] : aMeshes)
		{
			TGA::FBX::Node node;

			node.Name = mesh->GetPath().stem().string();
			auto& subMesh = mesh->GetSubMeshes()[0];

			std::vector<TGA::FBX::Vec3> verts(subMesh.GetVertices().size());

			for (size_t i = 0; auto & vert : verts)
			{
				memcpy(&vert.Value[0], &subMesh.GetVerticesPositions()[i].x, sizeof(float) * 3);
				i++;
			}

			node.Mesh.VerticesBuffer = verts;
			node.Mesh.IndicesBuffer = subMesh.GetIndices();
			auto basic = tranform.ToBasic();

			node.Transform.Position.Value[0] = basic.GetPosition().x;
			node.Transform.Position.Value[1] = basic.GetPosition().y;
			node.Transform.Position.Value[2] = basic.GetPosition().z;

			node.Transform.Quaterion.Value[0] = basic.GetQuaternion().x;
			node.Transform.Quaterion.Value[1] = basic.GetQuaternion().y;
			node.Transform.Quaterion.Value[2] = basic.GetQuaternion().z;
			node.Transform.Quaterion.Value[3] = basic.GetQuaternion().w;

			node.Transform.Scale.Value[0] = basic.GetScale().x;
			node.Transform.Scale.Value[1] = basic.GetScale().y;
			node.Transform.Scale.Value[2] = basic.GetScale().z;

			scene.AddNode(node);
		}

		TGA::FBX::Exporter exporter;
		exporter.ExportScene(&scene, aExportPath);
	}
}