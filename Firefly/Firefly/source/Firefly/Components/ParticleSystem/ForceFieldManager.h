#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Rendering/ParticleSystem/EmitterSettings.h"

struct ForceField;

namespace Firefly
{
	class Entity;

	class ForceFieldManager
	{
	public:
		~ForceFieldManager();

		static ForceFieldManager& Get();

		void AddForceField(const Ptr<Entity>& aForceFieldEntity);
		void AddForceField(uint64_t aForceFieldEntityID);
		void UpdateForceFields();
		void ClearForceFields();

		const std::vector<ForceField>& GetForceFields();

	private:
		static inline ForceFieldManager* myInstance = nullptr;

		std::vector<Ptr<Entity>> myForceFieldEntities;
		std::vector<ForceField> myForceFields;
	};
}