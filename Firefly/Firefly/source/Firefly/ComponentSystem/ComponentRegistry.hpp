#pragma once

#include <unordered_map>
#include <functional>
#include <memory>

//https://dxuuu.xyz/cpp-static-registration.html

#define REGISTER_COMPONENT(x) static bool x ## _entry = Firefly::ComponentRegistry::Add(x::GetFactoryName(), x::Create)

namespace Firefly
{
	class Component;
	class ComponentRegistry
	{
	public:
		typedef std::function<std::shared_ptr<Component>()> FactoryFunction;
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

		static std::shared_ptr<Component> Create(const std::string& aName)
		{
			auto& map = GetFactoryMap();
			if (map.find(aName) == map.end())
			{
				return nullptr;
			}

			return map[aName]();
		}

		template<class T>
		static std::shared_ptr<T> Create()
		{
			return std::reinterpret_pointer_cast<T>(Create(T::GetFactoryName()));
		}

		static FactoryMap& GetFactoryMap()
		{
			static FactoryMap map;
			return map;
		}
	private:

	};
}