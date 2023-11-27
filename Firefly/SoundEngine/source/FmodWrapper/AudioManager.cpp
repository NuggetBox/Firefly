#include "SoundEngine.pch.h"
#include "AudioManager.h"
#include "Utils/InputHandler.h"
#include "Firefly/Core/Log/DebugLogger.h"
#include <cstdio>
#include <iomanip>
#include <algorithm>

AudioManager::AudioManager()
{
	mySFXVolume = 0.f;
	myMaxSFXVolume = 1.f;
	//myCurrentSFXVolume = 0.f;
	myMusicVolume = 0.f;
	myMaxMusicVolume = 1.f;
	//myCurrentMusicVolume = 0.f;
	aNumListeners = 0;
	myListenerPosition = { 0,0,0 };
}

bool AudioManager::Initialization()
{
	myFNgin = new FNgin();


	if (myFNgin->Initialization("Assets\\Audio\\Desktop"))
	{
		myFNgin->LoadBank("Master.strings.bank", 0);
		myFNgin->LoadBank("Master.bank", 0);

		std::vector<std::string> myEvents;
		myFNgin->GetEvents(myEvents);

		std::sort(myEvents.begin(), myEvents.end());

		std::vector<FMOD_GUID> eventGUIDs = myFNgin->GetEventGUIDs("Master.bank");


		for (int i = 0; i < myEvents.size(); i++)
		{
			if (myEvents[i] != "" && i < eventGUIDs.size())
			{
				if (myFNgin->RegisterEvent(myEvents[i], i))
				{
					// Convert the GUID to a string representation
					char guidString[64];
					const FMOD_GUID eventGUID = eventGUIDs[i];
					int numChars = snprintf(guidString, sizeof(guidString), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
						eventGUID.Data1, eventGUID.Data2, eventGUID.Data3,
						eventGUID.Data4[0], eventGUID.Data4[1], eventGUID.Data4[2], eventGUID.Data4[3],
						eventGUID.Data4[4], eventGUID.Data4[5], eventGUID.Data4[6], eventGUID.Data4[7]);

					if (numChars > 0 && numChars < sizeof(guidString)) {
						LOGINFO(" {:<3}: {:<40}: {}", i, myEvents[i], guidString);
					}
					else {
						LOGERROR("Failed to format GUID string for event {}: {}", i, myEvents[i]);
					}
					myEventStrings.push_back(myEvents[i]);
					myEventGUIDs.push_back(eventGUIDs[i]);
				}
			}
		}

		mySFXVolume = 0.f;
		myMaxSFXVolume = 1.f;
		myCurrentSFXVolume = 0.f;
		myMusicVolume = 0.f;
		myMaxMusicVolume = 1.f;
		myCurrentMusicVolume = 0.f;

		bool c;
		c = GetBusVolume(BUS_SFX, &mySFXVolume); // returns false but get the volume
		c = GetBusVolume(BUS_SFX, &myCurrentSFXVolume);
		LOGINFO("GetBusSFXVolume: {}", c);

		c = GetBusVolume(BUS_MUSIC, &myMusicVolume); // returns false but get the volume
		c = GetBusVolume(BUS_MUSIC, &myCurrentMusicVolume);
		LOGINFO("GetBusMusicVolume: {}", c);

		//So no sound is on by default
		//StartStop(false);

		//AudioManager audioManager;
		//audioManager.AudioTimerTest();


		return true;
	}
	else
	{
		return false;
	}

	//AudioManager::SetGlobalParameter(PARAMETER_Material, 2);


}

void AudioManager::AudioTimerTest()
{
	//AUIDO WITH TIMER TEST.


	//const auto& timer = Utils::TimerManager::AddTimer([this]() { OnTimer(); }, 5, true, true);
}

void AudioManager::OnTimer()
{
	//LOGINFO("OnTimer Test");
	//PlayEventOneShot(EVENT_Platform_MovementLong);
}

