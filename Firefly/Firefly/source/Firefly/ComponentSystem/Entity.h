#pragma once

#include "Utils/Math/Transform.h"

#include <cassert>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "ComponentRegistry.hpp"
#include "Firefly/Core/Core.h"

namespace Firefly
{
	class Event;
	class Component;
}



namespace Firefly
{
	struct EntityModification
	{
		//ID of the modified entity
		uint64_t ID = 0;
		// format: ComponentName_ParameterName(_ArrayNum (for vectors and arrays))
		std::string Key = "";

		std::vector<float> FloatValues;
		std::vector<int> IntValues;
		std::vector<uint32_t> UintValues; //used for enums and bools
		std::vector<std::string> StringValues;
		std::vector<uint64_t> EntityIDValues;
	};

	class RenderBuffer;
	class Rigidbody;
	class Scene;
	struct Variable;
	class Entity
	{
	public:
		Entity(std::string aName = "New Entity", uint32_t aID = 0, std::string aTag = "Untagged");
		~Entity();

		static Ptr<Entity> Duplicate(Ptr<Entity> aEntityToCopy, Scene* aParentScene, bool aNewIDFlag, uint64_t aPrefabRootEntityID = 0);
		static void UpdatePrefabValues(Ptr<Entity> aPrefabRoot);
		static uint64_t GenerateRandomID();

		bool GetIsActive() const;
		void EarlyInitialize();
		void Initialize();
		void SetActive(bool aActiveFlag = true);
		void OnEvent(Firefly::Event& aEvent);

		const std::string& GetName() const;
		const std::string& GetTag() const;

		void SetName(const std::string& aName);
		void SetTag(const std::string& aTag);
		/// <summary>
		/// set child index to -1 to place at the end of the children list
		/// 
		/// Both the parent and child must be in a scene
		/// </summary>
		/// <param name="aParent"></param>
		/// <param name="aChildIndex"></param>
		void SetParent(Ptr<Entity> aParent, int aChildIndex = -1, bool aShouldMove = true, bool aKeepWorldTransformations = true);
		void SetParent(uint64_t aParentID, int aChildIndex = -1, bool aShouldMove = true, bool aKeepWorldTransformations = true);
		bool HasParent() const;

		inline Scene* GetParentScene() const { return myParentScene; }
		Ptr<Entity> GetParent();
		bool HasChildren() { return !myChildren.empty(); };
		std::vector<Ptr<Entity>> GetChildren();
		std::vector<Ptr<Component>> GetComponents();

		/// <summary>
		/// This counts all the children and their children and so on.
		/// if you need to use the return value more than once, save it in a variable as this is expensive.
		/// </summary>
		/// <returns></returns>
		int GetRecursiveChildrenCount();

		/// <summary>
		/// This collects all the children and their children and so on. returns a vector of shard pointers, 
		/// if you need to use the return value more than once, save it in a variable as this is expensive.
		/// </summary>
		/// <returns></returns>
		std::vector<Ptr<Entity>> GetChildrenRecursive();

		Ptr<Entity> GetChild(uint32_t aIndex);


		Ptr<Component> GetComponent(const std::string& aComponentName);
		template<class T>
		Ptr<T> GetComponent();
		/// <summary>
		/// Same as Get Component but only checks for base class of given class.
		/// </summary>
		/// <returns></returns>
		template<class T> Ptr<T> GetBaseComponent();
		template<class T>
		Ptr<T> GetComponentInChildren();
		template<class T, typename... TArgs>
		Ptr<T> GetOrCreateComponent(TArgs&&... aArgs);
		template<class T> bool HasComponent();
		bool HasComponent(const std::string& aComponentFactoryName);
		template<class T> bool SetComponentActive(const bool& aIsActive);
		template<class T> bool GetComponentActive();
		void SetAllComponentsActive(const bool& aIsActive);
		bool RemoveComponent(const std::string& aFactoryName);

		bool AddComponent(Ref<Component> aComponent, bool aShouldInitialize = true);
		void RemoveComponent(Ptr<Component> aComponent);

		Utils::Transform& GetTransform();

		void SetIDUnsafe(uint64_t aID) { myID = aID; }
		void SetParentIDUnsafe(uint64_t aID) { myParentID = aID; }
		void SetCorrespondingSourceID(uint64_t aID) { myCorrespondingSourceID = aID; }
		std::vector<uint64_t>& GetChildrenIDVecMutableUnsafe() { return myChildren; }

