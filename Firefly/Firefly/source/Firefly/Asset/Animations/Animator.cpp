#include "FFpch.h"
#include "Animator.h"

namespace Firefly
{
	int Animator::GetLayerIndex(const std::string& aLayerName) const
	{
		for (int i = 0; i < myLayers.size(); i++)
		{
			if (myLayers[i].GetName() == aLayerName)
			{
				return i;
			}
		}
		LOGERROR("Could not find layer with name: \"{}\". In animator with path \"{}\"", aLayerName, myPath.string());
		return -1;
	}
}