#include "ScriptEventManager.h"

#include "Firefly/Components/VisualScriptComponent.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"

#include "MuninScriptGraph.h"
#include "ScriptGraph/ScriptGraph.h"

void ScriptEventManager::SubscribeToEvent(const std::string& aEvent, const Ref<ScriptGraph>& aScriptGraph, uint64_t aRecievingEntityID, size_t aEventNodeID)
{
	Subscription subscription;
	subscription.ScriptGraph = aScriptGraph;
	subscription.EntityID = aRecievingEntityID;
	subscription.EventNodeID = aEventNodeID;

	if (ourEventSubMap.contains(aEvent))
	{
		const auto& subs = ourEventSubMap.at(aEvent);

		for (const auto& sub : subs)
		{
			if (sub.EventNodeID == aEventNodeID)
			{
				LOGWARNING("ScriptEventManager::SubscribeToEvent: Event already subscribed for this script");
				return;
			}
		}

		ourEventSubMap.at(aEvent).push_back(subscription);
	}
	else
	{
		std::vector<Subscription> subs;
		subs.push_back(subscription);
		ourEventSubMap.insert({ aEvent, subs });
	}
}

void ScriptEventManager::TriggerEvent(const std::string& aEvent, int aIndex, uint64_t aTargetEntityID, uint64_t aEntityIDToSend)
{
	if (!ourEventSubMap.contains(aEvent))
	{
		LOGWARNING("Event {} was triggered but no-one was subscribed to it", aEvent);
		return;
	}

	for (const auto& sub : ourEventSubMap.at(aEvent))
	{
		if (aTargetEntityID != 0)
		{
			if (sub.EntityID != aTargetEntityID)
			{
				continue;
			}
		}

		if (sub.ScriptGraph)
		{
			ScriptGraphNodePayload payload;
			payload.SetVariable("Entity", aEntityIDToSend);
			payload.SetVariable("Index", aIndex);

			sub.ScriptGraph->RunWithPayload("On Event", payload);
		}
	}
}

void ScriptEventManager::CleanUp()
{
	LOGINFO("ScriptEventManager::CleanUp: All Visual Script Events unsubscribed");
	ourEventSubMap.clear();
}