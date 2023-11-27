#pragma once
#include <vector>
#include <filesystem>

namespace fbxsdk
{
	class FbxNode;
	class FbxScene;
}

namespace TGA::FBX
{
	struct Vec3
	{
		float Value[3];
	};

	struct Quat
	{
		float Value[4];
	};

	struct Tranform
	{
		Vec3 Position = {0};
		Quat Quaterion = { 0 };
		Vec3 Scale = { 0 };
	};

	struct Mesh
	{
		std::vector<Vec3> VerticesBuffer;
		std::vector<uint32_t> IndicesBuffer;

	};

	struct Node
	{
		std::string_view Name;
		Tranform Transform;
		Mesh Mesh;
	};

	struct Scene
	{
		std::vector<Node> Nodes;
		void AddNode(Node& node)
		{
			Nodes.push_back(node);
		}
	};

	class Exporter
	{
	public:
		void ExportScene(Scene* aScene, const std::filesystem::path& aPath);
	private:
		void AddToScene(Node* node, fbxsdk::FbxNode* aFbxNode, fbxsdk::FbxScene* aScene);
	};
}
