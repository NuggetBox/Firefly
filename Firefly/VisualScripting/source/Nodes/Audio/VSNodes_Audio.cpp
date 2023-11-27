#include "VSNodes_Audio.h"

#include "FmodWrapper/AudioManager.h"

void VSNode_Audio_PlayEvent::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<SoundEventInstanceHandle>("InSoundEvent", PinDirection::Input);
	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);

}

size_t VSNode_Audio_PlayEvent::DoOperation()
{
	SoundEventInstanceHandle value;
	if (GetPinData("InSoundEvent", value))
	{
		const bool returnValue = AudioManager::PlayEvent(value);
		std::string returnFmodError = AudioManager::GetLastErrorAsSwitch();

		SetPinData<bool>("OutValue", returnValue);
		SetPinData<std::string>("OutError", returnFmodError);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_CreateEventInstance::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<std::string>("InSoundString", PinDirection::Input);
	CreateDataPin<SoundEventInstanceHandle>("OutSoundEvent", PinDirection::Output);

	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);
}

size_t VSNode_Audio_CreateEventInstance::DoOperation()
{

	std::string soundString;
	if (GetPinData("InSoundString", soundString))
	{
		bool returnValue = false;

		const SoundEventInstanceHandle returnSoundEvent = AudioManager::CreateEventInstance(soundString);
		std::string returnFMODError = AudioManager::GetLastErrorAsSwitch();
		//std::cout << returnFMODError << std::endl;  // Print the value to the console

		if (returnFMODError == "OK") returnValue = true;

		SetPinData<SoundEventInstanceHandle>("OutSoundEvent", returnSoundEvent);
		SetPinData<bool>("OutValue", returnValue);
		SetPinData<std::string>("OutError", returnFMODError);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_GetMinMaxDistance::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<float>("Value", PinDirection::Input);
	CreateDataPin<float>("OutValue", PinDirection::Output);
}

size_t VSNode_Audio_GetMinMaxDistance::DoOperation()
{
	float value;
	if (GetPinData("Value", value))
	{
		const float returnValue = value * value / value * sinf(value) * 2 * 3;
		SetPinData("OutValue", returnValue);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_GetPlaybackState::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<float>("Value", PinDirection::Input);
	CreateDataPin<float>("OutValue", PinDirection::Output);
}

size_t VSNode_Audio_GetPlaybackState::DoOperation()
{
	float value;
	if (GetPinData("Value", value))
	{
		const float returnValue = value * value / value * sinf(value) * 2 * 3;
		SetPinData("OutValue", returnValue);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_PlayEventOneShot::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<std::string>("InSoundEvent", PinDirection::Input);
	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);
}

size_t VSNode_Audio_PlayEventOneShot::DoOperation()
{
	std::string value;
	bool returnValue;
	std::string returnFmodError;
	if (GetPinData("InSoundEvent", value))
	{
		returnValue = AudioManager::PlayEventOneShot(value);
		returnFmodError = AudioManager::GetLastErrorAsSwitch();

		SetPinData<bool>("OutValue", returnValue);
		SetPinData<std::string>("OutError", returnFmodError);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_PlayEventOneShotWithParameters::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<std::string>("InSoundEvent", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("InPos", PinDirection::Input);
	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);
}

size_t VSNode_Audio_PlayEventOneShotWithParameters::DoOperation()
{
	std::string value;
	bool returnValue;
	std::string returnFmodError;
	if (GetPinData("InSoundEvent", value))
	{

		Utils::Vector3f Posvalue;
		if (GetPinData("InPos", Posvalue))
		{
			returnValue = AudioManager::PlayEventOneShotWithParameters(value, { Posvalue.x,Posvalue.y,Posvalue.z});
			returnFmodError = AudioManager::GetLastErrorAsSwitch();

			SetPinData<bool>("OutValue", returnValue);
			SetPinData<std::string>("OutError", returnFmodError);

			return ExitViaPin("Out");
		}

	}

	return 0;
}

void VSNode_Audio_SetEvent3DParameters::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<SoundEventInstanceHandle>("InSoundEvent", PinDirection::Input);
	CreateDataPin<Utils::Vector3f>("InPos", PinDirection::Input);
	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);
}

size_t VSNode_Audio_SetEvent3DParameters::DoOperation()
{
	SoundEventInstanceHandle value;
	if (GetPinData("InSoundEvent", value))
	{
		Utils::Vector3f Posvalue;
		if (GetPinData("InPos", Posvalue))
		{
			const bool returnValue = AudioManager::SetEvent3DParameters(value, Posvalue);
			std::string returnFmodError = AudioManager::GetLastErrorAsSwitch();

			SetPinData<bool>("OutValue", returnValue);
			SetPinData<std::string>("OutError", returnFmodError);

			return ExitViaPin("Out");
		}
	}
	return 0;
}

void VSNode_Audio_SetGlobalParameter::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<std::string>("ParameterName", PinDirection::Input);
	CreateDataPin<float>("Value", PinDirection::Input);
	CreateDataPin<bool>("IgnoreSeekSpeed", PinDirection::Input);

}

size_t VSNode_Audio_SetGlobalParameter::DoOperation()
{
	std::string parameterName = "";
	if (GetPinData("ParameterName", parameterName))
	{
		float value;
		if (GetPinData("Value", value))
		{
			bool ignoreSeekSpeed;
			GetPinData("IgnoreSeekSpeed", ignoreSeekSpeed);

			AudioManager::SetGlobalParameter(parameterName, value, ignoreSeekSpeed);

			return ExitViaPin("Out");
		}
	}


	return 0;
}

void VSNode_Audio_SetListenerAttributes::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<float>("Value", PinDirection::Input);
	CreateDataPin<float>("OutValue", PinDirection::Output);
}

size_t VSNode_Audio_SetListenerAttributes::DoOperation()
{
	float value;
	if (GetPinData("Value", value))
	{
		const float returnValue = value * value / value * sinf(value) * 2 * 3;
		SetPinData("OutValue", returnValue);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_Audio_StopEvent::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<SoundEventInstanceHandle>("InSoundEvent", PinDirection::Input);
	CreateDataPin<bool>("OutValue", PinDirection::Output);
	CreateDataPin<std::string>("OutError", PinDirection::Output);
}

size_t VSNode_Audio_StopEvent::DoOperation()
{
	SoundEventInstanceHandle value;
	bool returnValue;
	std::string returnFmodError;
	if (GetPinData("InSoundEvent", value))
	{
		returnValue = AudioManager::StopEvent(value);
		returnFmodError = AudioManager::GetLastErrorAsSwitch();

		SetPinData<bool>("OutValue", returnValue);
		SetPinData<std::string>("OutError", returnFmodError);

		return ExitViaPin("Out");
	}

	return 0;
}

void VSNode_AudioManagerInit::Init()
{
	CreateExecPin("In", PinDirection::Input, true);
	CreateExecPin("Out", PinDirection::Output, true);

	CreateDataPin<bool>("OutValue", PinDirection::Output);
}

size_t VSNode_AudioManagerInit::DoOperation()
{
	SetPinData("OutValue", AudioManager::Initialization());
	return ExitViaPin("Out");
}