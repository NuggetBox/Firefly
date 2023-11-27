#include "InputMap.h"

#include <fstream>
#include <iostream>

#include "InputHandler.h"
#include "../../Vendor/include/nlohmann/json.hpp"

InputMap::InputMap()
{

}

void InputMap::Init()
{
	myKeybinds.insert({ Keybinds::Interact, 'E' });
	myKeybinds.insert({ Keybinds::Pause, 'P' });
	Save();
	Load();
}

unsigned InputMap::GetKeybind(Keybinds aKeybind)
{
	return myKeybinds[aKeybind];
}

std::unordered_map<InputMap::Keybinds, unsigned> InputMap::GetAllKeyBinds()
{
	return myKeybinds;
}

bool InputMap::IsKeybindDown(Keybinds aKeyBind)
{
	auto key = myKeybinds[aKeyBind];
	if (key < 5 && key > 0)
	{
		switch (key)
		{
		case 1:
			return Utils::InputHandler::GetLeftClickDown();
			break;
		case 2:
			return Utils::InputHandler::GetRightClickDown();
			break;
		case 3:
			return Utils::InputHandler::GetXButtonDown(1);
			break;
		case 4:
			return Utils::InputHandler::GetXButtonDown(2);
			break;
		}
	}

	return Utils::InputHandler::GetKeyDown(key);
}

bool InputMap::IsKeybindHeld(Keybinds aKeyBind)
{
	auto key = myKeybinds[aKeyBind];
	if (key < 5 && key > 0)
	{
		switch (key)
		{
		case 1:
			return Utils::InputHandler::GetLeftClickHeld();
			break;
		case 2:
			return Utils::InputHandler::GetRightClickHeld();
			break;
		case 3:
			return Utils::InputHandler::GetXButtonHeld(1);
			break;
		case 4:
			return Utils::InputHandler::GetXButtonHeld(2);
			break;
		}
	}
	return Utils::InputHandler::GetKeyHeld(key);
}

bool InputMap::IsKeybindUp(Keybinds aKeyBind)
{
	auto key = myKeybinds[aKeyBind];
	if (key < 5 && key > 0)
	{
		switch (key)
		{
		case 1:
			return Utils::InputHandler::GetLeftClickUp();
			break;
		case 2:
			return Utils::InputHandler::GetRightClickUp();
			break;
		case 3:
			return Utils::InputHandler::GetXButtonUp(1);
			break;
		case 4:
			return Utils::InputHandler::GetXButtonUp(2);
			break;
		}
	}

	return Utils::InputHandler::GetKeyUp(key);
}

void InputMap::SetKeybind(Keybinds aKeybind, unsigned aValue)
{
	myKeybinds[aKeybind] = aValue;
}

void InputMap::Save()
{
	if (!std::filesystem::exists("User/"))
	{
		std::filesystem::create_directory("User/");
	}

	std::ofstream file("User/Keybinds.json");
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json json;

	for (auto bind : myKeybinds)
	{
		json[std::to_string(static_cast<int>(bind.first))] = bind.second;
	}

	file << std::setw(4) << json;
	file.close();
}

void InputMap::Load()
{
	if (!std::filesystem::exists("User/Keybinds.json"))
		return;

	nlohmann::json json;
	std::ifstream ifStream("User/Keybinds.json");
	if (ifStream.fail())
		return;

	ifStream >> json;

	for (int i = 0; i < static_cast<int>(InputMap::Keybinds::Count); i++)
	{
		if (json.contains(std::to_string(i)))
		{
			myKeybinds[static_cast<Keybinds>(i)] = json[std::to_string(i)];
		}
	}

	ifStream.close();
}
