#include "FFpch.h"
#include "UISceneLoaderConnector.h"

#include "UIButton.h"
#include "Firefly/ComponentSystem/ComponentRegistry.hpp"
#include "Firefly/ComponentSystem/Entity.h"
#include "Firefly/ComponentSystem/SceneManager.h"
#include "Firefly/Event/EntityEvents.h"
#include "Firefly/Event/Event.h"

REGISTER_COMPONENT(UISceneLoaderConnector);

UISceneLoaderConnector::UISceneLoaderConnector() : Component("UISceneLoaderConnector")
{
	myPath = "";
	using namespace Firefly;

	EditorVariable("Button Entity", ParameterType::Entity, &myButtonEntity);
	EditorVariable("Scene Path", ParameterType::File, &myPath, ".scene");
}

void UISceneLoaderConnector::Initialize()
{
	Component::Initialize();

	if (myButtonEntity.lock() != nullptr)
	{
		auto button = myButtonEntity.lock()->GetComponent<UIButton>().lock();
		if (button != nullptr)
			button->BindFunction({ [&]()
			{
					std::filesystem::path p(myPath);
					Firefly::SceneManager::Get().LoadScene(p);

			} });
	}
}

void UISceneLoaderConnector::OnEvent(Firefly::Event& aEvent)
{
	Firefly::EventDispatcher dispatcher(aEvent);

	dispatcher.Dispatch<Firefly::EntityPropertyUpdatedEvent>([&](Firefly::EntityPropertyUpdatedEvent& aEvent)
		{
			if (myButtonEntity.lock() != nullptr)
			{
				auto button = myButtonEntity.lock()->GetComponent<UIButton>().lock();
				if (button != nullptr)
					button->BindFunction({ [&]()
					{
							std::filesystem::path p(myPath);
							if (p.empty())
								Firefly::SceneManager::Get().LoadScene(p);
					} });
			}
			return false;
		});
}
