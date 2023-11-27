#include "FFpch.h"
#include "AnimatorComponent.h"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "Firefly/Components/Mesh/AnimatedMeshComponent.h"
#include "Firefly/Asset/ResourceCache.h"
#include "Firefly/Asset/Animation.h"
#include "Firefly/Asset/Animations/BlendSpace.h"
#include "Firefly/Event/EditorEvents.h"

#include "Utils/Timer.h"
#include "Firefly/Rendering/Renderer.h"
#include "Firefly/Asset/Animations/AvatarMask.h"
#include "Firefly/Components/Physics/RigidbodyComponent.h"
#include <Firefly/Components/Physics/CharacterControllerComponent.h>

#include "Utils/InputHandler.h"

namespace Firefly
{
	REGISTER_COMPONENT(AnimatorComponent);

	AnimatorComponent::AnimatorComponent()
		: Component("AnimatorComponent")
	{
		myAnimatedMeshComp = Ref<AnimatedMeshComponent>();
		myAnimatorPath = "";
		myUseUnscaledDeltaFlag = false;

		EditorVariable("Animator", ParameterType::File, &myAnimatorPath, ".animator");
		EditorVariable("Use Unscaled Delta", ParameterType::Bool, &myUseUnscaledDeltaFlag);
	}

	void AnimatorComponent::Initialize()
	{
		myAnimatedMeshComp = myEntity->GetComponent<AnimatedMeshComponent>();
	}

	void AnimatorComponent::EarlyInitialize()
	{
		if (myAnimatorPath != "")
		{
			LoadAnimator();
			for (auto& layer : myLayerInstances)
			{
				layer.EarlyInitialize();
			}
		}
		else
		{
			LOGERROR("Animator on entity:\"{}\" (ID: {}) does not have an animator file.", myEntity->GetName(), myEntity->GetID()); // this will also run when adding the component to an entity, dont know solution
		}
	}

	void AnimatorComponent::OnEvent(Firefly::Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(AnimatorComponent::OnUpdate));

		dispatcher.Dispatch<EntityPropertyUpdatedEvent>([&](EntityPropertyUpdatedEvent& e)
			{
				if (e.GetName() == "Animator")
				{
					LoadAnimator();
				}
				return false;
			});

