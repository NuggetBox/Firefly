#pragma once
#include "Event.h"

namespace Firefly
{
	class Scene;

	class SceneLoadedEvent : public Firefly::Event
	{
	public:

		SceneLoadedEvent( Ptr<Scene> aScene, bool aAddFlag)
		{
			myScene = aScene;
			myAddFlag = aAddFlag;
		}
		~SceneLoadedEvent() {}
		
		Ptr<Scene> GetScene() { return myScene; }
		bool GetAddFlag() { return myAddFlag; }

		EVENT_CLASS_TYPE(SceneLoaded)
	private:
		std::string myPath;
		Ptr<Scene> myScene;
		bool myAddFlag = false;
	};
	//sent right before unloading a scene
	class SceneUnloadedEvent : public Firefly::Event
	{
	public:
		SceneUnloadedEvent(Ptr<Scene> aScene)
		{
			myScene = aScene;
		}
		~SceneUnloadedEvent() {}

		Ptr<Scene> GetScene() { return myScene; }

		EVENT_CLASS_TYPE(SceneUnloaded)
	private:
		std::string myPath;
		Ptr<Scene> myScene;
	};
}