void AudioManager::Update()
{
	myFNgin->Update();

	if (myCurrentSFXVolume > mySFXVolume || myCurrentSFXVolume < mySFXVolume)
	{
		//LOGINFO("Volume change from: {} to {}", mySFXVolume, myCurrentSFXVolume);
		mySFXVolume = myCurrentSFXVolume;
		SetBusVolume(BUS_SFX, mySFXVolume);
		SetBusVolume(BUS_UI, mySFXVolume);
		SetBusVolume(BUS_VOICELINES, mySFXVolume);
	}


	if (myCurrentMusicVolume > myMusicVolume || myCurrentMusicVolume < myMusicVolume)
	{
		//LOGINFO("Volume change from: {} to {}", myMusicVolume, myCurrentMusicVolume);
		myMusicVolume = myCurrentMusicVolume;
		SetBusVolume(BUS_MUSIC, myMusicVolume);
		SetBusVolume(BUS_AMBIANCE, myMusicVolume);
	}
}

bool AudioManager::PlayEvent(const SoundEventInstanceHandle& anEventInstanceHandle)
{
#ifdef FF_DEBUG
	LOGINFO("AudioManager:: Played Event: {}", anEventInstanceHandle.GetEvent());
#endif

	return myFNgin->PlayEvent(anEventInstanceHandle);
}

bool AudioManager::PlayEventOneShot(const std::string& aEventPathDefine)
{
#ifdef FF_DEBUG
	LOGINFO("AudioManager:: Played Event One Shot: {}", aEventPathDefine);
#endif

	return myFNgin->PlayEventOneShot(aEventPathDefine);
}

bool AudioManager::PlayEventOneShotWithParameters(const std::string& aEventPathDefine, std::array<float, 3> aPosition, std::array<float, 3> aVelocity, std::array<float, 3> aForwardVector, std::array<float, 3> aUpVector, int* aLength)
{
	return myFNgin->PlayEventOneShotWithParameters(aEventPathDefine, aPosition, aVelocity, aForwardVector, aUpVector, aLength);
}

bool AudioManager::StopEvent(const SoundEventInstanceHandle& anEventInstanceHandle, bool immediately)
{
	return myFNgin->StopEvent(anEventInstanceHandle, immediately);
}

SoundEventInstanceHandle AudioManager::CreateEventInstance(const std::string& aEventPathDefine)
{
	return  myFNgin->CreateEventInstance(aEventPathDefine);
}

SoundEventInstanceHandle AudioManager::CreateEventInstanceFmod(const std::string& aEventPathDefine)
{

	FMOD::Studio::EventInstance* eventInstance = myFNgin->CreateEventInstanceFmod(aEventPathDefine);

	SoundEventInstanceHandle handle;
	handle.SetEvent(aEventPathDefine);
	handle.SetEventInstance(eventInstance);

	return handle;
}


bool AudioManager::SetEvent3DParameters(const SoundEventInstanceHandle& anEventInstanceHandle, Utils::Vector3f aPosition, Utils::Vector3f aVelocity, Utils::Vector3f aForwardVector, Utils::Vector3f aUpVector )
{
	std::array<float, 3> pos, vel, fwd, up;
	pos[0] = aPosition.x;
	pos[1] = aPosition.y;
	pos[2] = aPosition.z;

	vel[0] = aVelocity.x;
	vel[1] = aVelocity.y;
	vel[2] = aVelocity.z;

	fwd[0] = aForwardVector.x;
	fwd[1] = aForwardVector.y;
	fwd[2] = aForwardVector.z;

	up[0] = aUpVector.x;
	up[1] = aUpVector.y;
	up[2] = aUpVector.z;

	//LOGINFO("SetEvent3DParameters :{} X:{} Y:{} Z:{}", anEventInstanceHandle.GetEvent(), pos[0], pos[1], pos[2]);

	return myFNgin->SetEvent3DParameters(anEventInstanceHandle, pos,vel, fwd, up);
}

bool AudioManager::SetEvent3DPosition(const SoundEventInstanceHandle& anEventInstanceHandle, Utils::Vector3f aPosition = { 0,0,0 })
{
	std::array<float, 3> pos;
	pos[0] = aPosition.x;
	pos[1] = aPosition.y;
	pos[2] = aPosition.z;
	return myFNgin->SetEvent3DParameters(anEventInstanceHandle, pos);
}


