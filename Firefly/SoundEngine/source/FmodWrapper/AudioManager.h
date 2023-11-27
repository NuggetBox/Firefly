#pragma once
#include "FmodPathDefines.h"
#include "FNgin.h"
#include "FNginStructs.h"
#include "Utils/Math/Vector.h"

#include <studio/inc/fmod_studio.hpp>

class AudioManager
{
public:
	AudioManager();
	~AudioManager() = default;


	void AudioTimerTest();
	static void OnTimer();

	static bool Initialization();
	static void Update();

	static bool PlayEvent(const SoundEventInstanceHandle& anEventInstanceHandle);
	static bool PlayEventOneShot(const std::string& aEventPathDefine);
	static bool PlayEventOneShotWithParameters(const std::string& aEventPathDefine, std::array<float, 3> aPosition = { 0, 0, 0 }, std::array<float, 3> aVelocity = { 0, 0, 0 }, std::array<float, 3> aForwardVector = { 0, 0, 1 }, std::array<float, 3> aUpVector = { 0, 1, 0 }, int* aLength = 0);
	static bool StopEvent(const SoundEventInstanceHandle& anEventInstanceHandle, bool immediately = true);
	static SoundEventInstanceHandle CreateEventInstance(const std::string& aEventPathDefine);
	static SoundEventInstanceHandle CreateEventInstanceFmod(const std::string& aEventPathDefine);
	static bool SetEvent3DParameters(const SoundEventInstanceHandle& anEventInstanceHandle, Utils::Vector3f aPosition = { 0,0,0 }, Utils::Vector3f aVelocity = { 0,0,0 }, Utils::Vector3f aForwardVector = { 0,0,1 }, Utils::Vector3f aUpVector = { 0,1,0 });
	static bool SetEvent3DPosition(const SoundEventInstanceHandle& anEventInstanceHandle, Utils::Vector3f aPosition);

	static bool SetListenerAttributes(int aHandle, Utils::Vector3f aPosition = { 0,0,0 }, Utils::Vector3f aVelocity = { 0,0,0 }, Utils::Vector3f aForwardVector = { 0,0,1 }, Utils::Vector3f aUpVector = { 0,1,0 });
	static void SetListenerPosition(Utils::Vector3f aListenerPosition) { myListenerPosition = aListenerPosition; }
	static Utils::Vector3f GetListenerPosition() { return myListenerPosition; }
	static bool	GetBusVolume(const std::string& aBusName, float* aVolumePct);
	static bool	SetBusVolume(const std::string& aBusName, float aVolumePct);

	static bool	GetBusPaused(const std::string& aBusName, bool* aOnOff);
	static bool	SetBusPaused(const std::string& aBusName, bool aOnOff);

	static bool StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode);
	static float* GetBusVolumePTR(const std::string& aBusName);

	static bool	SetChannelVolume(unsigned int aChannel, float aVolumePct);
	static bool	SetEventVolume(const SoundEventInstanceHandle& aHandle, float aVolumePct);
	static bool	GetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float* outValue);
	static bool	SetEventParameter(const SoundEventInstanceHandle& aHandle, const std::string& aParameterName, float aValue);
	static bool	SetGlobalParameter(const std::string& aParameterName, float aValue, bool ignoreSeekSpeed = false);
	static bool	GetGlobalParameter(const std::string& aParameterName, float* outValue);

	static bool StartStop(bool aOn);

	static const std::vector<std::string>&	GetEvents();

	static float& GetSFXVolume() { return mySFXVolume; }
	static float& GetCurrentSFXVolume() { return myCurrentSFXVolume; }
	static void SetSFXVolume(float aVolume) { mySFXVolume = aVolume; }
	//static void SetCurrentSFXVolume(float aVolume) { myCurrentSFXVolume = aVolume; }
	static float& GetMaxSFXVolume() { return myMaxSFXVolume; }
	static void SetMaxSFXVolume(float aVolume) { myMaxSFXVolume = aVolume; }

	static float& GetMusicVolume() { return myMusicVolume; }
	static float& GetCurrentMusicVolume() { return myCurrentMusicVolume; }
	static void SetMusicVolume(float aVolume) { myMusicVolume = aVolume; }
	//static void SetCurrentMusicVolume(float aVolume) { myCurrentMusicVolume = aVolume; }
	static float& GetMaxMusicVolume() { return myMaxMusicVolume; }
	static void SetMaxMusicVolume(float aVolume) { myMaxMusicVolume = aVolume; }

	static bool Get3DMinMaxDistance(const std::string& anEventName, float* aMinDistance, float* aMaxDistance);

	static FMOD_STUDIO_PLAYBACK_STATE GetPlaybackState(const SoundEventInstanceHandle& aHandle);
	static bool GetIsPlaying(const SoundEventInstanceHandle& aHandle);
	static bool ReleaseEvent(const SoundEventInstanceHandle& anEventInstanceHandle);
	static bool ReleaseEvent(const std::string& anEventInstanceString);

	static FMOD::ChannelGroup* GetMasterChannelGroup();

	static SERESULT GetLastError();
	static std::string GetLastErrorAsSwitch();
	static int GetLastErrorAsInt();

	static std::vector<FMOD_GUID>			GetEventGUIDs(const std::string& bankName);
	static FMOD_GUID						GetEventGUID(const std::string& eventPath);


	static int GetEventLength(const SoundEventInstanceHandle& anEventInstanceHandle);

	static std::string GetEventPath(const FMOD_GUID& eventGUID);
	static std::vector<std::string> ConvertGUIDsToStrings(const std::vector<FMOD_GUID>& guidVector);
	static std::vector<std::string> ConvertGUIDsToEventNames(const std::vector<FMOD_GUID>& guidVector);
	static std::string ConvertGUIDToEventName(const FMOD_GUID& guid);
	static std::string ConvertGUIDToString(const FMOD_GUID& guid);

	static bool	GetEventTimelinePosition(const SoundEventInstanceHandle& aHandle, int* position);

private:
	friend class FNgin;

	inline static int aNumListeners;
	inline static Utils::Vector3f myListenerPosition;
	inline static FNgin* myFNgin;
	inline static std::vector<std::string> myEventStrings;
	inline static std::vector<FMOD_GUID> myEventGUIDs;

	inline static std::vector<std::string> mySoundEffects;
	inline static std::vector<std::string> mySoundEffectTypes;

	inline static bool isOn = false;

	inline static float mySFXVolume;
	inline static float myCurrentSFXVolume;
	inline static float myMaxSFXVolume;
	//inline static float myCurrentSFXVolume;
	inline static float myMusicVolume;
	inline static float myCurrentMusicVolume;
	inline static float myMaxMusicVolume;
	//inline static float myCurrentMusicVolume;
	//inline static float myMax;
	//inline static float myMin;
	//inline static float myMinMaxDistance;

	inline static bool myHasPlayed = false;

	inline static SERESULT myLastError = OK;
};