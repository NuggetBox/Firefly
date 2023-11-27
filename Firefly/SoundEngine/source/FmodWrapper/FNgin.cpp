#include "SoundEngine.pch.h"
#include "FNgin.h"
#include "FmodPathDefines.h"
#include "FNginStructs.h"

FNgin::FNgin()
{
}

FNgin::~FNgin()
{
	myFModStudioPtr->unloadAll();
	myFModStudioPtr->release();
}

bool FNgin::Initialization(const std::string& aRootPath, unsigned int maxChannels)
{
	myRootDirectory = aRootPath;
	if (!myRootDirectory.empty() && myRootDirectory.back() != '\\')
	{
		myRootDirectory.append("\\");
	}

	FMOD::Studio::System::create(&myFModStudioPtr);
	myLastError = myFModStudioPtr->initialize(static_cast<int>(maxChannels), FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, nullptr);

	if (myLastError != FMOD_OK)
		return false;

	myLastError = myFModStudioPtr->getCoreSystem(&myFModPtr);
	if (myLastError != FMOD_OK)
		return false;

	myChannels.reserve(maxChannels);



	isInitialized = true;
	return isInitialized;
}

void FNgin::Update()
{
	assert(isInitialized && "FNgin::Init has not been called!");
	std::vector<ChannelMap::iterator> stopped;
	for (ChannelMap::iterator It = myChannels.begin(); It != myChannels.end(); ++It)
	{
		bool isActive = false;
		It->second->isPlaying(&isActive);
		if (!isActive)
		{
			stopped.push_back(It);
		}
	}

	for (const auto& ch : stopped)
	{
		myChannels.erase(ch);
	}

	myFModStudioPtr->update();
}

bool FNgin::LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (myBanks.find(aFileName) != myBanks.end())
		return true;  // Already loaded.

	FMOD::Studio::Bank* soundBank = nullptr;
	std::string a = myRootDirectory + aFileName;
	myLastError = myFModStudioPtr->loadBankFile((myRootDirectory + aFileName).c_str(), someFlags, &soundBank);
	if (soundBank)
	{
		myBanks[aFileName] = soundBank;
		FMOD::Studio::EventDescription* eventList[512]{};

		int eventCount = 0;
		soundBank->getEventCount(&eventCount);
		if (eventCount > 0)
		{
			FMOD_RESULT result = soundBank->getEventList(&*eventList, 512, &eventCount);
			for (auto ptr : eventList)
			{
				char path[512];
				int size = 0;
				result = ptr->getPath(path, 512, &size);
				std::string p(path);
				myEvents[path] = { path, false, nullptr };
			}
		}

		return true;
	}

	return false;
}

bool FNgin::LoadEvent(const std::string& anEventName)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (auto It = myEvents.find(anEventName); It != myEvents.end())
	{
		if (!It->second.IsLoaded)
		{
			FMOD::Studio::EventDescription* eventDesc = nullptr;
			myLastError = myFModStudioPtr->getEvent(anEventName.c_str(), &eventDesc);
			if (eventDesc)
			{
				It->second.FmodEventDesc = eventDesc;
				It->second.IsLoaded = true;
				return true;

			}
		}
	}
	return false;
}

unsigned FNgin::GetChannels(std::vector<std::string>& outChannelList)
{
	unsigned int count = 0;
	for (auto It = myChannels.begin(); It != myChannels.end(); ++It)
	{
		//outChannelList.push_back(It->second->getChannelGroup());
		count++;
	}

	return count;
}

unsigned FNgin::GetBus(std::vector<std::string>& outBusList)
{
	unsigned int count = 0;
	for (auto It = myBus.begin(); It != myBus.end(); ++It)
	{
		//outBusList.push_back(It->second.Path);
		count++;
	}

	return count;
}

