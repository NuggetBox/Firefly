#pragma once
#include "PhysX/PxPhysicsAPI.h"
#include "Firefly/Physics/PhysicsCallbacks.h"
#include <Firefly/Physics/PhysicsBakery.h>
namespace Firefly
{
	namespace DebugMode
	{
		enum Mode
		{
			All,
			Debug,
			Profile,
			Memory,
		};
	};

	class PhysicsImplementation
	{
		friend class PhysicsScene;
		friend class PhysicsBakery;
	public:
		static void Initialize();
		static void AddActorCallback(std::function<void()> aFunc);
		static void FlushActorCallbacks();
		static void ClearActorCallbacks();
		static void ConnectToDebugger(DebugMode::Mode aMode = DebugMode::All);
		static void DisconnectDebugger();
		static void ShutDown();
		static PhysicsBakery& GetBakery();
		static PhysicQueryFilterCallback& GetQueryCallback();
		static physx::PxPhysics* GetPhysics() { return myPhysics; }
	private:
		static inline physx::PxFoundation* myFoundation;
		static inline physx::PxPhysics* myPhysics;
		static inline physx::PxDefaultCpuDispatcher* myCpuDispatcher;
		static inline physx::PxPvd* myPvd;
		static inline physx::PxPvdTransport* myTransport;
		static inline PhysicsBakery myBakery;
		static inline PhysicsAllocatorCallback myAllocator;
		static inline PhysicsErrorCallback myErrorHandler;
		static inline PhysicQueryFilterCallback myFilterCallback;
		static inline std::vector<std::function<void()>> myActorCallbackFunction;
	};
}