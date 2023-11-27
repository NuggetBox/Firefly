#include "FFpch.h"
#include "AnimatorLayer.h"
namespace Firefly
{
	AnimatorLayer::AnimatorLayer()
	{
	}

	uint64_t AnimatorLayer::GetStateID(const std::string& aName) const
	{
		for (auto& state : myStates)
		{
			if (state.second.Name == aName)
			{
				return state.second.ID;
			}
		}
		return 0;
	}

	const std::vector<AnimatorTransition> AnimatorLayer::GetTransitionsFromState(uint64_t aID) const
	{
		std::vector<AnimatorTransition> transitions;
		for (auto& trans : myTransitions)
		{
			if (trans.second.FromStateID == aID)
			{
				transitions.push_back(trans.second);
			}
		}
		return transitions;
	}



}