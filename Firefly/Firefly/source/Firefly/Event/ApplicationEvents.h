#pragma once
#include "Firefly/Event/Event.h"

#include <sstream>

namespace Firefly
{
	class EditorAppUpdateEvent : public Event
	{
	public:
		EditorAppUpdateEvent(bool aIsInPlayMode) { myIsInPlayMode = aIsInPlayMode; }
		~EditorAppUpdateEvent() {}

		EVENT_CLASS_TYPE(EditorAppUpdate)

		bool GetIsInPlayMode() { return myIsInPlayMode; }

	private:
		bool myIsInPlayMode = false;
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent(bool aIsInPlayMode) { myIsInPlayMode = aIsInPlayMode; }
		~AppUpdateEvent() {}

		EVENT_CLASS_TYPE(AppUpdate)

		bool GetIsInPlayMode() { return myIsInPlayMode; }

	private:
		bool myIsInPlayMode = false;
	};

	class AppFixedUpdateEvent : public Event
	{
	public:
		AppFixedUpdateEvent(bool aIsInPlayMode) { myIsInPlayMode = aIsInPlayMode; }
		~AppFixedUpdateEvent() {}

		bool GetIsInPlayMode() { return myIsInPlayMode; }

		EVENT_CLASS_TYPE(AppFixedUpdate)
	private:
		bool myIsInPlayMode = false;
	};

	class AppLateFixedUpdateEvent : public Event
	{
	public:
		AppLateFixedUpdateEvent(bool aIsInPlayMode) { myIsInPlayMode = aIsInPlayMode; }
		~AppLateFixedUpdateEvent() {}

		bool GetIsInPlayMode() { return myIsInPlayMode; }

		EVENT_CLASS_TYPE(AppLateFixedUpdate)
	private:
		bool myIsInPlayMode = false;
	};

	class AppLateUpdateEvent : public Event
	{
	public:
		AppLateUpdateEvent(bool aIsInPlayMode) { myIsInPlayMode = aIsInPlayMode; }
		~AppLateUpdateEvent() {}

		EVENT_CLASS_TYPE(AppLateUpdate)

			bool GetIsInPlayMode() { return myIsInPlayMode; }

	private:
		bool myIsInPlayMode = false;
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}
		~AppRenderEvent() {}

		EVENT_CLASS_TYPE(AppRender)
	};
}