		dispatcher.Dispatch<EditorAnimatorChangedEvent>([&](EditorAnimatorChangedEvent& e)
			{
				if (!myAnimator)
					return false;
				if (e.GetAnimatorID() == myAnimator->GetID())
				{
					RefreshAnimator();
				}
				return false;
			});
	}

	void AnimatorComponent::InterceptFrame(std::function<void(Frame&)> aLambda)
	{
		myInterceptFunctions.push_back(aLambda);
	}

	void AnimatorComponent::SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		SetEnterFunction(aStateName, aFunction, 0);
	}

	void AnimatorComponent::SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		SetUpdateFunction(aStateName, aFunction, 0);
	}

	void AnimatorComponent::SetExitFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		SetExitFunction(aStateName, aFunction, 0);
	}

	void AnimatorComponent::SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName)
	{
		SetEnterFunction(aStateName, aFunction, GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName)
	{
		SetEndOfTransitionEnterFunction(aStateName, aFunction, GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName)
	{
		SetUpdateFunction(aStateName, aFunction, GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::SetExitFunction(const std::string& aStateName, std::function<void()> aFunction, const std::string& aLayerName)
	{
		SetExitFunction(aStateName, aFunction, GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::SetEnterFunction: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::SetEnterFunction: Layer index out of bounds.");
			return;
		}
		myLayerInstances[aLayerIndex].SetEnterFunction(aStateName, aFunction);
	}

	void AnimatorComponent::SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("Layer index out of bounds.");
			return;
		}
		myLayerInstances[aLayerIndex].SetEndOfTransitionEnterFunction(aStateName, aFunction);
	}

	void AnimatorComponent::SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::SetUpdateFunction: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::SetEnterFunction: Layer index out of bounds.");
			return;
		}
		myLayerInstances[aLayerIndex].SetUpdateFunction(aStateName, aFunction);
	}

	void AnimatorComponent::SetExitFunction(const std::string& aStateName, std::function<void()> aFunction, uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::SetExitFunction: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::SetEnterFunction: Layer index out of bounds.");
			return;
		}
		myLayerInstances[aLayerIndex].SetExitFunction(aStateName, aFunction);

	}

	int AnimatorComponent::GetLayerIndex(const std::string& aLayerName) const
	{
		int index = myAnimator->GetLayerIndex(aLayerName);
		if (index == -1)
		{
			LOGERROR("Tried to get layer index of layer with name: \"{}\" but could not find the layer. Entity name: \"{}\"", aLayerName, myEntity->GetName());
		}
		return index;
	}

	void AnimatorComponent::SetFloatParameter(const std::string& aParameterName, float aValue)
	{
		if (myParameterNameToID.contains(aParameterName))
		{
			const auto paramID = myParameterNameToID[aParameterName];
			if (myParameterValues.contains(paramID))
			{
				myParameterValues[paramID] = *reinterpret_cast<uint32_t*>(&aValue);
			}
		}
	}

	void AnimatorComponent::SetBoolParameter(const std::string& aParameterName, bool aValue)
	{
		if (myParameterNameToID.contains(aParameterName))
		{
			const auto paramID = myParameterNameToID[aParameterName];
			if (myParameterValues.contains(paramID))
			{
				myParameterValues[paramID] = *reinterpret_cast<uint32_t*>(&aValue);
			}
		}
	}

	void AnimatorComponent::SetIntParameter(const std::string& aParameterName, int aValue)
	{
		if (myParameterNameToID.contains(aParameterName))
		{
			const auto paramID = myParameterNameToID[aParameterName];
			if (myParameterValues.contains(paramID))
			{
				myParameterValues[paramID] = *reinterpret_cast<uint32_t*>(&aValue);
			}
		}
	}

	void AnimatorComponent::SetTriggerParameter(const std::string& aParameterName)
	{
		if (myParameterNameToID.contains(aParameterName))
		{
			const auto paramID = myParameterNameToID[aParameterName];
			if (myParameterValues.contains(paramID))
			{
				myParameterValues[paramID] = true;
			}
		}
	}

	void AnimatorComponent::ResetTrigger(const std::string& aParameterName)
	{
		if (myParameterNameToID.contains(aParameterName))
		{
			const auto paramID = myParameterNameToID[aParameterName];
			if (myParameterValues.contains(paramID))
			{
				myParameterValues[paramID] = false;
			}
		}
	}

	const AnimatorState& AnimatorComponent::GetCurrentState()
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentState: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
		}
		return myLayerInstances[0].GetCurrentState();
	}

	const AnimatorState& AnimatorComponent::GetCurrentState(uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentState: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::GetCurrentState: Layer index out of bounds. Entity name: \"{}\"", myEntity->GetName());
		}
		return myLayerInstances[aLayerIndex].GetCurrentState();
	}

	const AnimatorState& AnimatorComponent::GetCurrentState(const std::string& aLayerName)
	{
		return GetCurrentState(GetLayerIndex(aLayerName));
	}

	Ref<Animation> AnimatorComponent::GetCurrentAnimation()
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimation: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return nullptr;
		}
		return myLayerInstances[0].GetCurrentAnimation();
	}

	Ref<Animation> AnimatorComponent::GetCurrentAnimation(const std::string& aLayerName)
	{
		return GetCurrentAnimation(GetLayerIndex(aLayerName));
	}

	Ref<Animation> AnimatorComponent::GetCurrentAnimation(uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimation: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return nullptr;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimation: Layer index out of bounds. Entity name: \"{}\"", myEntity->GetName());
			return nullptr;
		}
		return myLayerInstances[aLayerIndex].GetCurrentAnimation();
	}

	float AnimatorComponent::GetCurrentAnimationTime()
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimationTime: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return 0.0f;
		}
		return myLayerInstances[0].GetCurrentAnimationTime();
	}

	float AnimatorComponent::GetCurrentAnimationTime(const std::string& aLayerName)
	{

		return GetCurrentAnimationTime(GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::AddAimIK(const std::string& aBoneName, const Utils::Vector3f& aPosition, const Utils::Vector3f& aAxis)
	{
		myAimIKs.emplace_back(aBoneName, aPosition, aAxis);
	}

	void AnimatorComponent::RemoveAimIK(const std::string& aBoneName)
	{

		myAimIKs.erase(std::remove_if(myAimIKs.begin(), myAimIKs.end(), [aBoneName](const AimIK& aAimIK)
			{
				return aAimIK.BoneName == aBoneName;
			}), myAimIKs.end());
	}

	void AnimatorComponent::AddRotationToAdd(const std::string& aBoneName, const Utils::Quaternion& aRotation)
	{
		myRotationsToAdd.push_back(std::make_pair(aBoneName, aRotation));
	}

	void AnimatorComponent::RemoveRotationToAdd(const std::string& aBoneName)
	{
		myRotationsToAdd.erase(std::remove_if(myRotationsToAdd.begin(), myRotationsToAdd.end(), [aBoneName](const std::pair<std::string, Utils::Quaternion>& aPair)
			{
				return aPair.first == aBoneName;
			}), myRotationsToAdd.end());
	}

	void AnimatorComponent::AddIKChain(const std::vector<IKChain::BoneData>& aChain, const Utils::Vector3f& aTarget, const Utils::Vector3f& aPoleVector)
	{
		myIKChains.push_back({ aChain, aTarget ,aPoleVector });
	}

	void AnimatorComponent::LinkTransformToBone(Utils::BasicTransform& aTransform, const std::string& aBoneName, bool aBeforeIKFlag)
	{
		if (aBeforeIKFlag)
		{
			myLinkedTransformsBeforeIK.push_back(std::make_pair(&aTransform, aBoneName));
		}
		else
		{
			myLinkedTransforms.push_back(std::make_pair(&aTransform, aBoneName));
		}
	}

	float AnimatorComponent::GetCurrentAnimationTime(uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimationTime: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return 0.0f;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR("AnimatorComponent::GetCurrentAnimationTime: Layer index out of bounds. Entity name: \"{}\"", myEntity->GetName());
			return 0.0f;
		}
		return myLayerInstances[aLayerIndex].GetCurrentAnimationTime();
	}

	void AnimatorComponent::AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback)
	{
		AddAnimationEvent(aStateName, aFrame, aCallback, 0);
	}

	void AnimatorComponent::AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback, const std::string& aLayerName)
	{
		AddAnimationEvent(aStateName, aFrame, aCallback, GetLayerIndex(aLayerName));
	}

	void AnimatorComponent::AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback, uint64_t aLayerIndex)
	{
		if (myLayerInstances.size() == 0)
		{
			LOGERROR("AnimatorComponent::AddAnimationEvent: Animator has no layers. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		if (aLayerIndex >= myLayerInstances.size())
		{
			LOGERROR(__FUNCTION__": Layer index out of bounds. Entity name: \"{}\"", myEntity->GetName());
			return;
		}
		myLayerInstances[aLayerIndex].AddAnimationEvent(aStateName, aFrame, aCallback);
	}

	void AnimatorComponent::SetTriggerToBeTurnedFalse(uint64_t aTriggerID)
	{
		if (std::find(myTriggerParametersUsedThisFrame.begin(),
			myTriggerParametersUsedThisFrame.end(),
			aTriggerID) == myTriggerParametersUsedThisFrame.end())
		{
			myTriggerParametersUsedThisFrame.push_back(aTriggerID);
		}
	}

	float AnimatorComponent::GetFloatParameterValue(const std::string& aParameterName)
	{
		if (!myParameterNameToID.contains(aParameterName))
		{
			LOGERROR("AnimatorComponent::GetFloatParameterValue: Parameter with name {0} does not exist", aParameterName);
			return 0.0f;
		}
		return *reinterpret_cast<float*>(&myParameterValues[myParameterNameToID[aParameterName]]);
	}

	bool AnimatorComponent::GetBoolParameterValue(const std::string& aParameterName)
	{
		if (!myParameterNameToID.contains(aParameterName))
		{
			LOGERROR("AnimatorComponent::GetBoolParameterValue: Parameter with name {0} does not exist", aParameterName);
			return false;
		}
		return *reinterpret_cast<bool*>(&myParameterValues[myParameterNameToID[aParameterName]]);
	}

	int AnimatorComponent::GetIntParameterValue(const std::string& aParameterName)
	{
		if (!myParameterNameToID.contains(aParameterName))
		{
			LOGERROR("AnimatorComponent::GetIntParameterValue: Parameter with name {0} does not exist", aParameterName);
			return 0;
		}
		return *reinterpret_cast<int*>(&myParameterValues[myParameterNameToID[aParameterName]]);
	}

	bool AnimatorComponent::GetTriggerParameterValue(const std::string& aParameterName)
	{
		if (!myParameterNameToID.contains(aParameterName))
		{
			LOGERROR("AnimatorComponent::GetTriggerParameterValue: Parameter with name {0} does not exist", aParameterName);
			return false;
		}
		return *reinterpret_cast<bool*>(&myParameterValues[myParameterNameToID[aParameterName]]);
	}

	void AnimatorComponent::SetUseUnscaledDeltaFlag(bool aFlag)
	{
		myUseUnscaledDeltaFlag = aFlag;
	}

	Utils::BasicTransform AnimatorComponent::GetGlobalTransform(int aIndex, const std::vector< Utils::BasicTransform>& someTransforms, const Skeleton& aSkeleton)
	{
		Utils::BasicTransform result = someTransforms[aIndex];
		for (int parent = aSkeleton.Bones[aIndex].Parent; parent >= 0;
			parent = aSkeleton.Bones[parent].Parent)
		{
			result = Utils::BasicTransform::Combine(someTransforms[parent], result);
		}
		return result;
	}


	bool AnimatorComponent::OnUpdate(AppUpdateEvent& aEvent)
	{
		FF_PROFILESCOPE("Animator Comp");

		if (!aEvent.GetIsInPlayMode())
			return false;

		if (!myAnimator)
		{
			return false;
		}

		if (myLayerInstances.size() == 0)
		{
			if (myAnimator->IsLoaded())
			{
				RefreshAnimator();
			}
			return false;
		}

		for (auto& layer : myLayerInstances)
		{
			for (auto& anim : layer.myAnimationsMap)
			{
				if (!anim.second)
				{
					return false;
				}
				if (!anim.second->IsLoaded())
				{
					return false;
				}
			}

			if (layer.myAvatarMask)
			{
				if (!layer.myAvatarMask->IsLoaded())
				{
					return false;
				}
			}
		}

		if (myAnimatedMeshComp.lock() && myAnimator)
		{
			for (auto& layer : myLayerInstances)
			{
				layer.Update(myUseUnscaledDeltaFlag);
			}

			if (myAnimatedMeshComp.lock())
			{
				Utils::Vector3f rootMotion;
				std::vector<Utils::Matrix4f> matrices;
				Frame finalFrame;
				if (!myAnimatedMeshComp.lock()->GetMesh()->IsLoaded())
				{
					return false;
				}
				auto& skeleton = myAnimatedMeshComp.lock()->GetMesh()->GetSkeleton();
				bool first = true;
				for (auto& layer : myLayerInstances)
				{
					auto& state = layer.GetCurrentState();
					Frame frame;
					auto blendspace = layer.GetCurrentBlendSpace();
					if (layer.GetCurrentAnimation() && layer.GetCurrentAnimation()->IsLoaded())
					{
						frame = layer.GetCurrentAnimation()->GetFrame(layer.myCurrentAnimationTime, layer.GetCurrentState().Looping);
					}
					else if (blendspace && blendspace->IsLoaded())
					{
						std::optional<Frame> frameOpt;
						if (blendspace->GetDimensionType() == BlendspaceType::OneDimensional)
						{
							frameOpt = blendspace->Sample1D(layer.myCurrentAnimationTime, GetFloatParameterValue(state.HorizontalAxisParamID));
						}
						else
						{
							frameOpt = blendspace->Sample2D(layer.myCurrentAnimationTime,
								Utils::Vector2f(GetFloatParameterValue(state.HorizontalAxisParamID), GetFloatParameterValue(state.VerticalAxisParamID)));
						}
						if (frameOpt.has_value())
						{
							frame = frameOpt.value();
						}
						else
						{
							//LOGERROR("AnimatorComponent::OnUpdate: Blendspace returned no frame. Entity name: \"{}\"", myEntity->GetName()); // Spaming the log
						}

					}
					else
					{
						if (!first)
						{
							continue;
						}
						frame.LocalTransforms.resize(skeleton.Bones.size());
					}

					if (layer.IsTransitioning())
					{
						//blend with next animation
						const auto toStateID = layer.GetCurrentTransition().ToStateID;
						if (layer.myAnimationsMap.contains(toStateID))
						{
							const auto& blendAnimation = layer.myAnimationsMap[toStateID];
							const auto& toState = layer.myAnimatorLayer->GetState(toStateID);
							Frame blendFrame = blendAnimation->GetFrame(layer.myBlendAnimationTime, toState.Looping);
							const auto blendAmount = layer.GetTransitionProgress();
							frame.BlendWith(blendFrame, blendAmount);
						}
						else if (layer.myBlendSpacesMap.contains(toStateID))
						{
							const auto& blendSpace = layer.myBlendSpacesMap[toStateID];
							if (blendSpace)
							{
								const auto& toState = layer.myAnimatorLayer->GetState(toStateID);
								std::optional<Frame> blendFrameOpt;
								if (blendSpace->GetDimensionType() == BlendspaceType::OneDimensional)
								{
									blendFrameOpt = blendSpace->Sample1D(layer.myBlendAnimationTime, GetFloatParameterValue(toState.HorizontalAxisParamID));
								}
								else
								{
									blendFrameOpt = blendSpace->Sample2D(layer.myBlendAnimationTime,
										Utils::Vector2f(GetFloatParameterValue(toState.HorizontalAxisParamID), GetFloatParameterValue(toState.VerticalAxisParamID)));
								}
								if (blendFrameOpt.has_value())
								{
									const auto blendAmount = layer.GetTransitionProgress();
									frame.BlendWith(blendFrameOpt.value(), blendAmount);
								}
								else
								{
									LOGERROR("AnimatorComponent::OnUpdate: Blendspace returned no frame. Entity name: \"{}\"", myEntity->GetName());
								}
							}
							else // if nothing is assigned to the state, blend to identity
							{

								Frame blendFrame = Frame();
								blendFrame.LocalTransforms.resize(skeleton.Bones.size());
								for (int locTransBlend = 0; locTransBlend < blendFrame.LocalTransforms.size(); locTransBlend++)
								{
									blendFrame.LocalTransforms[locTransBlend] = Utils::BasicTransform();
								}
								const auto blendAmount = layer.GetTransitionProgress();
								frame.BlendWith(blendFrame, blendAmount);
							}
						}
					}

					//always take the whole of the base layer
					if (first)
					{
						first = false;
						finalFrame = frame;
						if (myLastRootMotionStateID != layer.GetCurrentState().ID)
						{
							myLastRootMotionSampleTime = 0;
							myLastRootPos = { 0,0,0 };
							myLastRootMotionStateID = layer.GetCurrentState().ID;
							//LOGINFO("New State ID");
						}
						if (layer.GetCurrentAnimation() && layer.GetCurrentAnimation()->IsLoaded())
						{
							bool apa = true;
							if (myLastRootMotionSampleTime > layer.GetCurrentAnimationTime())
							{
								rootMotion = layer.GetCurrentAnimation()->Frames[layer.GetCurrentAnimation()->Frames.size() - 2].LocalTransforms[0].GetPosition() - myLastRootPos;
								myLastRootPos = { 0,0,0 };
								//LOGINFO("end of animation loop root motion");
							}
							else
							{
								apa = false;
								auto frameNoLoop = layer.GetCurrentAnimation()->GetFrame(layer.myCurrentAnimationTime, false);
								rootMotion = frameNoLoop.LocalTransforms[0].GetPosition() - myLastRootPos;
								myLastRootPos = frameNoLoop.LocalTransforms[0].GetPosition();

							}
							finalFrame.LocalTransforms[0].SetPosition(0, 0, 0);
							/*LOGINFO("Root Motion: {}, {}, {}", rootMotion.x, rootMotion.y, rootMotion.z);
							LOGINFO("APA: {}", apa);*/

							myLastRootMotionSampleTime = layer.GetCurrentAnimationTime();
						}

					}
					else
					{
						if (layer.myAnimatorLayer->IsAdditive())
						{
							Frame baseFrame;
							bool hasValue = false;
							if (layer.GetCurrentAnimation())
							{
								baseFrame = layer.GetCurrentAnimation()->GetFrame(0, layer.GetCurrentState().Looping);
								hasValue = true;

							}
							else if (auto blendspace = layer.GetCurrentBlendSpace())
							{
								//no support here yet
								//LOGERROR("AnimatorComponent::OnUpdate: Additive Blendspace blending not supported yet. Entity name: \"{}\"", myEntity->GetName()); // Spaming the log
							}
							else
							{
								finalFrame.Add(frame, 1.0f);
							}
							if (hasValue)
							{
								for (int i = 0; i < frame.LocalTransforms.size(); i++)
								{
									frame.LocalTransforms[i].SetPosition(frame.LocalTransforms[i].GetPosition() -
										baseFrame.LocalTransforms[i].GetPosition());

									frame.LocalTransforms[i].SetRotation(baseFrame.LocalTransforms[i].GetQuaternion().GetInverse() *
										frame.LocalTransforms[i].GetQuaternion().GetNormalized());

									frame.LocalTransforms[i].SetScale(frame.LocalTransforms[i].GetScale() -
										baseFrame.LocalTransforms[i].GetScale());
								}
								finalFrame.Add(frame, layer.myAnimatorLayer->GetWeight());
							}
						}
						else
						{
							if (layer.myAvatarMask)
							{
								finalFrame.BlendWith(frame, layer.myAnimatorLayer->GetWeight(), layer.myAvatarMask);
							}
							else
							{
								finalFrame.BlendWith(frame, layer.myAnimatorLayer->GetWeight());
							}
						}
					}
				}

				Utils::BasicTransform entityTransform = myEntity->GetTransform().ToBasic();
				entityTransform.SetPosition(entityTransform.GetPosition() + myAnimatedMeshComp.lock()->GetOffset());

				for (auto& linkedTransform : myLinkedTransformsBeforeIK)
				{
					if (skeleton.BoneNameToIndex.contains(linkedTransform.second))
					{
						auto boneIndex = skeleton.BoneNameToIndex[linkedTransform.second];
						auto& transform = *linkedTransform.first;

						auto globalTrans = GetGlobalTransform(boneIndex, finalFrame.LocalTransforms, skeleton);
						transform = Utils::BasicTransform::Combine(entityTransform, globalTrans);
					}
				}

				for (auto& rotationToAdd : myRotationsToAdd)
				{
					if (skeleton.BoneNameToIndex.contains(rotationToAdd.first))
					{
						auto boneIndex = skeleton.BoneNameToIndex[rotationToAdd.first];

						finalFrame.LocalTransforms[boneIndex].SetRotation(finalFrame.LocalTransforms[boneIndex].GetQuaternion() * rotationToAdd.second);
					}
				}
				myRotationsToAdd.clear();


				//CHAIN IK
				for (auto& chainIK : myIKChains)
				{
					bool valid = true;

					std::vector<int> boneIndices;

					for (auto boneData : chainIK.Data)
					{
						if (!skeleton.BoneNameToIndex.contains(boneData.Name))
						{
							LOGERROR(__FUNCTION__"(): IK chain bone name not found in skeleton: {}", boneData.Name);
							valid = false;
							break;
						}
						else if (!boneIndices.empty() && skeleton.Bones[skeleton.BoneNameToIndex[boneData.Name]].Parent != boneIndices.back())
						{
							LOGERROR(__FUNCTION__"(): The bone {} is not a child to the previus bone called {}", boneData.Name, skeleton.Bones[boneIndices.back()].Name);
							valid = false;
							break;
						}
						else
						{
							boneIndices.push_back(skeleton.BoneNameToIndex[boneData.Name]);
						}
					}
					if (!valid)
					{
						continue;
					}


					std::vector<Utils::BasicTransform> chainTransforms;

					chainTransforms.push_back(Utils::BasicTransform::Combine(entityTransform, GetGlobalTransform(boneIndices.front(), finalFrame.LocalTransforms, skeleton)));
					for (int i = 1; i < boneIndices.size(); i++)
					{
						chainTransforms.push_back(finalFrame.LocalTransforms[boneIndices[i]]);
					}

					std::vector<Utils::Vector3f> worldPositions;
					worldPositions.resize(boneIndices.size());
					std::vector<float> lengths;
					lengths.resize(boneIndices.size());
					for (auto& len : lengths)
					{
						len = 0;
					}
					auto getChainGlobalTransform = [&](int aIndex) -> Utils::BasicTransform
					{
						auto world = chainTransforms[aIndex];
						for (int k = aIndex - 1; k >= 0; k--)
						{
							world = Utils::BasicTransform::Combine(chainTransforms[k], world);
						}
						return world;
					};
					auto globalChainTransformToLocal = [&](int aIndex, const Utils::BasicTransform& aGlobalTransform) -> Utils::BasicTransform
					{
						auto local = aGlobalTransform;
						for (int k = 0; k < aIndex; k++)
						{
							local = Utils::BasicTransform::Combine(Utils::BasicTransform::Inverse(chainTransforms[k]), local);
						}
						return local;
					};

					auto ikChainToWorld = [&]()
					{
						for (int i = 0; i < chainTransforms.size(); i++)
						{

							//Get Global Transform
							auto world = getChainGlobalTransform(i);
							//collect the position from the world matrix
							worldPositions[i] = world.GetPosition();

							if (i >= 1)
							{
								Utils::Vector3f prev = worldPositions[i - 1];
								lengths[i] = (prev - worldPositions[i]).Length();
							}
						}
					};

					auto worldToIKChain = [&]()
					{
						for (int i = 0; i < chainTransforms.size() - 1; i++)
						{
							auto world = getChainGlobalTransform(i);
							auto next = getChainGlobalTransform(i + 1);

							auto toNext = next.GetPosition() - world.GetPosition();
							toNext = world.GetQuaternion().GetInverse() * toNext;

							auto toDesired = worldPositions[i + 1] - world.GetPosition();
							toDesired = world.GetQuaternion().GetInverse() * toDesired;

							Utils::Quaternion delta = Utils::Quaternion::FromTo(toNext, toDesired);

							chainTransforms[i].SetRotation(chainTransforms[i].GetQuaternion() * delta); // might need to be flipped
						}
					};


					ikChainToWorld();

					auto targetPos = chainIK.Target;
					auto basePos = chainTransforms[0].GetPosition();

					auto iterateBackwards = [&]()
					{
						worldPositions.back() = targetPos;
						for (int j = worldPositions.size() - 2; j >= 0; --j)
						{
							auto diff = worldPositions[j] - worldPositions[j + 1];
							auto dir = diff.GetNormalized();

							worldPositions[j] = worldPositions[j + 1] + dir * lengths[j];
						}
					};

					auto iterateForwards = [&]()
					{
						worldPositions[0] = basePos;
						for (int j = 1; j < worldPositions.size(); ++j)
						{
							auto diff = worldPositions[j] - worldPositions[j - 1];
							auto dir = diff.GetNormalized();

							worldPositions[j] = worldPositions[j - 1] + dir * lengths[j];
						}
					};

					auto applyHingeConstraint = [&](int i, Utils::Vector3f aAxis)
					{

					};

					for (uint32_t iteration = 0; iteration < 10; iteration++)
					{
						auto effectorPos = worldPositions.back();
						if ((effectorPos - targetPos).LengthSqr() < 1.f)
						{
							break;
						}

						iterateBackwards();
						iterateForwards();
					}

					if (chainIK.Data.size() == 3 && chainIK.PoleVector.LengthSqr() != 0)
					{
						auto limbAxis = (worldPositions[2] - worldPositions[0]).GetNormalized();
						auto poleDirection = chainIK.PoleVector;
						auto boneDirection = (worldPositions[1] - worldPositions[0]).GetNormalized();


						Utils::Vector3f::Orthonormalize(limbAxis, poleDirection);
						Utils::Vector3f::Orthonormalize(limbAxis, boneDirection);

						auto angle = Utils::Quaternion::FromTo(boneDirection, poleDirection);

						worldPositions[1] = angle * (worldPositions[1] - worldPositions[0]) + worldPositions[0];
					}

					worldToIKChain();


					auto rootWorld = Utils::BasicTransform::Combine(entityTransform,
						GetGlobalTransform(skeleton.Bones[boneIndices[0]].Parent, finalFrame.LocalTransforms, skeleton));

					chainTransforms[0] = Utils::BasicTransform::Combine(Utils::BasicTransform::Inverse(rootWorld), chainTransforms[0]);
					for (int i = 0; i < boneIndices.size(); i++)
					{
						finalFrame.LocalTransforms[boneIndices[i]] = chainTransforms[i];
					}
				}
				myIKChains.clear();
				//

				for (auto& aimIK : myAimIKs)
				{
					if (skeleton.BoneNameToIndex.contains(aimIK.BoneName))
					{
						auto boneIndex = skeleton.BoneNameToIndex[aimIK.BoneName];
						auto worldTransform = finalFrame.LocalTransforms[boneIndex];
						std::vector<int> parents;
						//to World
						for (int parent = skeleton.Bones[boneIndex].Parent; parent >= 0;
							parent = skeleton.Bones[parent].Parent)
						{
							worldTransform = Utils::BasicTransform::Combine(finalFrame.LocalTransforms[parent], worldTransform);
							parents.push_back(parent);
						}
						worldTransform = Utils::BasicTransform::Combine(entityTransform, worldTransform);


						auto targetPos = aimIK.AimPos;
						auto currentDir = worldTransform.GetQuaternion() * aimIK.ForwardAxis;
						auto desiredDir = (targetPos - worldTransform.GetPosition()).GetNormalized();
						auto toQuaternion = Utils::Quaternion::FromTo(currentDir, desiredDir);
						worldTransform.SetRotation(toQuaternion * worldTransform.GetQuaternion());

						//to local 
						worldTransform = Utils::BasicTransform::Combine((Utils::BasicTransform::Inverse(entityTransform)), worldTransform);
						for (int i = parents.size() - 1; i >= 0; i--)
						{
							worldTransform = Utils::BasicTransform::Combine(Utils::BasicTransform::Inverse(finalFrame.LocalTransforms[parents[i]]), worldTransform);

						}

						finalFrame.LocalTransforms[boneIndex].SetRotation(worldTransform.GetQuaternion());


					}
				}
				myAimIKs.clear();

				for (size_t i = 0; i < myInterceptFunctions.size(); i++)
				{
					myInterceptFunctions[i](finalFrame);
				}

				finalFrame.CalculateTransforms(skeleton, matrices);
				if (rootMotion.LengthSqr() > 0)
				{
					if (myEntity->HasComponent<Firefly::RigidbodyComponent>())
					{
						myEntity->GetComponent<Firefly::RigidbodyComponent>().lock()->Teleport(myEntity->GetTransform().GetPosition() + rootMotion * myEntity->GetTransform().GetForward());
					}
					else if (myEntity->HasComponent<Firefly::CharacterControllerComponent>())
					{
						//LOGINFO("Root motion: {}, {}, {}", rootMotion.x, rootMotion.y, rootMotion.z);
						auto movemnt = rootMotion * Utils::Quaternion::FromTo({ 0,0,1 }, myEntity->GetTransform().GetForward());
						//LOGINFO("movemnt : {}, {}, {}", movemnt.x, movemnt.y, movemnt.z);
						myEntity->GetComponent<Firefly::CharacterControllerComponent>().lock()->Move(rootMotion * Utils::Quaternion::FromTo({ 0,0,1 }, myEntity->GetTransform().GetForward()));
					}
					else
					{
						myEntity->GetTransform().AddLocalPosition(rootMotion);
					}
				}
				int apa = myLinkedTransforms.size();
				for (auto& linkedTransform : myLinkedTransforms)
				{
					if (skeleton.BoneNameToIndex.contains(linkedTransform.second))
					{
						auto boneIndex = skeleton.BoneNameToIndex[linkedTransform.second];
						auto& transform = *linkedTransform.first;

						auto globalTrans = GetGlobalTransform(boneIndex, finalFrame.LocalTransforms, skeleton);
						transform = Utils::BasicTransform::Combine(entityTransform, globalTrans);
					}
				}

				myAnimatedMeshComp.lock()->SetCurrentMatrices(matrices);
			}
		}

		//clear trigger values
		for (auto& triggerID : myTriggerParametersUsedThisFrame)
		{
			myParameterValues[triggerID] = false;
		}
		myTriggerParametersUsedThisFrame.clear();
		//
		return false;
	}

	void AnimatorComponent::LoadAnimator()
	{

		myAnimator = ResourceCache::GetAsset<Animator>(myAnimatorPath, true);
		if (myAnimator)
		{
			RefreshAnimator();
		}

	}
	void AnimatorComponent::RefreshAnimator()
	{
		myParameterNameToID.clear();
		myParameterValues.clear();
		myLayerInstances.clear();

		auto& params = myAnimator->GetParameters();
		for (auto& paramPair : params)
		{
			myParameterNameToID[paramPair.second.Name] = paramPair.first;
			myParameterValues[paramPair.first] = 0;
		}

		if (myAnimator)
		{
			int i = 0;
			for (auto& layer : myAnimator->GetLayers())
			{
				myLayerInstances.push_back(AnimatorLayerInstance());
				myLayerInstances.back().myAnimatorComponent = this;
				//TODO: make this call refresh instead of load
				myLayerInstances.back().Load(layer);

			}
		}
	}
	//////////////////////////////////////////////		ANIMATOR LAYER INSTANCE BELOW		//////////////////////////////////////////////



	void AnimatorLayerInstance::EarlyInitialize()
	{
		SetCurrentState(myAnimatorLayer->GetEntryStateID());
		myCurrentAnimationTime = 0;
		myStateAnimationEvents.clear();
	}
	void AnimatorLayerInstance::Update(bool aIsUnscaledDelta)
	{
		float deltaTime = 0.f;
		if (aIsUnscaledDelta)
		{
			deltaTime = Utils::Timer::GetUnscaledDeltaTime();
		}
		else
		{
			deltaTime = Utils::Timer::GetDeltaTime();
		}

		if (!myEntryStateEnterCalled)
		{
			myEntryStateEnterCalled = true;
			if (myEnterFunctions.contains(GetEntryStateID()))
			{
				myEnterFunctions[GetEntryStateID()]();
			}
		}
		//call update fuction on current state, or if transitioning call update on the next State
		uint64_t updateStateID = myCurrentStateID;
		if (myIsTransitioning)
		{
			updateStateID = myCurrentTransition.ToStateID;
		}
		if (myUpdateFunctions.contains(updateStateID))
		{
			myUpdateFunctions[updateStateID]();
		}
		//
		if (myAnimationsMap.contains(myCurrentStateID))
		{
			if (myCurrentAnimationTime < myAnimationsMap[myCurrentStateID]->GetDuration())
			{
				myCurrentAnimationTime += deltaTime * myAnimatorLayer->GetState(myCurrentStateID).Speed * myAnimatorComponent->myAnimationSpeed;
			}

		}
		else if (auto blendspace = myBlendSpacesMap[myCurrentStateID])
		{
			auto state = this->GetCurrentState();
			float duration = 0;
			if (blendspace->GetDimensionType() == BlendspaceType::OneDimensional)
			{
				duration = blendspace->GetDuration1D(myAnimatorComponent->GetFloatParameterValue(state.HorizontalAxisParamID));
			}
			else
			{
				duration = blendspace->GetDuration2D({ myAnimatorComponent->GetFloatParameterValue(state.HorizontalAxisParamID),
					myAnimatorComponent->GetFloatParameterValue(state.VerticalAxisParamID) });
			}
			if (myCurrentAnimationTime < duration)
			{
				myCurrentAnimationTime += deltaTime * myAnimatorLayer->GetState(myCurrentStateID).Speed * myAnimatorComponent->myAnimationSpeed;
			}
		}
		else
		{
			myCurrentAnimationTime = 0;
		}


		if (myAnimationsMap.contains(myCurrentStateID))
		{
			if (myCurrentAnimationTime > myAnimationsMap[myCurrentStateID]->GetDuration())
			{

				if (myAnimatorLayer->GetState(myCurrentStateID).Looping)
				{
					while (myCurrentAnimationTime > myAnimationsMap[myCurrentStateID]->GetDuration())
					{
						myCurrentAnimationTime -= myAnimationsMap[myCurrentStateID]->GetDuration();
					}
				}
				else
				{
					myCurrentAnimationTime = myAnimationsMap[myCurrentStateID]->GetDuration();
				}
			}
			//check and execute animation callbacks
			if (myStateAnimationEvents.contains(myCurrentStateID))
			{
				auto& animationEvents = myStateAnimationEvents[myCurrentStateID];
				int animationFrame = static_cast<int>((myCurrentAnimationTime / myAnimationsMap[myCurrentStateID]->GetDuration()) * myAnimationsMap[myCurrentStateID]->FrameCount);
				if (animationFrame != myLastAnimationFrame)
				{
					for (auto& ev : animationEvents)
					{
						int lastAnimFrame = myLastAnimationFrame;
						//if just looped
						if (animationFrame < myLastAnimationFrame)
						{
							lastAnimFrame -= myAnimationsMap[myCurrentStateID]->FrameCount;
						}
						if (ev.Frame <= animationFrame && ev.Frame > lastAnimFrame)
						{
							ev.Callback();
						}
					}
					myLastAnimationFrame = animationFrame;
				}
			}
			//
		}
		else if (myBlendSpacesMap.contains(myCurrentStateID))
		{
			auto blendspace = myBlendSpacesMap[myCurrentStateID];
			if (blendspace && blendspace->IsLoaded())
			{
				auto state = this->GetCurrentState();
				float duration = 0;
				if (blendspace->GetDimensionType() == BlendspaceType::OneDimensional)
				{
					duration = blendspace->GetDuration1D(myAnimatorComponent->GetFloatParameterValue(state.HorizontalAxisParamID));
				}
				else
				{
					duration = blendspace->GetDuration2D({ myAnimatorComponent->GetFloatParameterValue(state.HorizontalAxisParamID),
						myAnimatorComponent->GetFloatParameterValue(state.VerticalAxisParamID) });
				}
				if (myCurrentAnimationTime > duration)
				{
					while (myCurrentAnimationTime > duration)
					{
						myCurrentAnimationTime -= duration;
					}
				}
				//check and execute animation callbacks
				if (myStateAnimationEvents.contains(myCurrentStateID))
				{
					auto& animationEvents = myStateAnimationEvents[myCurrentStateID];
					if (!myBlendSpacesMap[myCurrentStateID]->Empty())
					{
						int animationFrame = static_cast<int>((myCurrentAnimationTime / duration) * myBlendSpacesMap[myCurrentStateID]->GetAnyAnimation()->FrameCount);

						if (animationFrame != myLastAnimationFrame)
						{
							for (auto& ev : animationEvents)
							{
								int lastAnimFrame = myLastAnimationFrame;
								//if just looped
								if (animationFrame < myLastAnimationFrame)
								{
									lastAnimFrame -= myBlendSpacesMap[myCurrentStateID]->GetAnyAnimation()->FrameCount;
								}
								if (ev.Frame <= animationFrame && ev.Frame > lastAnimFrame)
								{
									ev.Callback();
								}
							}
							myLastAnimationFrame = animationFrame;
						}
					}
				}
			}
		}
		else
		{
			myCurrentAnimationTime = 0;
		}

		if (!myIsTransitioning)
		{

			for (auto& transition : myCurrentTransitions)
			{
				if (CheckTransition(transition))
				{
					StartTransitionTo(transition.ID);
					break;
				}

			}
		}

		if (myIsTransitioning)
		{
			UpdateTransition(aIsUnscaledDelta);
		}
	}
	void AnimatorLayerInstance::CollectTransitions()
	{
		myCurrentTransitions.clear();
		myCurrentTransitions = myAnimatorLayer->GetTransitionsFromState(myCurrentStateID);
		auto anyStateTrans = myAnimatorLayer->GetTransitionsFromState(myAnimatorLayer->GetAnyStateStateID());
		myCurrentTransitions.insert(myCurrentTransitions.begin(), anyStateTrans.begin(), anyStateTrans.end());
	}
	void AnimatorLayerInstance::StartTransitionTo(uint64_t aTransitionID)
	{
		//
		auto fromState = myAnimatorLayer->GetState(myCurrentStateID);
		auto toState = myAnimatorLayer->GetState(myAnimatorLayer->GetTransition(aTransitionID).ToStateID);
		//LOGINFO("Transition from {} to {}", fromState.Name, toState.Name);
		if (myExitFunctions.contains(myCurrentStateID))
		{
			myExitFunctions[myCurrentStateID]();
		}
		myCurrentTransition = myAnimatorLayer->GetTransition(aTransitionID);
		if (myEnterFunctions.contains(myCurrentTransition.ToStateID))
		{
			myEnterFunctions[myCurrentTransition.ToStateID]();
		}
		myIsTransitioning = true;
		myBlendAnimationTime = 0;
		myTransitionTime = 0;

		myCurrentTransitions.clear(); // dont allow transitions while transitioning
	}
	void AnimatorLayerInstance::SetEnterFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		auto stateId = myAnimatorLayer->GetStateID(aStateName);
		if (stateId != 0)
		{
			myEnterFunctions[stateId] = aFunction;
		}
	}
	void AnimatorLayerInstance::SetEndOfTransitionEnterFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		auto stateId = myAnimatorLayer->GetStateID(aStateName);
		if (stateId != 0)
		{
			myEndOfTransitionEnterFunctions[stateId] = aFunction;
		}
	}
	void AnimatorLayerInstance::SetUpdateFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		auto stateId = myAnimatorLayer->GetStateID(aStateName);
		if (stateId != 0)
		{
			myUpdateFunctions[stateId] = aFunction;
		}
	}
	void AnimatorLayerInstance::SetExitFunction(const std::string& aStateName, std::function<void()> aFunction)
	{
		auto stateId = myAnimatorLayer->GetStateID(aStateName);
		if (stateId != 0)
		{
			myExitFunctions[stateId] = aFunction;
		}
	}
	void AnimatorLayerInstance::SetCurrentState(const std::string& aStateName)
	{
		uint64_t id = 0;

		for (auto& state : myAnimatorLayer->GetStates())
		{
			if (state.second.Name == aStateName)
			{
				id = state.second.ID;
				break;
			}
		}
		if (id == 0)
		{
			//tried to set state that doesnt exist
			LOGERROR("Tried to set current state to \"{}\" on entity:\"{}\" (ID: {}) that doesnt exist.", aStateName, myAnimatorComponent->GetEntity()->GetName(), myAnimatorComponent->GetEntity()->GetID());
			return;
		}

		SetCurrentState(id);
	}
	void AnimatorLayerInstance::SetCurrentState(uint64_t aID)
	{
		myCurrentStateID = aID;
		myCurrentAnimationTime = 0;
		myLastAnimationFrame = -1;
		CollectTransitions();
	}
	void AnimatorLayerInstance::AddAnimationEvent(std::string aStateName, int aFrame, std::function<void()> aCallback)
	{
		auto stateId = myAnimatorLayer->GetStateID(aStateName);
		if (stateId == 0)
		{
			LOGERROR("Tried to add animation event to state \"{}\" that doesnt exist. On entity:\"{}\" (ID: {}) ", aStateName, myAnimatorComponent->GetEntity()->GetName(), myAnimatorComponent->GetEntity()->GetID());
			return;
		}
		if (!myAnimationsMap.contains(stateId) && !myBlendSpacesMap.contains(stateId))
		{
			LOGERROR("Tried to add animation event to state \"{}\" that doesnt have an animation or blendspace. On entity:\"{}\" (ID: {}) ", aStateName, myAnimatorComponent->GetEntity()->GetName(), myAnimatorComponent->GetEntity()->GetID());
			return;
		}
		if (myAnimationsMap.contains(stateId))
		{

			if (aFrame < 0 || aFrame >= myAnimationsMap[stateId]->FrameCount)
			{
				LOGERROR("Tried to add animation event to state \"{}\" on entity:\"{}\" (ID: {}) on a frame outside the range.", aStateName, myAnimatorComponent->GetEntity()->GetName(), myAnimatorComponent->GetEntity()->GetID());
				return;
			}
		}
		else if (myBlendSpacesMap.contains(stateId))
		{

			if (!myBlendSpacesMap[stateId]->Empty())
			{
				if (aFrame < 0 || aFrame >= myBlendSpacesMap[stateId]->GetAnyAnimation()->FrameCount)
				{
					LOGERROR("Tried to add animation event to state \"{}\" on entity:\"{}\" (ID: {}) on a frame outside the range.", aStateName, myAnimatorComponent->GetEntity()->GetName(), myAnimatorComponent->GetEntity()->GetID());
					return;
				}
			}
			else
			{
				//no entries in blendspace
				LOGERROR(__FUNCTION__"No Entries in blendspace");
			}
		}
		myStateAnimationEvents[stateId].emplace_back(aFrame, aCallback);

	}
	void AnimatorLayerInstance::Load(const AnimatorLayer& aLayer)
	{
		myAnimationsMap.clear();
		myAnimatorLayer = &aLayer;
		Refresh();

		SetCurrentState(aLayer.GetEntryStateID());
	}
	void AnimatorLayerInstance::Refresh()
	{

		if (!myAnimatorLayer->GetAvatarPath().empty())
		{
			myAvatarMask = ResourceCache::GetAsset<AvatarMask>(myAnimatorLayer->GetAvatarPath(), true);
		}

		auto& states = myAnimatorLayer->GetStates();
		for (auto& statePair : states)
		{
			auto& state = statePair.second;
			if (state.ID == myAnimatorLayer->GetAnyStateStateID())
			{
				continue;
			}
			if (state.AnimationPath != "")
			{
				const std::string extension = state.AnimationPath.substr(state.AnimationPath.find_last_of('.'));
				if (std::filesystem::exists(state.AnimationPath))
				{
					if (extension == ".anim")
					{
						myAnimationsMap.insert({ state.ID ,ResourceCache::GetAsset<Animation>(state.AnimationPath, true) });
					}
					else if (extension == ".blend")
					{
						myBlendSpacesMap.insert({ state.ID ,ResourceCache::GetAsset<BlendSpace>(state.AnimationPath, true) });
					}
				}
			}
			else
			{
				LOGERROR("Animator with path\"{}\" tried to load a state \"{}\" in layer \"{}\" without an animation assigned",
					myAnimatorComponent->GetAnimator()->GetPath().string(), state.Name, myAnimatorLayer->GetName());
			}
		}

		//if the current state no longer exists we need to set it to the entry state
		if (!myAnimatorLayer->GetStates().contains(myCurrentStateID))
		{
			SetCurrentState(myAnimatorLayer->GetEntryStateID());
		}
		CollectTransitions();
	}
	bool AnimatorLayerInstance::CheckTransition(AnimatorTransition& aTransition)
	{
		bool shouldTransition = true;
		if (aTransition.HasExitTime)
		{
			if (myAnimationsMap.contains(myCurrentStateID))
			{
				shouldTransition = shouldTransition && ((myCurrentAnimationTime / myAnimationsMap[myCurrentStateID]->GetDuration()) >= aTransition.ExitTime);
			}
			else
			{
				//IF IT IS A BLENDSPACE, do we need exit time on blendspace?
			}
			if (!shouldTransition) // Early return so we don't consume triggers.
			{
				return shouldTransition;
			}
		}
		std::vector<uint64_t> triggersUsed;
		for (auto& paramInst : aTransition.Parameters)
		{
			bool paramShouldTransition = true;
			auto type = myAnimatorComponent->GetAnimator()->GetParameters().at(paramInst.ParameterID).Type;
			if (type == AnimatorParameterType::Int)
			{
				int value = myAnimatorComponent->GetIntParameterValue(paramInst.ParameterID);
				int compareVal = *reinterpret_cast<int*>(&paramInst.Value);

				if (paramInst.Condition == 0) //less than
				{
					if (value >= compareVal)
					{
						paramShouldTransition = false;
					}
				}
				else if (paramInst.Condition == 1) //greater than
				{
					if (value <= compareVal)
					{
						paramShouldTransition = false;
					}
				}
				else if (paramInst.Condition == 2) //equal
				{
					if (value != compareVal)
					{
						paramShouldTransition = false;
					}
				}

			}
			else if (type == AnimatorParameterType::Float)
			{
				float value = myAnimatorComponent->GetFloatParameterValue(paramInst.ParameterID);
				float compareVal = *reinterpret_cast<float*>(&paramInst.Value);

				if (paramInst.Condition == 0) //less than
				{
					if (value >= compareVal)
					{
						paramShouldTransition = false;
					}
				}
				else if (paramInst.Condition == 1) //greater than
				{
					if (value <= compareVal)
					{
						paramShouldTransition = false;
					}
				}
			}
			else if (type == AnimatorParameterType::Bool)
			{
				bool value = myAnimatorComponent->GetBoolParameterValue(paramInst.ParameterID);

				bool condition = paramInst.Condition;
				paramShouldTransition = (value == condition);
			}
			else if (type == AnimatorParameterType::Trigger)
			{
				bool value = myAnimatorComponent->GetTriggerParameterValue(paramInst.ParameterID);
				paramShouldTransition = value;
				triggersUsed.push_back(paramInst.ParameterID);
				//myAnimatorComponent->SetTriggerToBeTurnedFalse(paramInst.ParameterID);
				/*if (value)
				{
					LOGINFO("Shoudl TRigger");
				}*/
			}
			shouldTransition = shouldTransition && paramShouldTransition;

		}
		if (shouldTransition)
		{
			for (auto& trigger : triggersUsed)
			{
				myAnimatorComponent->SetTriggerToBeTurnedFalse(trigger);
			}
		}
		return shouldTransition;
	}
	void AnimatorLayerInstance::UpdateTransition(bool aIsUnscaledDelta)
	{
		float deltaTime = 0.f;
		if (aIsUnscaledDelta)
		{
			deltaTime = Utils::Timer::GetUnscaledDeltaTime();
		}
		else
		{
			deltaTime = Utils::Timer::GetDeltaTime();
		}

		myTransitionTime += deltaTime;
		myBlendAnimationTime += deltaTime * myAnimatorLayer->GetState(myCurrentTransition.ToStateID).Speed;

		if (myTransitionTime >= myCurrentTransition.TransitionDuration)
		{
			SetCurrentState(myCurrentTransition.ToStateID);
			myCurrentAnimationTime = myBlendAnimationTime;
			myTransitionTime = 0;
			myIsTransitioning = false;
			myBlendAnimationTime = 0;

			if (myEndOfTransitionEnterFunctions.contains(myCurrentTransition.ToStateID))
			{
				myEndOfTransitionEnterFunctions[myCurrentTransition.ToStateID]();
			}
		}
	}
}