bool AudioManager::SetListenerAttributes(int aHandle, Utils::Vector3f aPosition, Utils::Vector3f aVelocity, Utils::Vector3f aForwardVector, Utils::Vector3f aUpVector)
{
	std::array<float, 3> pos, vel, fwd, up;
	pos[0] = aPosition.x;
	pos[1] = aPosition.y;
	pos[2] = aPosition.z;

	vel[0] = aVelocity.x;
	vel[1] = aVelocity.y;
	vel[2] = aVelocity.z;

	fwd[0] = aForwardVector.x;
	fwd[1] = aForwardVector.y;
	fwd[2] = aForwardVector.z;

	up[0] = aUpVector.x;
	up[1] = aUpVector.y;
	up[2] = aUpVector.z;

	myListenerPosition = aPosition;

	return myFNgin->SetListenerAttributes(aHandle, pos, vel, fwd, up);
}

bool AudioManager::GetBusVolume(const std::string& aBusName, float* aVolumePct)
{
	return myFNgin->GetBusVolume(aBusName, aVolumePct);
}

bool AudioManager::SetBusVolume(const std::string& aBusName, float aVolumePct)
{
	return myFNgin->SetBusVolume(aBusName, aVolumePct);
}

bool AudioManager::GetBusPaused(const std::string& aBusName, bool* aOnOff)
{
	return myFNgin->GetBusPaused(aBusName, aOnOff);
}

bool AudioManager::SetBusPaused(const std::string& aBusName, bool aOnOff)
{
	return myFNgin->SetBusPaused(aBusName, aOnOff);
}

bool AudioManager::StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode)
{
	return 	myFNgin->StopEventsOnBus(aBusName, aMode);
}

float* AudioManager::GetBusVolumePTR(const std::string& aBusName)
{
	float* aVolumePct = 0;
	myFNgin->GetBusVolume(aBusName, aVolumePct);

	return aVolumePct;
}

bool AudioManager::SetChannelVolume(unsigned int aChannel, float aVolumePct)
{
	return myFNgin->SetChannelVolume(aChannel, aVolumePct);
}

bool AudioManager::SetEventVolume(const SoundEventInstanceHandle& aHandle, float aVolumePct)
{
	return myFNgin->SetSoundEventVolume(aHandle, aVolumePct);
}

bool AudioManager::GetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float* outValue)
{
	return myFNgin->GetEventParameter(aHandle, aParameterName, outValue);
}

bool AudioManager::SetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float aValue)
{
	return myFNgin->SetEventParameter(aHandle, aParameterName, aValue);
}

bool AudioManager::SetGlobalParameter(const std::string& aParameterName, float aValue, bool ignoreSeekSpeed)
{
	//LOGINFO("SetGlobalParameter :{} Value:{}", aParameterName, aValue);
	return myFNgin->SetGlobalParameter(aParameterName, aValue, ignoreSeekSpeed);
}

bool AudioManager::GetGlobalParameter(const std::string& aParameterName, float* outValue)
{
	return myFNgin->GetGlobalParameter(aParameterName, outValue);
}

bool AudioManager::StartStop(bool aOn)
{
	//disabled sound fornow its a bit anoonying
	if (aOn)
	{
		LOGINFO("Sound ON");
		myFNgin->SetBusVolume(BUS_MASTER, 1.f);
		isOn = true;
		return isOn;
	}
	else
	{
		LOGINFO("Sound Off");
		myFNgin->SetBusVolume(BUS_MASTER, 0.f);
		isOn = false;
		return isOn;
	}

	////FlipFlop if on turn off/If off turn On
	//isOn = aOn;
	//if (isOn)
	//{
	//	LOGINFO("Sound Off");
	//	myFNgin->PlayEventOneShot(EVENT_UI_Click);
	//	myFNgin->SetBusVolume(BUS_MASTER, 0.f);
	//	isOn = false;
	//	return isOn;
	//}
	//else
	//{
	//	LOGINFO("Sound ON");
	//	myFNgin->SetBusVolume(BUS_MASTER, 1.f);
	//	myFNgin->PlayEventOneShot(EVENT_UI_Click);
	//	isOn = true;
	//	return isOn;
	//}
}

