#include "FFpch.h"
#include "Firefly/ComponentSystem/ComponentSourceIncludes.h"
#include "AudioComponent.h"
#include "Firefly/Application/Application.h"
#include "Firefly/Event/EditorEvents.h"
#include "Firefly/Rendering/Renderer.h"

#include <sstream>

namespace Firefly
{
	REGISTER_COMPONENT(AudioComponent);

	AudioComponent::AudioComponent() : Component("AudioComponent")
	{
		myEventGUIDs = GetEventGUIDs("Master.bank");

		EditorVariable("Sound Event:", Firefly::ParameterType::Enum, &myIndex, AudioManager::ConvertGUIDsToEventNames(myEventGUIDs));
		EditorButton("Apply", [this]() {Apply(); });
		EditorVariable("Event Path:", Firefly::ParameterType::String, &myEventPath);
		EditorVariable("Use PlayEvent:", Firefly::ParameterType::Bool, &myPlayEvent);
		EditorVariable("Use PlayEventOneShot:", Firefly::ParameterType::Bool, &myPlayEventOneShot);
	}

	AudioComponent::~AudioComponent()
	{
		StopEvent();
	}

	void AudioComponent::Initialize()
	{

		if (myIndex < myEventGUIDs.size())
		{
			if (Application::Get().GetIsInPlayMode())
			{
				if (!mySoundEventInstanceHandle.IsValid())
				{
					mySoundEventInstanceHandle = AudioManager::CreateEventInstance(myEventPath);
				}

				AudioManager::Get3DMinMaxDistance(AudioManager::GetEventPath(myEventGUIDs[myIndex]), &myMin, &myMax);

				if (myPlayEvent)
				{
					PlayEvent();
				}

				if (myPlayEventOneShot)
				{
					PlayEventOneShot();
				}
			}
		}
		else
		{
			LOGERROR("Event GUID not found");
		}
	}

	void AudioComponent::OnEvent(Firefly::Event& aEvent)
	{
		Firefly::EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<EntityPropertyUpdatedEvent>(BIND_EVENT_FN(AudioComponent::OnPropertyUpdated));
		dispatcher.Dispatch<AppUpdateEvent>(BIND_EVENT_FN(AudioComponent::OnUpdate));
	}

	void AudioComponent::PlayEvent()
	{
#ifdef FF_DEBUG
		if (!myEntity->GetParent().expired())
		{
			LOGINFO("AudioComponent:: Played Event: {}  AT :{} IN :{}", mySoundEventInstanceHandle.GetEvent(), myEntity->GetName(), myEntity->GetParent().lock()->GetName());
		}
		else
		{
			LOGINFO("AudioComponent:: Played Event: {}  AT :{}", mySoundEventInstanceHandle.GetEvent(), myEntity->GetName());
		}
#endif

		AudioManager::PlayEvent(mySoundEventInstanceHandle);
	}

	void AudioComponent::PlayEventOneShot()
	{
		if (myIndex < myEventGUIDs.size())
		{
			AudioManager::PlayEventOneShot(AudioManager::GetEventPath(myEventGUIDs[myIndex]));
		}
	}

	void AudioComponent::StopEvent(bool immediately)
	{
		AudioManager::StopEvent(mySoundEventInstanceHandle, immediately);
	}

	void AudioComponent::SetEvent3DParameters(Utils::Vector3f aPosition)
	{
		AudioManager::SetEvent3DParameters(mySoundEventInstanceHandle, aPosition);
	}

	bool AudioComponent::StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode)
	{
		return AudioManager::StopEventsOnBus(aBusName, aMode);
	}

	void AudioComponent::DrawDebugMinMax()
	{
		Firefly::Renderer::SubmitDebugSphere(myEntity->GetTransform().GetPosition(), myMin, 25, { 0,0,1,1 });
		Firefly::Renderer::SubmitDebugSphere(myEntity->GetTransform().GetPosition(), myMax, 25, { 1,0,0,1 });
	}

	void AudioComponent::StartStop()
	{
		if (myStoped)
		{
			myStoped = false;
			PlayEvent();
		}

		else if (!myStoped)
		{
			myStoped = true;
			StopEvent();
		}
	}

	bool AudioComponent::OnPropertyUpdated(Firefly::EntityPropertyUpdatedEvent& aEvent)
	{
		if (myIndex < myEventGUIDs.size())
		{
			if (mySoundEventInstanceHandle.GetEvent() != AudioManager::GetEventPath(myEventGUIDs[myIndex]))
			{
				StopEvent();
				mySoundEventInstanceHandle = AudioManager::CreateEventInstance(myEventPath);
				AudioManager::Get3DMinMaxDistance(AudioManager::GetEventPath(myEventGUIDs[myIndex]), &myMin, &myMax);
			}
		}

		if (myPlayEvent)
		{
			myPlayEventOneShot = false;
		}

		if (myPlayEventOneShot)
		{
			myPlayEvent = false;
		}

		return false;
	}
	bool AudioComponent::OnUpdate(AppUpdateEvent& aUpdateEvent)
	{
		if (Application::Get().GetIsInPlayMode())
		{
			AudioManager::SetEvent3DParameters(mySoundEventInstanceHandle, myEntity->GetTransform().GetPosition());
		}

		if (myUseDebugView)
		{
			DrawDebugMinMax();
		}

		return false;
	}
	bool AudioComponent::OnStopEvent(EditorStopEvent& aEditorStopEvent)
	{
		StopEvent();
		return false;
	}


	std::vector<FMOD_GUID> AudioComponent::GetEventGUIDs(const std::string& bankName)
	{
		return AudioManager::GetEventGUIDs(bankName);
	}

	FMOD_GUID AudioComponent::GetEventGUID(const std::string& eventPath)
	{
		return AudioManager::GetEventGUID(eventPath);
	}

	void AudioComponent::Apply()
	{
		myEventPath = AudioManager::GetEventPath(myEventGUIDs[myIndex]);

		if (myEntity->IsPrefab())
		{
			Firefly::EntityModification mod;
			mod.Key = GetName() + "_Event Path:";
			mod.StringValues.clear();
			mod.StringValues.push_back(myEventPath);
			mod.ID = myEntity->GetID();

			myEntity->AddModification(mod);
		}
	}
}