std::vector<FMOD_GUID> FNgin::GetEventGUIDs(const std::string& bankName)
{
	auto bankIt = myBanks.find(bankName);
	if (bankIt == myBanks.end()) return {};

	FMOD::Studio::Bank* bank = bankIt->second;

	int eventCount;
	bank->getEventCount(&eventCount);

	std::vector<FMOD::Studio::EventDescription*> eventDescriptions(eventCount);
	bank->getEventList(eventDescriptions.data(), eventCount, nullptr);

	std::vector<FMOD_GUID> eventGUIDs(eventCount);
	for (int i = 0; i < eventCount; ++i)
	{
		eventDescriptions[i]->getID(&eventGUIDs[i]);
	}

	return eventGUIDs;
}

FMOD_GUID FNgin::GetEventGUID(const std::string& eventPath)
{
	auto it = myEventPathToGUID.find(eventPath);
	if (it != myEventPathToGUID.end())
	{
		return it->second;
	}
	else
	{
		return FMOD_GUID(); // Return an empty GUID if not found
	}
}

unsigned FNgin::GetEvents(std::vector<std::string>& outEventList)
{
	unsigned int count = 0;
	for (auto It = myEvents.begin(); It != myEvents.end(); ++It)
	{
		outEventList.push_back(It->second.Path);
		count++;
	}

	return count;
}

bool FNgin::RegisterEvent(const std::string& anEventName, unsigned int anEventId)
{
	if (myEventAccelMap.find(anEventId) == myEventAccelMap.end())
	{
		myEventAccelMap[anEventId] = anEventName;
		return true;
	}

	return false;
}

bool FNgin::ReleaseEvent(const SoundEventInstanceHandle& aHandle)
{
	if (aHandle.IsValid())
	{
		assert(isInitialized && "FNgin::Init has not been called!");
		const auto foundEvent = myEvents.find(aHandle.GetEvent());
		if (foundEvent != myEvents.end())
		{
			if (const auto foundInstance = foundEvent->second.FmodEventInstances.find(aHandle.GetId()); foundInstance != foundEvent->second.FmodEventInstances.end())
			{
				myLastError = foundInstance->second->release();
				return myLastError == FMOD_OK;
			}
		}
	}

	myLastError = FMOD_ERR_EVENT_NOTFOUND;
	return false;
}

bool FNgin::ReleaseEvent(const std::string& anEventInstanceString)
{
	assert(isInitialized && "FNgin::Init has not been called!");

	// Iterate over registered events
	for (const auto& eventEntry : myEvents)
	{
		// Iterate over event instances
		for (const auto& instanceEntry : eventEntry.second.FmodEventInstances)
		{
			// Use the existing ReleaseEvent function with the SoundEventInstanceHandle
			SoundEventInstanceHandle handle(eventEntry.first, instanceEntry.first);
			if (handle.GetEvent() == anEventInstanceString)
			{
				return ReleaseEvent(handle);
			}
		}
	}

	myLastError = FMOD_ERR_EVENT_NOTFOUND;
	return false;
}

//Returns my SoundEventInstanceHandle 
SoundEventInstanceHandle FNgin::CreateEventInstance(const std::string& anEventName)
{
	if (!isInitialized)
	{
		Initialization("Assets/AudioBanks/Build/Desktop");
	}
	assert(isInitialized && "FNgin::Init has not been called!");
	auto foundEvent = myEvents.find(anEventName);
	if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
	{
		LoadEvent(anEventName);
		foundEvent = myEvents.find(anEventName);
		if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
			return SoundEventInstanceHandle::InvalidHandle;
	}

	Event& theEvent = foundEvent->second;

	FMOD::Studio::EventInstance* eventInstance;
	myLastError = theEvent.FmodEventDesc->createInstance(&eventInstance);
	if (eventInstance)
	{
		SoundEventInstanceHandle result(anEventName, theEvent.NextInstanceId++);
		theEvent.FmodEventInstances.insert({ result.GetId(),eventInstance });
		myLastError = FMOD_OK;
		return result;
	}

	return SoundEventInstanceHandle::InvalidHandle;
}
//Returns FMODS EventInstance 
FMOD::Studio::EventInstance* FNgin::CreateEventInstanceFmod(const std::string& anEventName)
{
	if (!isInitialized)
	{
		Initialization("Assets/AudioBanks/Build/Desktop");
	}
	assert(isInitialized && "FNgin::Init has not been called!");
	auto foundEvent = myEvents.find(anEventName);
	if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
	{
		LoadEvent(anEventName);
		foundEvent = myEvents.find(anEventName);
		if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
			return nullptr; 
	}

	Event& theEvent = foundEvent->second;

	FMOD::Studio::EventInstance* eventInstance;
	myLastError = theEvent.FmodEventDesc->createInstance(&eventInstance);
	if (eventInstance)
	{
		SoundEventInstanceHandle result(anEventName, theEvent.NextInstanceId++);
		theEvent.FmodEventInstances.insert({ result.GetId(),eventInstance });
		myLastError = FMOD_OK;
		return eventInstance; 
	}

	return nullptr; 
}


