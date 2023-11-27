#include "FFpch.h"
#include "AnimatorImporter.h"
#include "Firefly/Asset/Animations/Animator.h"
#include "nlohmann/json.hpp"
namespace Firefly
{
	bool AnimatorImporter::ImportAnimator(Ref<Animator> aAnimator)
	{

		std::ifstream file(aAnimator->GetPath());
		if (!file.is_open())
		{
			LOGERROR("File could not be opened! Path: {}", aAnimator->GetPath().string());
			return false;
		}
		nlohmann::json json;
		file >> json;
		file.close();

		aAnimator->myID = json["ID"];
		auto& layersJsonArr = json["Layers"];

		for (auto& layerJson : layersJsonArr)
		{
			aAnimator->myLayers.push_back(AnimatorLayer());
			auto& layer = aAnimator->myLayers.back();

			auto& statesJsonArr = layerJson["States"];
			for (int i = 0; i < statesJsonArr.size(); i++)
			{
				auto& stateJson = statesJsonArr[i];
				AnimatorState state;
				state.Name = stateJson["Name"];
				state.AnimationPath = stateJson["AnimationPath"];
				state.ID = stateJson["ID"];
				auto& posJson = stateJson["Position"];
				state.Position = { posJson[0], posJson[1] };
				state.Speed = stateJson["Speed"];
				state.Looping = stateJson["Looping"];
				if (stateJson.contains("BlendspaceHorizontalAxisParamID"))
				{
					state.HorizontalAxisParamID = stateJson["BlendspaceHorizontalAxisParamID"];
				}
				if (stateJson.contains("BlendspaceVerticalAxisParamID"))
				{
					state.VerticalAxisParamID = stateJson["BlendspaceVerticalAxisParamID"];
				}
				layer.myStates.emplace(state.ID, state);
			}

			auto& transitionsJsonArr = layerJson["Transitions"];
			for (int i = 0; i < transitionsJsonArr.size(); i++)
			{
				auto& transitionJson = transitionsJsonArr[i];
				AnimatorTransition transition;
				transition.ID = transitionJson["ID"];
				transition.FromStateID = transitionJson["FromStateID"];
				transition.ToStateID = transitionJson["ToStateID"];
				if (transitionJson.contains("HasExitTime"))
				{
					transition.HasExitTime = transitionJson["HasExitTime"];
				}
				if (transitionJson.contains("ExitTime"))
				{
					transition.ExitTime = transitionJson["ExitTime"];
				}
				if (transitionJson.contains("TransitionDuration"))
				{
					transition.TransitionDuration = transitionJson["TransitionDuration"];
				}


				auto& parametersJsonArr = transitionJson["Parameters"];
				for (int j = 0; j < parametersJsonArr.size(); j++)
				{
					auto& parameterJson = parametersJsonArr[j];
					AnimatorParameterInstance parameter = {};
					parameter.ParameterID = parameterJson["ParameterID"];
					parameter.Condition = parameterJson["Condition"];
					parameter.ID = parameterJson["ID"];
					int val = parameterJson["Value"].get<int>();
					auto valueBytes = reinterpret_cast<byte*>(&val);
					parameter.Value[0] = *(valueBytes + 0);
					parameter.Value[1] = *(valueBytes + 1);
					parameter.Value[2] = *(valueBytes + 2);
					parameter.Value[3] = *(valueBytes + 3);

					transition.Parameters.push_back(parameter);
				}
				layer.myTransitions.emplace(transition.ID, transition);
			}

			layer.myAnyStateID = layerJson["AnyStateID"];
			layer.myEntryStateID = layerJson["EntryStateID"];
			layer.myName = layerJson["Name"];
			layer.myIsAdditive = layerJson["IsAdditive"];
			if (layerJson.contains("Weight"))
			{
				layer.myWeight = layerJson["Weight"];
			}
			if (layerJson.contains("Avatar"))
			{
				layer.myAvatarPath = layerJson["Avatar"];
			}
		}



		auto& parametersJsonArr = json["Parameters"];
		for (int i = 0; i < parametersJsonArr.size(); i++)
		{
			auto& parameterJson = parametersJsonArr[i];
			AnimatorParameter parameter;
			parameter.Name = parameterJson["Name"];
			parameter.Type = static_cast<Firefly::AnimatorParameterType>(parameterJson["Type"]);
			parameter.ID = parameterJson["ID"];
			aAnimator->myParameters.emplace(parameter.ID, parameter);
		}

		return true;
	}

	uint64_t Firefly::AnimatorImporter::ImportID(const std::filesystem::path& aPath)
	{
		std::ifstream file(aPath);
		if (!file.is_open())
		{
			LOGERROR("File could not be opened! Path: {}", aPath.string());
			return 0;
		}
		nlohmann::json json;
		file >> json;
		file.close();
		return json["ID"].get<uint64_t>();
	}

}