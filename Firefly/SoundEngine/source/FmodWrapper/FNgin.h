#pragma once
#include "FNginStructs.h"
#include "core/inc/fmod.hpp"
#include "studio/inc/fmod_studio.hpp"
#include <assert.h>
#include <array>
#include <string>
#include <memory>

class FNgin
{
	static inline std::unordered_map<unsigned int, std::string> myEventAccelMap{};
	static inline std::unordered_map<std::string, FMOD_GUID> myEventPathToGUID;
public:
	struct ListenerHandle;

	FNgin();
	~FNgin();

	bool							Initialization(const std::string& aRootPath, unsigned int maxChannels = 64);
	void							Update();
	bool							LoadBank(const std::string& aFileName, FMOD_STUDIO_LOAD_BANK_FLAGS someFlags);
	bool							LoadEvent(const std::string& anEventName);

	//Not Working atm
	unsigned						GetChannels(std::vector<std::string>& outChannelList);
	//Not Working atm
	unsigned						GetBus(std::vector<std::string>& outBusList);

	std::vector<FMOD_GUID>			GetEventGUIDs(const std::string& bankName);
	FMOD_GUID						GetEventGUID(const std::string& eventPath);

	unsigned						GetEvents(std::vector<std::string>& outEventList);
	bool							RegisterEvent(const std::string& anEventName, unsigned int anEventId);
	bool							ReleaseEvent(const SoundEventInstanceHandle& aHandle);
	bool							ReleaseEvent(const std::string& anEventInstanceString);


	SoundEventInstanceHandle		CreateEventInstance(const std::string& anEventName);
	FMOD::Studio::EventInstance*	CreateEventInstanceFmod(const std::string& anEventName);
	bool							SetEvent3DParameters(const SoundEventInstanceHandle& aHandle, std::array<float, 3> aPosition = { 0, 0, 0 }, std::array<float, 3> aVelocity = { 0, 0, 0 }, std::array<float, 3> aForwardVector = { 0, 0, 1 }, std::array<float, 3> aUpVector = { 0, 1, 0 });
	bool							PlayEvent(const SoundEventInstanceHandle& anEventInstanceHandle);
	bool							PlayEventOneShot(const std::string& anEventName);
	bool							PlayEventOneShotWithParameters(const std::string& anEventName, std::array<float, 3> aPosition = { 0, 0, 0 }, std::array<float, 3> aVelocity = { 0, 0, 0 }, std::array<float, 3> aForwardVector = { 0, 0, 1 }, std::array<float, 3> aUpVector = { 0, 1, 0 }, int* aLength = 0);
	bool							StopEvent(const SoundEventInstanceHandle& aHandle, bool immediately);
	FMOD_STUDIO_PLAYBACK_STATE		GetPlaybackState(const SoundEventInstanceHandle& aHandle);

	int								GetNumListeners();
	bool							AddListener(const std::array<float, 3> aPosition = { 0, 0, 0 }, const std::array<float, 3> aVelocity = { 0, 0, 0 }, const std::array<float, 3> aForwardVector = { 0, 0, 1 }, const std::array<float, 3> aUpVector = { 0, 1, 0 });
	bool							RemoveListener(int anIndex);
	bool							SetListenerAttributes(int listener, std::array<float, 3> aPosition = { 0,0,0 }, std::array<float, 3> aVelocity = { 0,0,0 }, std::array<float, 3> aForwardVector = { 0,0,1 }, std::array<float, 3> aUpVector = { 0,1,0 }, const FMOD_VECTOR* attenuationposition = nullptr);

	bool							GetBusPaused(const std::string& aBusName, bool* aOnOff);
	bool							SetBusPaused(const std::string& aBusName, bool aOnOff);

	bool							GetBusVolume(const std::string& aBusName, float* aVolumePct);
	bool							SetBusVolume(const std::string& aBusName, float aVolumePct);
	bool							SetChannelVolume(unsigned int aChannel, float aVolumePct);
	bool							SetSoundEventVolume(const SoundEventInstanceHandle& aHandle, float aVolumePct);

	FMOD::ChannelGroup*				GetMasterChannelGroup();

	bool							GetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float* outValue);
	bool							SetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float aValue);
	bool							SetGlobalParameter(const std::string& aParameterName, float aValue, bool ignoreSeekSpeed = false);
	bool							GetGlobalParameter(const std::string& aParameterName, float* outValue);

	bool							StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode);

	bool							Get3DMinMaxDistance(const std::string& anEventName, float* aMinDistance, float* aMaxDistance);

	bool							GetEventTimelinePosition(const SoundEventInstanceHandle& aHandle, int* position);





	void SetMasterVolume(float aVolume);
	void SetSFXVolume(float aVolume);
	void SetMusicVolume(float aVolume);
	void SetAmbientVolume(float aVolume);

	[[nodiscard]] float GetMasterVolume() const { return myMasterVolume; }
	[[nodiscard]] float GetSFXVolume() const { return mySFXVolume; }
	[[nodiscard]] float GetMusicVolume() const { return myMusicVolume; }
	[[nodiscard]] float GetAmbientVolume() const { return myAmbientVolume; }

	FMOD_RESULT GetLastError()const { return myLastError; }

public:

	//struct ListenerHandle;

	// Represents a specific listener
	struct ListenerHandle
	{
		friend struct Listener;
	private:
		int myId = -1;
	public:
		//int GetId() const;
		//bool IsValid() const;
	};

private:

	// Defines a spatial listener at a relative position.
	struct Listener
	{
		int ListenerId = 0;
		int FmodId = -1;
		std::array<float, 3> Location{ 0, 0, 0 };
		std::array<float, 3> Velocity{ 0, 0, 0 };
		std::array<float, 3> Forward{ 0, 0, 1 };
		std::array<float, 3> Up{ 0, 1, 0 };

		struct ListenerHandle Handle() const;
	};

	static inline std::unordered_map<int, Listener> myListeners{};
	static inline int nextListenerId = 0;


	struct Event
	{
		std::string Path;
		bool IsLoaded = false;
		FMOD::Studio::EventDescription* FmodEventDesc = nullptr;
		int NextInstanceId = 0;
		int NextCallbackId = 0;
		std::unordered_map<unsigned int, FMOD::Studio::EventInstance*> FmodEventInstances;
		std::unordered_map<unsigned int, std::shared_ptr<EventCallbackBase>> Callbacks;
	};

	typedef std::unordered_map<unsigned int, FMOD::Channel*> ChannelMap;
	typedef std::unordered_map<std::string, FMOD::Sound*> SoundMap;
	typedef std::unordered_map<std::string, Event> EventMap;
	typedef std::unordered_map<std::string, FMOD::Studio::Bank*> BankMap;
	typedef std::unordered_map<std::string, FMOD::Studio::Bus*> BusMap;

	ChannelMap myChannels;
	SoundMap mySounds;
	EventMap myEvents;
	BankMap myBanks;
	BusMap myBus;
	FMOD::ChannelGroup* myMastergroup = 0;

	std::string myRootDirectory;

	FMOD::Studio::System* myFModStudioPtr = nullptr;
	FMOD::System* myFModPtr = nullptr;

	unsigned int myNextChId = 0;

	bool isInitialized = false;

	FMOD_RESULT myLastError = FMOD_OK;

	float myMasterVolume;
	float mySFXVolume;
	float myMusicVolume;
	float myAmbientVolume;

};