bool FNgin::SetEvent3DParameters(const SoundEventInstanceHandle& aHandle, std::array<float, 3> aPosition, std::array<float, 3> aVelocity, std::array<float, 3> aForwardVector, std::array<float, 3> aUpVector)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (!aHandle.IsValid())
		return false;

	const auto foundEvent = myEvents.find(aHandle.myEventName);
	if (foundEvent == myEvents.end())
	{
		return false;
	}

	FMOD_VECTOR pos, vel, fwd, up;
	pos = { aPosition[0], aPosition[1], aPosition[2] };
	vel = { aVelocity[0], aVelocity[1], aVelocity[2] };
	fwd = { aForwardVector[0], aForwardVector[1], aForwardVector[2] };
	up = { aUpVector[0], aUpVector[1], aUpVector[2] };
	const FMOD_3D_ATTRIBUTES attributes = { pos, vel, fwd, up };

	myLastError = foundEvent->second.FmodEventInstances[aHandle.GetId()]->set3DAttributes(&attributes);
	return myLastError == FMOD_OK;
}

bool FNgin::PlayEvent(const SoundEventInstanceHandle& anEventInstanceHandle)
{
	if (anEventInstanceHandle.IsValid())
	{
		assert(isInitialized && "FNgin::Init has not been called!");
		const auto foundEvent = myEvents.find(anEventInstanceHandle.GetEvent());
		if (foundEvent != myEvents.end())
		{
			if (const auto foundInstance = foundEvent->second.FmodEventInstances.find(anEventInstanceHandle.GetId()); foundInstance != foundEvent->second.FmodEventInstances.end())
			{
				myLastError = foundInstance->second->start();
				return myLastError == FMOD_OK;
			}
		}
	}

	myLastError = FMOD_ERR_EVENT_NOTFOUND;
	return false;
}

bool FNgin::PlayEventOneShot(const std::string& anEventName)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	auto foundEvent = myEvents.find(anEventName);
	if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
	{
		LoadEvent(anEventName);
		foundEvent = myEvents.find(anEventName);
		if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
			return false;
	}

	FMOD::Studio::EventInstance* eventInstance;
	myLastError = foundEvent->second.FmodEventDesc->createInstance(&eventInstance);
	if (eventInstance)
	{
		myLastError = eventInstance->start();
		myLastError = eventInstance->release();
	}

	return myLastError == FMOD_OK;
}

