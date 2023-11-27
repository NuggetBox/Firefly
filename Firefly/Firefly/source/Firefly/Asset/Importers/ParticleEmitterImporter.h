#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/ParticleSystem/ParticleEmitter.h"

namespace Firefly
{
	class ParticleEmitterImporter
	{
	public:
		bool ImportEmitterTemplate(Ref<ParticleEmitterTemplate> aAsset);
	};
}