const std::vector<std::string>& AudioManager::GetEvents()
{
	return myEventStrings;
}

bool AudioManager::Get3DMinMaxDistance(const std::string& anEventName, float* aMinDistance, float* aMaxDistance)
{
	return myFNgin->Get3DMinMaxDistance(anEventName, aMinDistance, aMaxDistance);
}

FMOD_STUDIO_PLAYBACK_STATE AudioManager::GetPlaybackState(const SoundEventInstanceHandle& aHandle)
{
	return myFNgin->GetPlaybackState(aHandle);
}

bool AudioManager::GetIsPlaying(const SoundEventInstanceHandle& aHandle)
{
	const int state = GetPlaybackState(aHandle);
	if (state != 0 && state != 3)
	{
		return false;
	}
	return true;
}

bool AudioManager::ReleaseEvent(const SoundEventInstanceHandle& anEventInstanceHandle)
{
	return myFNgin->ReleaseEvent(anEventInstanceHandle);
}

bool AudioManager::ReleaseEvent(const std::string& anEventInstanceString)
{
	return myFNgin->ReleaseEvent(anEventInstanceString);
}


FMOD::ChannelGroup* AudioManager::GetMasterChannelGroup()
{
	return myFNgin->GetMasterChannelGroup();
}

SERESULT AudioManager::GetLastError()
{
	//Daniels sätt
#pragma warning(disable: 26812)
	myLastError = static_cast<SERESULT>(myFNgin->GetLastError());
	return myLastError;
#pragma warning(default: 26812)
}