bool FNgin::PlayEventOneShotWithParameters(const std::string& anEventName, std::array<float, 3> aPosition, std::array<float, 3> aVelocity, std::array<float, 3> aForwardVector, std::array<float, 3> aUpVector, int* aLength)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	auto foundEvent = myEvents.find(anEventName);
	if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
	{
		LoadEvent(anEventName);
		foundEvent = myEvents.find(anEventName);
		if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
			return false;
	}

	FMOD::Studio::EventInstance* eventInstance;
	myLastError = foundEvent->second.FmodEventDesc->createInstance(&eventInstance);
	if (eventInstance)
	{
		FMOD_VECTOR pos, vel, fwd, up;
		pos = { aPosition[0], aPosition[1], aPosition[2] };
		vel = { aVelocity[0], aVelocity[1], aVelocity[2] };
		fwd = { aForwardVector[0], aForwardVector[1], aForwardVector[2] };
		up = { aUpVector[0], aUpVector[1], aUpVector[2] };
		const FMOD_3D_ATTRIBUTES attributes = { pos, vel, fwd, up };
		myLastError = eventInstance->set3DAttributes(&attributes);
		myLastError = eventInstance->start();

		//Get Lenght off sound event
		FMOD::Studio::EventDescription* eventDescription = nullptr;
		if (eventInstance->getDescription(&eventDescription) != FMOD_OK)
		{
			return myLastError == FMOD_ERR_FILE_BAD;
		}
		eventDescription->getLength(aLength);

		myLastError = eventInstance->release();
	}

	return myLastError == FMOD_OK;
}

bool FNgin::StopEvent(const SoundEventInstanceHandle& aHandle, bool immediately)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (!aHandle.IsValid())
		return false;

	const auto foundEvent = myEvents.find(aHandle.myEventName);
	if (foundEvent == myEvents.end())
	{
		return false;
	}

	myLastError = FMOD_ERR_INVALID_HANDLE;

	if (const auto instance = foundEvent->second.FmodEventInstances.find(aHandle.GetId());
		instance != foundEvent->second.FmodEventInstances.end())
	{
		const FMOD_STUDIO_STOP_MODE stopMode = immediately ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
		myLastError = instance->second->stop(stopMode);
	}

	return myLastError == FMOD_OK;
}

FMOD_STUDIO_PLAYBACK_STATE FNgin::GetPlaybackState(const SoundEventInstanceHandle& aHandle)
{
	const auto foundEvent = myEvents.find(aHandle.myEventName);

	if (const auto instance = foundEvent->second.FmodEventInstances.find(aHandle.GetId());
		instance != foundEvent->second.FmodEventInstances.end())
	{
		FMOD_STUDIO_PLAYBACK_STATE state;
		myLastError = instance->second->getPlaybackState(&state);
		return state;
	}
}

int FNgin::GetNumListeners()
{
	assert(isInitialized && "FNgin::Init has not been called!");
	int numListeners;
	myFModPtr->get3DNumListeners(&numListeners);
	return numListeners;
}

//FNgin::ListenerHandle FNgin::GetNextFreeListener()
//{
//	const int numListeners = FNgin::GetNumListeners(); // We always have at least one listener.
//	if (numListeners < 8)
//	{
//		FNgin::Listener newListener;
//		newListener.FmodId = numListeners;
//		newListener.ListenerId = ++nextListenerId;
//
//		assert(FNgin::AddListener(newListener.Location, newListener.Velocity, newListener.Forward, newListener.Up));
//		myListeners[newListener.ListenerId] = newListener;
//
//		return newListener.Handle();
//	}
//
//	return FNgin::ListenerHandle();
//}

bool FNgin::AddListener(const std::array<float, 3> aPosition, const std::array<float, 3> aVelocity, const std::array<float, 3> aForwardVector, const std::array<float, 3> aUpVector)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	const int numListeners = GetNumListeners();
	if (numListeners < 8)
	{
		myFModPtr->set3DNumListeners(numListeners + 1);

		FMOD_VECTOR pos, vel, fwd, up;
		pos = { aPosition[0], aPosition[1], aPosition[2] };
		vel = { aVelocity[0], aVelocity[1], aVelocity[2] };
		fwd = { aForwardVector[0], aForwardVector[1], aForwardVector[2] };
		up = { aUpVector[0], aUpVector[1], aUpVector[2] };

		myLastError = myFModPtr->set3DListenerAttributes(numListeners, &pos, &vel, &fwd, &up);
	}
	else
	{
		myLastError = FMOD_ERR_TOOMANYCHANNELS;
	}

	return myLastError == FMOD_OK;
}

