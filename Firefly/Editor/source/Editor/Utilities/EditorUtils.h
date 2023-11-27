#pragma once
#include <Utils/Math/Vector3.hpp>

#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
}

namespace EditorUtils
{
	/// <summary>
	/// File Ending should not have a dot
	/// </summary>
	/// <param name="aFilter"></param>
	/// <param name="fileEnding"></param>
	/// <returns></returns>
	std::filesystem::path GetSaveFilePath(const char* aFilter, const char* fileEnding);
	std::filesystem::path GetOpenFilePath(const char* aFilter);

	std::filesystem::path GetFolder();

	bool IsOnlySamePrefabChildrenSelected();
	bool IsAnySamePrefabChildrenSelected(const Ptr<Firefly::Entity>& aEntity);

	int GetParentCount(const Ptr<Firefly::Entity>& aEntity);

	/// <summary>
	/// UNSAFE FUCTION, ONLY USE IF YOU KNOW WHAT YOU ARE DOING
	/// </summary>
	/// <param name="aActive"></param>
	void SetImGuiPayloadActive(bool aActive);

	//Gets the path relative to the working directory (AssetData/) for a full path
	const std::filesystem::path GetRelativePath(const std::filesystem::path& aFullPath);

	void CreateUserFolder();

	bool AcceptAllDragDrops(const Utils::Vector3f& aPosition = {});

	bool AcceptSceneToAdd();

	bool AcceptPrefabToAddToScene(const Utils::Vector3f& aPosition = {});
	bool AcceptMeshToAddToScene(const Utils::Vector3f& aPosition = {});
	bool AcceptSkeletonToAddToScene(const Utils::Vector3f& aPosition = {});
	bool AcceptAnimationToAddToScene(const Utils::Vector3f& aPosition = {});
	bool AcceptEmitterToAddToScene(const Utils::Vector3f& aPosition = {});
}