std::string AudioManager::GetLastErrorAsSwitch()
{

	switch (myLastError) {
	case OK:
		return "OK";
	case ERR_BADCOMMAND:
		return "ERR_BADCOMMAND";
		
	case ERR_CHANNEL_ALLOC:
		return "ERR_CHANNEL_ALLOC";
		
	case ERR_CHANNEL_STOLEN:
		return "ERR_CHANNEL_STOLEN";
		
	case ERR_DMA:
		return "ERR_DMA";
		
	case ERR_DSP_CONNECTION:
		return "ERR_DSP_CONNECTION";
		
	case ERR_DSP_DONTPROCESS:
		return "ERR_DSP_DONTPROCESS";
		
	case ERR_DSP_FORMAT:
		return "ERR_DSP_FORMAT";
		
	case ERR_DSP_INUSE:
		return "ERR_DSP_INUSE";
		
	case ERR_DSP_NOTFOUND:
		return "ERR_DSP_NOTFOUND";
		
	case ERR_DSP_RESERVED:
		return "ERR_DSP_RESERVED";
		
	case ERR_DSP_SILENCE:
		return "ERR_DSP_SILENCE";
		
	case ERR_DSP_TYPE:
		return "ERR_DSP_TYPE";
		
	case ERR_FILE_BAD:
		return "ERR_FILE_BAD";
		
	case ERR_FILE_COULDNOTSEEK:
		return "ERR_FILE_COULDNOTSEEK";
		
	case ERR_FILE_DISKEJECTED:
		return "ERR_FILE_DISKEJECTED";
		
	case ERR_FILE_EOF:
		return "ERR_FILE_EOF";
		
	case ERR_FILE_ENDOFDATA:
		return "ERR_FILE_ENDOFDATA";
		
	case ERR_FILE_NOTFOUND:
		return "ERR_FILE_NOTFOUND";
		
	case ERR_FORMAT:
		return "ERR_FORMAT";
		
	case ERR_HEADER_MISMATCH:
		return "ERR_HEADER_MISMATCH";
		
	case ERR_HTTP:
		return "ERR_HTTP";
		
	case ERR_HTTP_ACCESS:
		return "ERR_HTTP_ACCESS";
		
	case ERR_HTTP_PROXY_AUTH:
		return "ERR_HTTP_PROXY_AUTH";
		
	case ERR_HTTP_SERVER_ERROR:
		return "ERR_HTTP_SERVER_ERROR";
		
	case ERR_HTTP_TIMEOUT:
		return "ERR_HTTP_TIMEOUT";
		
	case ERR_INITIALIZATION:
		return "ERR_INITIALIZATION";
		
	case ERR_INITIALIZED:
		return "ERR_INITIALIZED";
		
	case ERR_INTERNAL:
		return "ERR_INTERNAL";
		
	case ERR_INVALID_FLOAT:
		return "ERR_INVALID_FLOAT";
		
	case ERR_INVALID_HANDLE:
		return "ERR_INVALID_HANDLE";
		
	case ERR_INVALID_PARAM:
		return "ERR_INVALID_PARAM";
		
	case ERR_INVALID_POSITION:
		return "ERR_INVALID_POSITION";
		
	case ERR_INVALID_SPEAKER:
		return "ERR_INVALID_SPEAKER";
		
	case ERR_INVALID_SYNCPOINT:
		return "ERR_INVALID_SYNCPOINT";
		
	case ERR_INVALID_THREAD:
		return "ERR_INVALID_THREAD";
		
	case ERR_INVALID_VECTOR:
		return "ERR_INVALID_VECTOR";
		
	case ERR_MAXAUDIBLE:
		return "ERR_MAXAUDIBLE";
		
	case ERR_MEMORY:
		return "ERR_MEMORY";
		
	case ERR_MEMORY_CANTPOINT:
		return "ERR_MEMORY_CANTPOINT";
		
	case ERR_NEEDS3D:
		return "ERR_NEEDS3D";
		
	case ERR_NEEDSHARDWARE:
		return "ERR_NEEDSHARDWARE";
		
	case ERR_NET_CONNECT:
		return "ERR_NET_CONNECT";
		
	case ERR_NET_SOCKET_ERROR:
		return "ERR_NET_SOCKET_ERROR";
		
	case ERR_NET_URL:
		return "ERR_NET_URL";
		
	case ERR_NET_WOULD_BLOCK:
		return "ERR_NET_WOULD_BLOCK";
		
	case ERR_NOTREADY:
		return "ERR_NOTREADY";
		
	case ERR_OUTPUT_ALLOCATED:
		return "ERR_OUTPUT_ALLOCATED";
		
	case ERR_OUTPUT_CREATEBUFFER:
		return "ERR_OUTPUT_CREATEBUFFER";
		
	case ERR_OUTPUT_DRIVERCALL:
		return "ERR_OUTPUT_DRIVERCALL";
		
	case ERR_OUTPUT_FORMAT:
		return "ERR_OUTPUT_FORMAT";
		
	case ERR_OUTPUT_INIT:
		return "ERR_OUTPUT_INIT";
		
	case ERR_OUTPUT_NODRIVERS:
		return "ERR_OUTPUT_NODRIVERS";
		
	case ERR_PLUGIN:
		return "ERR_PLUGIN";
		
	case ERR_PLUGIN_MISSING:
		return "ERR_PLUGIN_MISSING";
		
	case ERR_PLUGIN_RESOURCE:
		return "ERR_PLUGIN_RESOURCE";
		
	case ERR_PLUGIN_VERSION:
		return "ERR_PLUGIN_VERSION";
		
	case ERR_RECORD:
		return "ERR_RECORD";
		
	case ERR_REVERB_CHANNELGROUP:
		return "ERR_REVERB_CHANNELGROUP";
		
	case ERR_REVERB_INSTANCE:
		return "ERR_REVERB_INSTANCE";
		
	case ERR_SUBSOUNDS:
		return "ERR_SUBSOUNDS";
		
	case ERR_SUBSOUND_ALLOCATED:
		return "ERR_SUBSOUND_ALLOCATED";
		
	case ERR_SUBSOUND_CANTMOVE:
		return "ERR_SUBSOUND_CANTMOVE";
		
	case ERR_TAGNOTFOUND:
		return "ERR_TAGNOTFOUND";
		
	case ERR_TOOMANYCHANNELS:
		return "ERR_TOOMANYCHANNELS";
		
	case ERR_TRUNCATED:
		return "ERR_TRUNCATED";
		
	case ERR_UNIMPLEMENTED:
		return "ERR_UNIMPLEMENTED";
		
	case ERR_UNINITIALIZED:
		return "ERR_UNINITIALIZED";
		
	case ERR_UNSUPPORTED:
		return "ERR_UNSUPPORTED";
		
	case ERR_VERSION:
		return "ERR_VERSION";
		
	case ERR_EVENT_ALREADY_LOADED:
		return "ERR_EVENT_ALREADY_LOADED";
		
	case ERR_EVENT_LIVEUPDATE_BUSY:
		return "ERR_EVENT_LIVEUPDATE_BUSY";
		
	case ERR_EVENT_LIVEUPDATE_MISMATCH:
		return "ERR_EVENT_LIVEUPDATE_MISMATCH";
		
	case ERR_EVENT_LIVEUPDATE_TIMEOUT:
		return "ERR_EVENT_LIVEUPDATE_TIMEOUT";
		
	case ERR_EVENT_NOTFOUND:
		return "ERR_EVENT_NOTFOUND";
		
	case ERR_STUDIO_UNINITIALIZED:
		return "ERR_STUDIO_UNINITIALIZED";
		
	case ERR_STUDIO_NOT_LOADED:
		return "ERR_STUDIO_NOT_LOADED";
		
	case ERR_INVALID_STRING:
		return "ERR_INVALID_STRING";
		
	case ERR_ALREADY_LOCKED:
		return "ERR_ALREADY_LOCKED";
		
	case ERR_NOT_LOCKED:
		return "ERR_NOT_LOCKED";
		
	case ERR_RECORD_DISCONNECTED:
		return "ERR_RECORD_DISCONNECTED";
		
	case ERR_TOOMANYSAMPLES:
		return "ERR_TOOMANYSAMPLES";
		
	case SERESULT_FORCEINT:
		return "SERESULT_FORCEINT";
		
	default:
		break;
	}
}

