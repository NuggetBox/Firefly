#pragma once
#include <string>
#include <unordered_map>

#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Entity;
}

class ScriptGraph;
struct ScriptGraphNodePayload;

class ScriptEventManager
{
public:
	static void SubscribeToEvent(const std::string& aEvent, const Ref<ScriptGraph>& aScriptGraph, uint64_t aRecievingEntityID, size_t aEventNodeID);
	static void TriggerEvent(const std::string& aEvent, int aIndex, uint64_t aTargetEntityID = 0, uint64_t aEntityIDToSend = 0);

	static void CleanUp();

private:
	struct Subscription
	{
		Ref<ScriptGraph> ScriptGraph;
		uint64_t EntityID;
		size_t EventNodeID;
	};

	static inline std::unordered_map<std::string, std::vector<Subscription>> ourEventSubMap;
};