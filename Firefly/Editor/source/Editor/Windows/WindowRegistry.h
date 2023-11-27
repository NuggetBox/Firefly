#pragma once

//https://dxuuu.xyz/cpp-static-registration.html

#define REGISTER_WINDOW(x) static bool x ## _entry = WindowRegistry::Add(x::GetFactoryName(), x::Create)

class EditorWindow;
class WindowRegistry
{
public:
	typedef std::function<std::shared_ptr<EditorWindow>()> FactoryFunction;
	typedef std::unordered_map<std::string, FactoryFunction> FactoryMap;

	static bool Add(const std::string& aName, FactoryFunction aFacFunc)
	{
		auto& map = GetFactoryMap();
		if (map.find(aName) != map.end()) {
			return false;
		}

		GetFactoryMap()[aName] = aFacFunc;
		return true;
	}

	static std::shared_ptr<EditorWindow> Create(const std::string& aName)
	{
		auto& map = GetFactoryMap();
		if (map.find(aName) == map.end())
		{
			return nullptr;
		}

		return map[aName]();
	}

	static FactoryMap& GetFactoryMap()
	{
		static FactoryMap map;
		return map;
	}
private:

};