int AudioManager::GetLastErrorAsInt()
{
	return 	static_cast<int>(myLastError);
}

std::vector<FMOD_GUID> AudioManager::GetEventGUIDs(const std::string& bankName)
{
	return myFNgin->GetEventGUIDs(bankName);
}

FMOD_GUID AudioManager::GetEventGUID(const std::string& eventPath)
{
	return myFNgin->GetEventGUID(eventPath);
}

int AudioManager::GetEventLength(const SoundEventInstanceHandle& anEventInstanceHandle)
{
	FMOD::Studio::EventDescription* eventDescription = nullptr;
	FMOD::Studio::EventInstance* eventInstance = anEventInstanceHandle.GetEventInstance();


	if (eventInstance->getDescription(&eventDescription) != FMOD_OK)
	{
		// Handle error
		LOGERROR("GetEventLength: !FMOD_OK");
		return -1;
	}

	int length = 0;
	if (eventDescription->getLength(&length) != FMOD_OK)
	{
		// Handle error
		LOGERROR("GetEventLength: !FMOD_OK");
		return -1;
	}

	return length;
}

std::string AudioManager::GetEventPath(const FMOD_GUID& eventGUID)
{
	auto it = std::find_if(myEventGUIDs.begin(), myEventGUIDs.end(), [&](const FMOD_GUID& guid) {
		return memcmp(&eventGUID, &guid, sizeof(FMOD_GUID)) == 0;
		});

	if (it != myEventGUIDs.end())
	{
		size_t index = std::distance(myEventGUIDs.begin(), it);
		index += 1; // The first GUID is the master bank GUID, which we don't want to return
		return myEventStrings[index];
	}
	else
	{
		LOGERROR("GUID not found in AudioManager");
		return "";
	}
}