		float GetCreationTime() const;
		void ResetLifeTime();
		float GetLifeTime() const;

		uint64_t GetID() const;
		uint64_t GetParentID();
		//the ID of the root of this prefab in the scene
		uint64_t GetPrefabRootEntityID();
		//The ID of the prefab asset
		uint64_t GetPrefabID() const { return myPrefabID; }
		//The corresponding id in the prefab asset
		uint64_t GetCorrespondingSourceID();
		void SetPrefabID(uint64_t aID);
		bool IsPrefab() const { return myPrefabRootEntityID != 0; }
		bool IsPrefabRoot() const { return myPrefabRootEntityID == myID; }

		void SetPrefabRootEntityID(uint64_t aRootID);
		Ptr<Entity> GetPrefabRoot();

		void UnpackPrefab();

		void AddModification(const EntityModification& aModification, bool aAddToRoot = true);
		void RevertModification(Variable& aParam, const Component* const aComponent);
		void RemoveModification(const std::string& key, uint64_t aID);
		const std::vector<EntityModification>& GetModifications() const;
		bool IsParameterModified(const std::string& aKey);

		void UpdateTransformLocalPositionModification();
		void UpdateTransformLocalRotationModification();
		void UpdateTransformLocalScaleModification();

	private:
		friend class Scene;
		Utils::Transform myTransform;

		std::vector<std::shared_ptr<Component>> myComponents;
		std::unordered_map<std::string, std::shared_ptr<Component>> myComponentMap;

		std::vector<uint64_t> myChildren;

		Scene* myParentScene;

		std::string myName;
		std::string myTag;
		bool myIsActiveFlag;

		float myCreationTime;

		uint64_t myID = 0;
		uint64_t myParentID = 0;

		uint64_t myPrefabRootEntityID = 0;
		uint64_t myPrefabID = 0;
		uint64_t myCorrespondingSourceID = 0;

		std::vector<EntityModification> myPrefabModifications;
	};

	template<class T>
	inline Ptr<T> Entity::GetComponent()
	{
		auto it = myComponentMap.find(T::GetFactoryName());
		if (it != myComponentMap.end())
		{
			return std::reinterpret_pointer_cast<T>(it->second);
		}
		return Ptr<T>();
	}


	template <class T>
	Ptr<T> Entity::GetBaseComponent()
	{
		for (auto component : myComponents)
		{
			auto baseComp = std::dynamic_pointer_cast<T>(component);
			if (baseComp)
				return baseComp;
		}
		return Ptr<T>();
	}

	template<class T>
	inline Ptr<T> Entity::GetComponentInChildren()
	{
		auto children = GetChildrenRecursive();
		for (auto& child : children)
		{
			if (child.expired())
			{
				continue;
			}
			auto component = child.lock()->GetComponent<T>().lock();
			if (component)
			{
				return component;
			}
		}
		return Ptr<T>();
	}

	template<class T, typename ...TArgs>
	inline Ptr<T> Entity::GetOrCreateComponent(TArgs && ...aArgs)
	{
		auto it = myComponentMap.find(T::GetFactoryName());
		if (it == myComponentMap.end())
		{
			std::shared_ptr<Component> comp = ComponentRegistry::Create(T::GetFactoryName());
			AddComponent(comp, true);

			return std::reinterpret_pointer_cast<T>(comp);
		}

		return std::reinterpret_pointer_cast<T>(it->second);
	}

	template<class T>
	inline bool Entity::HasComponent()
	{
		return HasComponent(T::GetFactoryName());
	}

	template<class T>
	inline bool Entity::SetComponentActive(const bool& aIsActive)
	{
		auto it = myComponentMap.find(T::GetFactoryName());
		if (it != myComponentMap.end())
		{
			/*EntityOnComponentEnableEvent e = { aIsActive };
			it->second->OnEvent(e);*/
			it->second->myIsActive = aIsActive;
			return true;
		}
		return false;
	}

	template<class T>
	inline bool Entity::GetComponentActive()
	{
		auto it = myComponentMap.find(T::GetFactoryName());
		if (it != myComponentMap.end())
		{
			return it->second->myIsActive;
		}
		return false;
	}


}