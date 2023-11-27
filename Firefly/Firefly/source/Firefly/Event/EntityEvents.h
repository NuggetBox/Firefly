#pragma once
#include "Event.h"
#include "Firefly/ComponentSystem/Component.h"

namespace Firefly
{
	class EntityPropertyUpdatedEvent : public Firefly::Event
	{
	public:
		EntityPropertyUpdatedEvent(const std::string& aParamName, ParameterType aParamType)
		{
			myParamName = aParamName;
			myParamType = aParamType;
		}
		~EntityPropertyUpdatedEvent() {}

		const std::string& GetParamName() const { return myParamName; }
		ParameterType GetParamType() const { return myParamType; }

		EVENT_CLASS_TYPE(EntityPropertyUpdated)
	private:
		std::string myParamName;
		ParameterType myParamType;
	};

	class EntityOnCollisionEnterEvent : public Firefly::Event
	{
	public:
		EntityOnCollisionEnterEvent(Ptr<Entity> aCollidedEntity, 
			const Utils::Vector3f& aContactPoint = {}, const Utils::Vector3f& aImpulse = {}, const Utils::Vector3f& aNormal = {})
		{
			myCollidedEntity = aCollidedEntity;
			myContactPoint = aContactPoint;
			myImpulse = aImpulse;
			myNormal = aNormal;
		}
		~EntityOnCollisionEnterEvent() {}

		Ptr<Entity> GetCollidedEntity() { return myCollidedEntity; }
		const Utils::Vector3f& GetContactPoint() const { return myContactPoint; }
		const Utils::Vector3f& GetImpulse() const { return myImpulse; }
		const Utils::Vector3f& GetNormal() const { return myNormal; }

		EVENT_CLASS_TYPE(EntityOnCollisionEnter)

	private:
		Ptr<Entity> myCollidedEntity;
		Utils::Vector3f myContactPoint;
		Utils::Vector3f myImpulse;
		Utils::Vector3f myNormal;
	};

	class EntityOnTriggerEnterEvent : public Firefly::Event
	{
	public:
		EntityOnTriggerEnterEvent(Ptr<Entity> aCollidedEntity)
		{
			myCollidedEntity = aCollidedEntity;
		}
		~EntityOnTriggerEnterEvent() {}

		Ptr<Entity> GetCollidedEntity() { return myCollidedEntity; }

		EVENT_CLASS_TYPE(EntityOnTriggerEnter)
	private:
		Ptr<Entity> myCollidedEntity;
	};

	class EntityOnCollisionExitEvent : public Firefly::Event
	{
	public:
		EntityOnCollisionExitEvent(Ptr<Entity> aCollidedEntity)
		{
			myCollidedEntity = aCollidedEntity;
		}
		~EntityOnCollisionExitEvent() {}


		Ptr<Entity> GetCollidedEntity() { return myCollidedEntity; }

		EVENT_CLASS_TYPE(EntityOnCollisionExit)
	private:
		Ptr<Entity> myCollidedEntity;
	};

	class EntityOnTriggerExitEvent : public Firefly::Event
	{
	public:
		EntityOnTriggerExitEvent(Ptr<Entity> aCollidedEntity)
		{
			myCollidedEntity = aCollidedEntity;
		}
		~EntityOnTriggerExitEvent() {}


		Ptr<Entity> GetCollidedEntity() { return myCollidedEntity; }

		EVENT_CLASS_TYPE(EntityOnTriggerExit)
	private:
		Ptr<Entity> myCollidedEntity;
	};

	class EntityOnComponentEnableEvent : public Firefly::Event
	{
	public:
		EntityOnComponentEnableEvent(const bool& aStatus)
		{
			myStatus = aStatus;
		}
		~EntityOnComponentEnableEvent() {}

		bool GetStatus() { return myStatus; }
		

		EVENT_CLASS_TYPE(EntityOnComponentEnable)
	private:
		bool myStatus;
	};

	class EntityOnEnableEvent : public Firefly::Event
	{
	public:
		EntityOnEnableEvent(const bool& aStatus)
		{
			myStatus = aStatus;
		}
		~EntityOnEnableEvent() {}

		bool GetStatus() { return myStatus; }



		EVENT_CLASS_TYPE(EntityOnEnable)
	private:
		bool myStatus;
	};
}