#pragma once
#include "Firefly/Event/Event.h"
#include <sstream>
#include "Firefly/ComponentSystem/Scene.h"

namespace Firefly
{
	class Scene;
}

class EditorPlayEvent : public Firefly::Event
{
public:
	EditorPlayEvent() {}
	~EditorPlayEvent() {}

	EVENT_CLASS_TYPE(EditorPlay)
};

class EditorFileEvent : public Firefly::Event
{
public:
	EditorFileEvent(const std::filesystem::path& aPath) : myPath(aPath) {}
	~EditorFileEvent() {}

	std::filesystem::path& GetFilePath() { return myPath; }

	EVENT_CLASS_TYPE(EditorFile)
private:
	std::filesystem::path myPath;
};

class EditorStopEvent : public Firefly::Event
{
public:
	EditorStopEvent() {}
	~EditorStopEvent() {}

	EVENT_CLASS_TYPE(EditorStop)
};

class EmitterUpdatedEvent : public Firefly::Event
{
public:
	EmitterUpdatedEvent(const std::string& aPath) { myEmitterPath = aPath; }
	~EmitterUpdatedEvent() {}

	const std::string& GetPath() { return myEmitterPath; }

	EVENT_CLASS_TYPE(EmitterUpdated)

private:
	std::string myEmitterPath;
};

class EditorAnimatorChangedEvent : public Firefly::Event
{
public:
	EditorAnimatorChangedEvent(const uint64_t& aAnimatorID) { myAnimatorID = aAnimatorID; }
	~EditorAnimatorChangedEvent() {}

	const uint64_t& GetAnimatorID() { return myAnimatorID; }

	EVENT_CLASS_TYPE(EditorAnimatorChanged)
		
private:
	uint64_t myAnimatorID;
};

class VisualScriptUpdatedEvent : public Firefly::Event
{
public:
	VisualScriptUpdatedEvent(const std::string& aPath) { myVisualScriptPath = aPath; }
	~VisualScriptUpdatedEvent() {}

	const std::string& GetPath() { return myVisualScriptPath; }

	EVENT_CLASS_TYPE(VisualScriptUpdated)

private:
	std::string myVisualScriptPath;
};