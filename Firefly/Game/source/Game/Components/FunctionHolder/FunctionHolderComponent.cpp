#include "Gamepch.h"
#include "FunctionHolderComponent.h"
#include "Firefly/Components/Sprites/Sprite2DComponent.h"
#include "Firefly/Rendering/RenderCommands.h"
#include "Utils/Timer.h"

using namespace Firefly;
using namespace Utils;

FunctionHolderComponent::FunctionHolderComponent() : Component("FunctionHolderComponent")
{
}

FunctionHolderComponent::FunctionHolderComponent(const std::string& aName) : Component(aName)
{
}

void FunctionHolderComponent::Initialize()
{
}

void FunctionHolderComponent::CallFunction()
{
	LOGERROR("Called from Function Holder Base Class !!!");
}

void FunctionHolderComponent::CallFunction(void* aArg)
{
}

void FunctionHolderComponent::OnEvent(Firefly::Event& aEvent)
{
}
