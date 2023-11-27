#pragma once
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector.h"
class AnimatorWindow;
namespace Firefly
{
	enum class AnimatorParameterType : uint32_t
	{
		Float,
		Int,
		Bool,
		Trigger,
		COUNT
	};
	static inline std::string AnimParameterToString(AnimatorParameterType aType)
	{
		switch (aType)
		{
		case AnimatorParameterType::Float: return "Float";
		case AnimatorParameterType::Int: return "Int";
		case AnimatorParameterType::Bool: return "Bool";
		case AnimatorParameterType::Trigger: return "Trigger";
		}
		return "";
	}
	static inline AnimatorParameterType StringToAnimParameterType(std::string aString)
	{
		if (aString == "Float") return AnimatorParameterType::Float;
		if (aString == "Int") return AnimatorParameterType::Int;
		if (aString == "Bool") return AnimatorParameterType::Bool;
		if (aString == "Trigger") return AnimatorParameterType::Trigger;
		return AnimatorParameterType::Float;
	}
	//represents the parameter in the animator
	struct AnimatorParameter
	{
		std::string Name;
		AnimatorParameterType Type;
		char Data[4] = { 0,0,0,0 };
		uint64_t ID;
	};

	//represents parameters in transitions
	struct AnimatorParameterInstance
	{
		uint64_t ID;

		uint64_t ParameterID;
		uint8_t Condition; // 0 = false or Less Than, 1 = true or Greater Than, 2 = Equal
		unsigned char Value[4]; // reinterpreted to correct type
	};

	struct AnimatorTransition
	{
		std::string Name = "Transition";

		uint64_t ID;
		uint64_t FromStateID;
		uint64_t ToStateID;
		std::vector<AnimatorParameterInstance> Parameters;
		bool HasExitTime = false;
		float ExitTime = 1.f;
		float TransitionDuration = 0;
		//float TransitionOffset = 0;
	};
	struct AnimatorState
	{
		std::string Name = "";
		std::string AnimationPath = "";
		uint64_t ID = 0;
		Utils::Vector2f Position = { 0,0 };
		float Speed = 1;
		bool Looping = true;

		uint64_t HorizontalAxisParamID = 0;
		uint64_t VerticalAxisParamID = 0;
	};

	class AnimatorLayer
	{
	public:
		AnimatorLayer();
		~AnimatorLayer() = default;

		uint64_t GetStateID(const std::string& aName)const;
		const std::vector<AnimatorTransition> GetTransitionsFromState(uint64_t aID) const;

		__forceinline const std::unordered_map<uint64_t, AnimatorState>& GetStates() const { return myStates; }
		__forceinline const std::unordered_map<uint64_t, AnimatorTransition>& GetTransitions() const { return myTransitions; }
		__forceinline const AnimatorState& GetState(uint64_t aID) const { return myStates.at(aID); }
		__forceinline AnimatorState& GetStateMutable(uint64_t aID) { return myStates.at(aID); }
		__forceinline const AnimatorTransition& GetTransition(uint64_t aID) const { return myTransitions.at(aID); }
		__forceinline uint64_t GetEntryStateID() const { return myEntryStateID; }
		__forceinline uint64_t GetAnyStateStateID() const { return myAnyStateID; }
		__forceinline const AnimatorState& GetEntryState() const { return myStates.at(myEntryStateID); }
		__forceinline const std::string& GetName() const { return myName; }
		__forceinline const bool& IsAdditive() const { return myIsAdditive; }
		__forceinline const float& GetWeight() const { return myWeight; }
		__forceinline const std::string& GetAvatarPath() const { return myAvatarPath; }

	private:
		friend class ::AnimatorWindow;
		friend class AnimatorImporter;
		std::unordered_map<uint64_t, AnimatorState> myStates;
		std::unordered_map<uint64_t, AnimatorTransition> myTransitions;

		std::string myAvatarPath;
		std::string myName;
		bool myIsAdditive = false;
		uint64_t myEntryStateID;
		uint64_t myAnyStateID;
		float myWeight = 1.f;

	};
}