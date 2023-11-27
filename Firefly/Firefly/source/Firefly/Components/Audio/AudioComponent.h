#pragma once
#include "Firefly/ComponentSystem/Component.h"
#include "Firefly/Core/Core.h"
#include "FmodWrapper/AudioManager.h"
#include "Firefly/Event/EditorEvents.h"

namespace Firefly
{

	class EntityPropertyUpdatedEvent;
	class AppUpdateEvent;

	class AudioComponent : public Firefly::Component
	{

		struct SoundAsset
		{
			std::string myEventPath;
			FMOD_GUID myEventGUID;
			size_t myId = -1;
		};

	public:
		AudioComponent();
		~AudioComponent();
		void Initialize() override;
		void OnEvent(Firefly::Event& aEvent) override;
		void PlayEvent();
		void PlayEventOneShot();
		void StopEvent(bool immediately = true);
		void SetEvent3DParameters(Utils::Vector3f aPosition);
		bool StopEventsOnBus(const std::string& aBusName, FMOD_STUDIO_STOP_MODE aMode);

		static std::string GetFactoryName() { return "AudioComponent"; }
		static Ref<Firefly::Component> Create() { return CreateRef<AudioComponent>(); }

		void DrawDebugMinMax();
		void StartStop();

		std::vector<FMOD_GUID>			GetEventGUIDs(const std::string& bankName);
		FMOD_GUID						GetEventGUID(const std::string& eventPath);

		void Apply();

	private:
		bool myPlayEvent = false;
		bool myPlayEventOneShot = false;

		float myVolume = 0.f;
		bool myPlaySoundEvent = false;
		bool myUseSoundList = false;
		bool myUseDebugView = false;
		bool myUseMultiple = false;
		float myMin = 0.f;
		float myMax= 0.f;
		
		bool myStoped = true;
		std::vector<std::string> mySoundEventList;
		std::vector<SoundAsset*> mySoundAssetList;

		int myAmountOfSounds;


		std::vector<FMOD_GUID> myEventGUIDs;
		std::string myEventPath;
		uint32_t myIndex;

		SoundEventInstanceHandle mySoundEventInstanceHandle;
		std::vector<SoundEventInstanceHandle> mySoundEventInstanceHandleList;


		bool OnPropertyUpdated(EntityPropertyUpdatedEvent& aEvent);
		bool OnUpdate(AppUpdateEvent& aUpdateEvent);
		bool OnStopEvent(EditorStopEvent& aEditorStopEvent);

		std::string myMeshPath;
	};
}