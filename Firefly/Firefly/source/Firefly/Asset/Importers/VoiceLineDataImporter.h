#pragma once
#include "Firefly/Core/Core.h"



namespace Firefly
{
	class VoiceLineData;
	class VoiceLineDataImporter
	{
	public:
		VoiceLineDataImporter() = default;
		~VoiceLineDataImporter() = default;

		bool ImportVoiceLineData(Ref<VoiceLineData> aVoiceLineData);

	private:

	};
}