#include "FFpch.h"
#include "AvatarMask.h"
#include "nlohmann/json.hpp"

#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Mesh/AnimatedMesh.h"

namespace Firefly
{
	void AvatarMask::AddBoneToIgnore(uint32_t aBoneID)
	{
		myBonesToIgnore.insert(aBoneID);
	}
	void AvatarMask::RemoveBoneToIgnore(uint32_t aBoneID)
	{
		myBonesToIgnore.erase(aBoneID);
	}
	bool AvatarMask::IsLoaded() const
	{
		if (!myAnimatedMesh)
		{
			return false;
		}
		return myIsLoaded && myAnimatedMesh->IsLoaded();
	}

	void AvatarMask::SaveTo(const std::filesystem::path& aPath)
	{
		nlohmann::json json;
		int index = 0;
		for (auto boneID : myBonesToIgnore)
		{
			json["BonesToIgnore"][index++] = boneID;
		}

		index = 0;
		for (auto influencePair : myInfluences)
		{
			json["Influences"][index]["TargetBone"] = influencePair.first;
			json["Influences"][index]["Value"] = influencePair.second;
			index++;
		}

		json["AnimatedMesh"] = myAnimatedMesh->GetPath().string();
		std::ofstream file(aPath);
		file << std::setw(4) << json;
	}

	void AvatarMask::SetInfluence(uint32_t aBone, float aInfluence)
	{
		myInfluences[aBone] = aInfluence;
	}

	float AvatarMask::GetInfluence(uint32_t aBone)
	{
		if (myInfluences.contains(aBone))
		{
			return myInfluences[aBone];
		}
		else
		{
			return 1.0f;
		}
	}



}