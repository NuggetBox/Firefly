#include "FFpch.h"
#include "PhysicsImplementation.h"
#include "PhysX/task/PxCpuDispatcher.h"
#include "PhysX/extensions/PxDefaultCpuDispatcher.h"
#include "PhysX/pvd/PxPvd.h"
#include <Firefly/Core/Log/DebugLogger.h>
#include <Firefly/Physics/PhysicsUtils.h>
#include "Firefly/ComponentSystem/SceneManager.h"

#define NUM_THREADS 0

namespace Firefly
{
	void PhysicsImplementation::Initialize()
	{
		myFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, myAllocator, myErrorHandler);
		if (!myFoundation)
		{
			LOGERROR("PhysX could not create foundation!");
			assert(false);
		}

		myPvd = PxCreatePvd(*myFoundation);
		myTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		myPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, physx::PxTolerancesScale(100.f, 1000.f), true, myPvd);
		if (!myPhysics)
		{
			LOGERROR("PhysX could not create Physics");
		}
		myBakery.Initialize();
		auto cp = physx::PxDefaultCpuDispatcherCreate(NUM_THREADS);
		myCpuDispatcher = cp;

	}

	void PhysicsImplementation::AddActorCallback(std::function<void()> aFunc)
	{
		myActorCallbackFunction.emplace_back(aFunc);
	}
	void PhysicsImplementation::FlushActorCallbacks()
	{
		for (int32_t i = myActorCallbackFunction.size() - 1; i >= 0; --i)
		{
			myActorCallbackFunction[i]();
		}
		myActorCallbackFunction.clear();
	}
	void PhysicsImplementation::ClearActorCallbacks()
	{
		myActorCallbackFunction.clear();
	}
	void PhysicsImplementation::ConnectToDebugger(DebugMode::Mode aMode)
	{
		PxInitExtensions(*myPhysics, myPvd);

		physx::PxPvdInstrumentationFlag::Enum flag = physx::PxPvdInstrumentationFlag::eALL;

		switch (aMode)
		{
		case Firefly::DebugMode::All:
			flag = physx::PxPvdInstrumentationFlag::eALL;
			break;
		case Firefly::DebugMode::Debug:
			flag = physx::PxPvdInstrumentationFlag::eDEBUG;

			break;
		case Firefly::DebugMode::Profile:
			flag = physx::PxPvdInstrumentationFlag::ePROFILE;

			break;
		case Firefly::DebugMode::Memory:
			flag = physx::PxPvdInstrumentationFlag::eMEMORY;

			break;
		default:
			break;
		}


		myPvd->connect(*myTransport, flag);
	}
	void PhysicsImplementation::DisconnectDebugger()
	{
		myPvd->disconnect();
	}
	void PhysicsImplementation::ShutDown()
	{
		SceneManager::Get().DestroyPhysicsScene();
		PHYSX_SAFE_DESTROY(myTransport);
		myBakery.Shutdown();
		PHYSX_SAFE_DESTROY(myPhysics);
		PHYSX_SAFE_DESTROY(myFoundation);
	}
	PhysicsBakery& PhysicsImplementation::GetBakery()
	{
		return myBakery;
	}
	PhysicQueryFilterCallback& PhysicsImplementation::GetQueryCallback()
	{
		return myFilterCallback;
	}
}