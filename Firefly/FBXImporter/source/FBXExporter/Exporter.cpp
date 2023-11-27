#include "TGAFbx.pch.h"
#include "Exporter.h"

#include <fbxsdk.h>
#include <span>

namespace TGA::FBX
{
	using namespace fbxsdk;
	void Exporter::ExportScene(Scene* aScene, const std::filesystem::path& aPath)
	{
		if (aScene == nullptr)
		{
			return;
		}

		FbxManager* sdkManager = FbxManager::Create();

		FbxIOSettings* settings = FbxIOSettings::Create(sdkManager, IOSROOT);

		settings->SetBoolProp(EXP_FBX_MATERIAL, true);
		settings->SetBoolProp(EXP_FBX_TEXTURE, true);
		
		FbxScene* fbxScene = FbxScene::Create(sdkManager, "");

		FbxExporter* exporter = FbxExporter::Create(sdkManager, "exporter");

		FbxNode* rootNode = FbxNode::Create(fbxScene, "RootNode");
		for (auto& node : aScene->Nodes)
		{
			FbxNode* childNode = FbxNode::Create(fbxScene, "ChildNode");
			rootNode->AddChild(childNode);

			AddToScene(&node, childNode, fbxScene);
		}
		fbxScene->GetRootNode()->AddChild(rootNode);

		FbxAxisSystem axisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
		axisSystem.ConvertScene(fbxScene);

		exporter->Initialize(aPath.string().c_str(), -1, settings);

		exporter->Export(fbxScene);
	}

	void Exporter::AddToScene(Node* node, fbxsdk::FbxNode* aFbxNode, fbxsdk::FbxScene* aScene)
	{
		fbxsdk::FbxMesh* mesh = fbxsdk::FbxMesh::Create(aScene, node->Name.data());

		mesh->InitControlPoints(node->Mesh.VerticesBuffer.size());
		for (size_t i = 0; auto& vert : node->Mesh.VerticesBuffer)
		{
			mesh->SetControlPointAt(FbxVector4(vert.Value[0], vert.Value[1], vert.Value[2]), i);
			i++;
		}
		auto span = std::span(node->Mesh.IndicesBuffer);
		for (size_t i = 0; i < span.size(); i += 3)
		{
			auto thisSpan = span.subspan(i, i + 3);
			mesh->BeginPolygon();
			mesh->AddPolygon(thisSpan[0]);
			mesh->AddPolygon(thisSpan[1]);
			mesh->AddPolygon(thisSpan[2]);
			mesh->EndPolygon();
		}
		aFbxNode->SetNodeAttribute(mesh);

		FbxAMatrix tranform;
		auto pos = fbxsdk::FbxVector4(node->Transform.Position.Value[0], node->Transform.Position.Value[1], node->Transform.Position.Value[2]);
		auto scale = fbxsdk::FbxVector4(node->Transform.Scale.Value[0], node->Transform.Scale.Value[1], node->Transform.Scale.Value[2]);
		auto quat = fbxsdk::FbxQuaternion(node->Transform.Quaterion.Value[0], node->Transform.Quaterion.Value[1], node->Transform.Quaterion.Value[2], node->Transform.Quaterion.Value[3]);
		tranform.SetTQS(pos, quat, scale);

		aFbxNode->LclTranslation.Set(tranform.GetT());
		aFbxNode->LclRotation.Set(tranform.GetR());
		aFbxNode->LclScaling.Set(tranform.GetS());
	}
}