bool FNgin::RemoveListener(int anIndex)
{
	assert(isInitialized && "SoundEngine::Init has not been called!");
	const int numListeners = GetNumListeners();
	if (anIndex >= numListeners || numListeners == 1)
		return false;

	if (anIndex != numListeners)
	{
		// And here we go, move everything after this index forward.
		for (int i = anIndex; i < numListeners - 1; i++)
		{
			FMOD_VECTOR pos, vel, fwd, up;
			myFModPtr->get3DListenerAttributes(i + 1, &pos, &vel, &fwd, &up);
			myFModPtr->set3DListenerAttributes(i, &pos, &vel, &fwd, &up);
		}
	}

	myLastError = myFModPtr->set3DNumListeners(numListeners - 1);

	return myLastError == FMOD_OK;
}

bool FNgin::SetListenerAttributes(int listener, std::array<float, 3> aPosition, std::array<float, 3> aVelocity, std::array<float, 3> aForwardVector, std::array<float, 3> aUpVector, const FMOD_VECTOR* attenuationposition)
{
	FMOD_VECTOR pos, vel, fwd, up;
	pos = { aPosition[0], aPosition[1], aPosition[2] };
	vel = { aVelocity[0], aVelocity[1], aVelocity[2] };
	fwd = { aForwardVector[0], aForwardVector[1], aForwardVector[2] };
	up = { aUpVector[0], aUpVector[1], aUpVector[2] };
	const FMOD_3D_ATTRIBUTES attributes = { pos, vel, fwd, up };

	return myFModStudioPtr->setListenerAttributes(listener, &attributes, attenuationposition);
}


bool FNgin::GetBusPaused(const std::string& aBusName, bool* aOnOff)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myFModStudioPtr->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->getPaused(aOnOff);
	}
	return false;
}

bool FNgin::SetBusPaused(const std::string& aBusName, bool aOnOff)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myFModStudioPtr->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->setVolume(aOnOff);
	}
	return false;
}

bool FNgin::GetBusVolume(const std::string& aBusName, float* aVolume)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myFModStudioPtr->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->getVolume(aVolume);
	}
	return false;
}

bool FNgin::SetBusVolume(const std::string& aBusName, float aVolume)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myFModStudioPtr->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->setVolume(aVolume);
	}
	return false;
}

bool FNgin::SetChannelVolume(unsigned int aChannel, float aVolumePct)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (auto ch = myChannels.find(aChannel); ch == myChannels.end())
	{
		myLastError = FMOD_ERR_INVALID_HANDLE;
	}
	else
	{
		myLastError = ch->second->setVolume(aVolumePct);
	}

	return myLastError == FMOD_OK;
}

bool FNgin::SetSoundEventVolume(const SoundEventInstanceHandle& aHandle, float aVolumePct)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (!aHandle.IsValid())
		return false;

	const auto foundEvent = myEvents.find(aHandle.myEventName);
	if (foundEvent == myEvents.end())
	{
		return false;
	}

	myLastError = FMOD_ERR_INVALID_HANDLE;

	if (const auto instance = foundEvent->second.FmodEventInstances.find(aHandle.GetId());
		instance != foundEvent->second.FmodEventInstances.end())
	{
		myLastError = instance->second->setVolume(aVolumePct);
	}

	return myLastError == FMOD_OK;
}

FMOD::ChannelGroup* FNgin::GetMasterChannelGroup()
{
	FMOD::System* system;

	// Create an instance of the FMOD::System class
	FMOD::System_Create(&system);

	// Initialize the FMOD system
	system->init(32, FMOD_INIT_NORMAL, nullptr);

	// Get the master channel group for the FMOD system
	system->getMasterChannelGroup(&myMastergroup);

	// Release the FMOD system instance
	system->release();

	// Return the master channel group
	return myMastergroup;
}