std::vector<std::string> AudioManager::ConvertGUIDsToStrings(const std::vector<FMOD_GUID>& guidVector)
{
	std::vector<std::string> stringVector;
	stringVector.reserve(guidVector.size());

	for (const auto& guid : guidVector)
	{
		char guidString[64];
		int numChars = snprintf(guidString, sizeof(guidString), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

		if (numChars > 0 && numChars < sizeof(guidString))
		{
			stringVector.emplace_back(guidString);
		}
		else
		{
			LOGERROR("Failed to format GUID string");
		}
	}

	return stringVector;
}

std::vector<std::string> AudioManager::ConvertGUIDsToEventNames(const std::vector<FMOD_GUID>& guidVector)
{
	std::vector<std::string> eventNames;
	eventNames.reserve(guidVector.size());

	for (const auto& guid : guidVector)
	{
		auto it = std::find_if(myEventGUIDs.begin(), myEventGUIDs.end(), [&](const FMOD_GUID& eventGUID) {
			return memcmp(&guid, &eventGUID, sizeof(FMOD_GUID)) == 0;
			});

		if (it != myEventGUIDs.end())
		{
			size_t index = std::distance(myEventGUIDs.begin(), it);
			eventNames.emplace_back(myEventStrings[index]);
		}
		else
		{
			LOGERROR("GUID not found in AudioManager");
		}
	}

	return eventNames;
}

std::string AudioManager::ConvertGUIDToEventName(const FMOD_GUID& guid)
{
	auto it = std::find_if(myEventGUIDs.begin(), myEventGUIDs.end(), [&](const FMOD_GUID& eventGUID) {
		return memcmp(&guid, &eventGUID, sizeof(FMOD_GUID)) == 0;
		});

	if (it != myEventGUIDs.end())
	{
		size_t index = std::distance(myEventGUIDs.begin(), it);
		return myEventStrings[index];
	}
	else
	{
		LOGERROR("GUID not found in AudioManager");
		return "";
	}
}

std::string AudioManager::ConvertGUIDToString(const FMOD_GUID& guid)
{
	char guidString[64];
	int numChars = snprintf(guidString, sizeof(guidString), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	if (numChars > 0 && numChars < sizeof(guidString))
	{
		return std::string(guidString);
	}
	else
	{
		LOGERROR("Failed to format GUID string");
		return "";
	}
}

bool AudioManager::GetEventTimelinePosition(const SoundEventInstanceHandle& aHandle, int* position)
{
	if (!myFNgin)
	{
				return false;
	}
	auto eventInstance = aHandle.GetEventInstance();
	if (!eventInstance)
	{
				return false;
	}
	FMOD_RESULT result = eventInstance->getTimelinePosition(position);
	if (result != FMOD_OK)
	{
				return false;
	}
	return true;
}


//
//bool AudioManager::GetEventProperties(const SoundEventInstanceHandle& aHandle, FMOD_STUDIO_EVENT_PROPERTIES* aProperties)
//{
//	if (!myFNgin)
//	{
//		return false;
//	}
//
//	memset(aProperties, 0, sizeof(FMOD_STUDIO_EVENT_PROPERTIES));
//	aProperties->cbsize = sizeof(FMOD_STUDIO_EVENT_PROPERTIES);
//
//	auto eventInstance = myFNgin->GetEventInstance(aHandle);
//	if (!eventInstance)
//	{
//		return false;
//	}
//
//	FMOD_RESULT result = eventInstance->getProperty(FMOD_STUDIO_EVENT_PROPERTY_TIMELINE_POSITION, &(aProperties->position));
//	if (result != FMOD_OK)
//	{
//		return false;
//	}
//
//	result = eventInstance->getProperty(FMOD_STUDIO_EVENT_PROPERTY_TIMELINE_LENGTH, &(aProperties->length));
//	if (result != FMOD_OK)
//	{
//		return false;
//	}
//
//	result = eventInstance->getProperty(FMOD_STUDIO_EVENT_PROPERTY_IS_PLAYING, &(aProperties->isPlaying));
//	if (result != FMOD_OK)
//	{
//		return false;
//	}
//
//	// Add more properties as needed
//
//	return true;
//}
//
