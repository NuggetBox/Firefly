#pragma once

#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "Firefly/Asset/Animations/Animator.h"

#include <unordered_map>
#include <map>

#define BIND_ANIMATOR_FN(fn) (std::bind(&fn, this, std::placeholders::_1))

namespace Firefly
{
	class Animation;
	class BlendSpace;
	class Animator;
	class AnimatedMeshComponent;
	class AppUpdateEvent;
	class AnimatorComponent;
	class AvatarMask;
	class Frame;
	struct Skeleton;

	struct AnimatorState;

	struct AnimationEvent
	{
		int Frame = 0;
		std::function<void()> Callback;
	};
	class AnimatorLayerInstance
	{
	public:
		void EarlyInitialize();
		void Update(bool aIsUnscaledDelta = false);
		void CollectTransitions();
		void StartTransitionTo(uint64_t aTransitionID);

		void SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction);
		void SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction);
		void SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction);
		void SetExitFunction(const std::string& aStateName, std::function<void()> aFunction);

		void SetCurrentState(const std::string& aStateName);
		void SetCurrentState(uint64_t aID);

		void AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback);

		const AnimatorState& GetCurrentState() { return myAnimatorLayer->GetState(myCurrentStateID); }
		Ref<Animation> GetCurrentAnimation()
		{
			if (myAnimationsMap.contains(myCurrentStateID))
				return myAnimationsMap[myCurrentStateID];
			return Ref<Animation>();
		}
		Ref<BlendSpace> GetCurrentBlendSpace()
		{
			if (myBlendSpacesMap.contains(myCurrentStateID))
				return myBlendSpacesMap[myCurrentStateID];
			return Ref<BlendSpace>();
		}
		const AnimatorTransition& GetCurrentTransition() { return myCurrentTransition; }
		__forceinline float GetCurrentAnimationTime() { return myCurrentAnimationTime; }

		bool IsTransitioning() const { return myIsTransitioning; }


		float GetTransitionProgress() const { return myTransitionTime / myCurrentTransition.TransitionDuration; }



		void Load(const AnimatorLayer& aLayer);
		void Refresh();
	private:
		friend class AnimatorComponent;
		friend class AnimatorWindow;

		bool CheckTransition(AnimatorTransition& aTransition);
		void UpdateTransition(bool aIsUnscaledDelta = false);

		__forceinline uint64_t GetEntryStateID() { return myAnimatorLayer->GetEntryStateID(); }

		AnimatorComponent* myAnimatorComponent = nullptr;
		const AnimatorLayer* myAnimatorLayer;

		bool myEntryStateEnterCalled = false;

		float myCurrentAnimationTime;

		float myBlendAnimationTime;
		int myLastAnimationFrame = 0;


		bool myIsTransitioning = false;
		float myTransitionTime = 0.0f;
		AnimatorTransition myCurrentTransition;

		uint64_t myCurrentStateID;

		std::vector<AnimatorTransition> myCurrentTransitions;


		std::unordered_map<uint64_t, Ref<Animation>> myAnimationsMap;
		std::unordered_map<uint64_t, Ref<BlendSpace>> myBlendSpacesMap;
		//enter functions
		std::unordered_map<uint64_t, std::function<void()>> myEnterFunctions;
		//end of transitions enter functions
		std::unordered_map<uint64_t, std::function<void()>> myEndOfTransitionEnterFunctions;
		//update functions
		std::unordered_map<uint64_t, std::function<void()>> myUpdateFunctions;
		//exit functions
		std::unordered_map<uint64_t, std::function<void()>> myExitFunctions;


		std::unordered_map<uint64_t, std::vector<AnimationEvent>> myStateAnimationEvents;

		Ref<AvatarMask> myAvatarMask;
	};


	class AnimatorComponent : public Component
	{
	public:
		AnimatorComponent();
		~AnimatorComponent() = default;

		void Initialize() override;
		void EarlyInitialize() override;
		void OnEvent(Firefly::Event& aEvent) override;

		void InterceptFrame(std::function<void(Frame&)> aLambda);

		//ALWAYS SETS ON LAYER 0
		void SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction);
		//ALWAYS SETS ON LAYER 0
		void SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction);
		//ALWAYS SETS ON LAYER 0
		void SetExitFunction(const std::string& aStateName, std::function<void()> aFunction);

		void SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName);
		void SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName);
		void SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName);
		void SetExitFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName);
		void SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex);
		void SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex = 0);
		void SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex);
		void SetExitFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex);

		int GetLayerIndex(const std::string& aLayerName) const;


		void SetFloatParameter(const std::string& aParameterName, float aValue);
		void SetBoolParameter(const std::string& aParameterName, bool aValue);
		void SetIntParameter(const std::string& aParameterName, int aValue);
		void SetTriggerParameter(const std::string& aParameterName);
		void ResetTrigger(const std::string& aParameterName);

		//void SetCurrentState(const std::string& aStateName);
		//void SetCurrentState(uint64_t aID);
		// 

		//ALWAYS GETS FROM LAYER 0
		const AnimatorState& GetCurrentState();
		const AnimatorState& GetCurrentState(uint64_t aLayerIndex);
		const AnimatorState& GetCurrentState(const std::string& aLayerName);

		//ALWAYS GETS FROM LAYER 0
		Ref<Animation> GetCurrentAnimation();
		Ref<Animation> GetCurrentAnimation(const std::string& aLayerName);
		Ref<Animation> GetCurrentAnimation(uint64_t aLayerIndex);

		//ALWAYS GETS FROM LAYER 0
		float GetCurrentAnimationTime();
		float GetCurrentAnimationTime(const std::string& aLayerName);
		float GetCurrentAnimationTime(uint64_t aLayerIndex);



		static std::string GetFactoryName() { return "AnimatorComponent"; }
		static Ref<Component> Create() { return CreateRef<AnimatorComponent>(); }

		//#871 Fabian Fix (Severity Critical), (Priority High) <3
		float myAnimationSpeed = 1;

		Ref<Animator> GetAnimator() { return myAnimator; }

		//ALWAYS SETS ON LAYER 0
		void AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback);
		void AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback, const std::string& aLayerName);
		void AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback, uint64_t aLayerIndex);

		void SetTriggerToBeTurnedFalse(uint64_t aTriggerID);
		__forceinline float GetFloatParameterValue(uint64_t aID) { return *reinterpret_cast<float*>(&myParameterValues[aID]); }
		float GetFloatParameterValue(const std::string& aParameterName);
		__forceinline bool GetBoolParameterValue(uint64_t aID) { return *reinterpret_cast<bool*>(&myParameterValues[aID]); }
		bool GetBoolParameterValue(const std::string& aParameterName);
		__forceinline int GetIntParameterValue(uint64_t aID) { return *reinterpret_cast<int*>(&myParameterValues[aID]); }
		int GetIntParameterValue(const std::string& aParameterName);
		__forceinline bool GetTriggerParameterValue(uint64_t aID) { return *reinterpret_cast<bool*>(&myParameterValues[aID]); }
		bool GetTriggerParameterValue(const std::string& aParameterName);

		void SetUseUnscaledDeltaFlag(bool aFlag);
		bool GetUsingUnscaledDelta() const { return myUseUnscaledDeltaFlag; };

		void AddAimIK(const std::string& aBoneName, const Utils::Vector3f& aPosition, const Utils::Vector3f& aAxis);
		void RemoveAimIK(const std::string& aBoneName);

		void AddRotationToAdd(const std::string& aBoneName, const Utils::Quaternion& aRotation);
		void RemoveRotationToAdd(const std::string& aBoneName);
		struct IKChain
		{
			struct BoneData
			{
				std::string Name;
				Utils::Vector3f PoleVector = { 0,0,0 };
			};
			std::vector<BoneData> Data;
			Utils::Vector3f Target;
			Utils::Vector3f PoleVector = { 0,0,0 };
		};
		void AddIKChain(const std::vector<IKChain::BoneData>& aChain, const Utils::Vector3f& aTarget, const Utils::Vector3f& aPoleVector = Utils::Vector3f(0, 0, 0));

		void LinkTransformToBone(Utils::BasicTransform& aTransform, const std::string& aBoneName, bool aBeforeIKFlag = false);
	private:
		friend class AnimatorWindow;

		//IK Chain STUFF
		Utils::BasicTransform GetGlobalTransform(int aIndex, const std::vector< Utils::BasicTransform>& someTransforms, const Skeleton& aSkeleton);
		//

		bool OnUpdate(AppUpdateEvent& aEvent);
		std::vector<AnimatorLayerInstance> myLayerInstances;

		void LoadAnimator();
		void RefreshAnimator();

		std::string myAnimatorPath;

		std::unordered_map<std::string, uint64_t> myParameterNameToID;
		std::unordered_map<uint64_t, uint32_t> myParameterValues;
		Ptr<AnimatedMeshComponent> myAnimatedMeshComp;
		Ref<Animator> myAnimator;

		std::vector<uint64_t> myTriggerParametersUsedThisFrame;

		Utils::Vector3f myLastRootPos;
		float myLastRootMotionSampleTime = 0.f;
		uint64_t myLastRootMotionStateID = 0;
		struct AimIK
		{
			std::string BoneName;
			Utils::Vector3f AimPos;
			Utils::Vector3f ForwardAxis;
		};
		std::vector<AimIK> myAimIKs; //boneName, position
		std::vector<std::pair<std::string, Utils::Quaternion>> myRotationsToAdd;

		std::vector<IKChain> myIKChains;

		std::vector<std::pair<Utils::BasicTransform*, std::string>> myLinkedTransforms;
		std::vector<std::pair<Utils::BasicTransform*, std::string>> myLinkedTransformsBeforeIK;

		std::vector< std::function<void(Frame&)>> myInterceptFunctions;

		bool myUseUnscaledDeltaFlag;
	};
}