bool FNgin::GetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float* outValue)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (!aHandle.IsValid())
		return false;

	const auto foundEvent = myEvents.find(aHandle.myEventName);
	if (foundEvent == myEvents.end())
	{
		return false;
	}

	myLastError = FMOD_ERR_INVALID_HANDLE;

	if (const auto instance = foundEvent->second.FmodEventInstances.find(aHandle.GetId());
		instance != foundEvent->second.FmodEventInstances.end())
	{
		myLastError = instance->second->getParameterByName(aParameterName.c_str(), outValue);
	}

	return myLastError == FMOD_OK;
}

bool FNgin::SetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float aValue)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	if (!aHandle.IsValid())
		return false;

	const auto foundEvent = myEvents.find(aHandle.myEventName);
	if (foundEvent == myEvents.end())
	{
		return false;
	}

	myLastError = FMOD_ERR_INVALID_HANDLE;

	if (const auto instance = foundEvent->second.FmodEventInstances.find(aHandle.GetId());
		instance != foundEvent->second.FmodEventInstances.end())
	{
		myLastError = instance->second->setParameterByName(aParameterName.c_str(), aValue);
	}

	return myLastError == FMOD_OK;
}

bool FNgin::SetGlobalParameter(const std::string& aParameterName, float aValue, bool ignoreSeekSpeed)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	myLastError = myFModStudioPtr->setParameterByName(aParameterName.c_str(), aValue, ignoreSeekSpeed);
	return myLastError == FMOD_OK;
}

bool FNgin::GetGlobalParameter(const std::string& aParameterName, float* outValue)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	myLastError = myFModStudioPtr->getParameterByName(aParameterName.c_str(), outValue);
	return myLastError == FMOD_OK;
}

bool FNgin::StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode)
{
	FMOD::Studio::Bus* aBus = nullptr;
	if (!myFModStudioPtr->getBus(aBusName.c_str(), &aBus))
	{
		return aBus->stopAllEvents(aMode);
	}
	return false;
}

bool FNgin::Get3DMinMaxDistance(const std::string& anEventName, float* aMinDistance, float* aMaxDistance)
{
	assert(isInitialized && "FNgin::Init has not been called!");
	auto foundEvent = myEvents.find(anEventName);
	if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
	{
		LoadEvent(anEventName);
		foundEvent = myEvents.find(anEventName);
		if (foundEvent == myEvents.end() || !foundEvent->second.IsLoaded)
			return false;
	}

	FMOD::Studio::EventInstance* eventInstance;
	myLastError = foundEvent->second.FmodEventDesc->getMinMaxDistance(aMinDistance, aMaxDistance);

	return myLastError == FMOD_OK;
}

bool FNgin::GetEventTimelinePosition(const SoundEventInstanceHandle& aHandle, int* position)
{
	if (aHandle.IsValid())
	{
		FMOD::Studio::EventInstance* eventInstance = aHandle.GetEventInstance();
		if (eventInstance)
		{
			FMOD_RESULT result = eventInstance->getTimelinePosition(position);
			if (result == FMOD_OK)
				return true;
			else
				myLastError = result;
		}
	}

	return false;
}

void FNgin::SetMasterVolume(float aVolume)
{
	myMasterVolume = aVolume;
	SetBusVolume(BUS_MASTER, myMasterVolume);
}

void FNgin::SetSFXVolume(float aVolume)
{
	mySFXVolume = aVolume;
	SetBusVolume(BUS_SFX, myMasterVolume);

}

void FNgin::SetMusicVolume(float aVolume)
{
	myMusicVolume = aVolume;
	SetBusVolume(BUS_MUSIC, myMasterVolume);
}

void FNgin::SetAmbientVolume(float aVolume)
{
	myAmbientVolume = aVolume;
	SetBusVolume(BUS_AMBIANCE, myMasterVolume);
}
