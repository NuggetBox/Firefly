#pragma once

#include <string>
#include <vector>
#include <functional>
#include "ComponentSystemUtils.h"

#define BIND_COMP_BUTTON_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Firefly
{
	class Event;
}

class InspectorWindow;

namespace Firefly
{
	enum class ParameterType
	{
		Int,
		Float,
		String,
		Bool,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Button,
		Entity,
		File,
		Enum,
		List,
		Header,
		HeaderEnd
	};
	int GetByteSize(const Firefly::ParameterType& aType);
	struct Variable
	{
		std::string Name;
		ParameterType Type;
		void* Pointer;

		//used for number sliders
		float Increment = 1.0f;
		float Min = 0;
		float Max = 0;
		//
		
		//Used for enum parameter
		std::function<std::string(uint32_t)> EnumToStringFunction;
		uint32_t EnumCount = 0;
		std::vector<std::string> EnumNames;
		//

		ParameterType ListType; // used for list parameter
		size_t ListCount = 0;

		std::function<void()> ButtonFunction; // Used for buttons
		std::string FileExtensions = ""; // Only used for file parameters
		uint64_t EntityID = 0; //for entity parameters
		std::vector<uint64_t> EntityIDVector; //for entity list parameters
		std::string Tooltip = "";

		int HeaderDepth = 0;

		bool DefaultOpen = false; //For list variables
		bool Hidden = false;
		bool UseAlpha = true; // For Color
	};

	struct EntityModification;
	class Scene;

	std::string ParameterTypeToString(ParameterType aType);
	ParameterType StringToParameterType(const std::string& aType);

	//void SetParameterFromModificationas(Variable& aParam, EntityModification& aModification, Scene* aScene = nullptr, bool aSetEntityFlag = false);
	void CopyParameterValue(Variable& aParam, const Variable& aParamToCopy, Scene* aScene = nullptr, bool aSetEntityFlag = false);


	class Entity;
	class Component
	{
	public:
		Component(const std::string& aName);
		Component(Component& aComp);
		virtual ~Component() = default;

		Component(const Component&) = delete;
		Component(Component&&) = delete;
		Component& operator=(const Component&) = delete;
		Component& operator=(Component&&) = delete;


		static Ref<Component> Duplicate(Ptr<Component> aComponentToCopy, Ptr<Entity> aTargetEnt);

		virtual void Initialize() {}
		virtual void EarlyInitialize() {}

		virtual void OnEvent(Firefly::Event& aEvent) {}
		virtual void OnRemove() {};

		void SetEntity(Entity* aEntity);
		Entity* GetEntity() { return myEntity; }

		void SetIsActive(bool aIsActive) { myIsActive = aIsActive; }
		bool GetIsActive() { return myIsActive; }

		inline const std::string& GetName() const { return myName; }

		const std::vector<Variable>& GetSerializedVariables() const { return mySerializedVariables; }
		std::vector<Variable>& GetSerializedVariablesMutable() { return mySerializedVariables; }

	protected:
		Entity* myEntity;
		template<class T>
		Ptr<T> GetComponent();
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue);
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue, bool aUseAlpha);
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue, std::function<std::string(uint32_t)> aToStringFunction, int myEnumCount);
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue, const std::vector<std::string>& someEnumNames);
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue, float aIncrement, float aMin, float aMax);
		//separate different extensions with an ";"
		void EditorVariable(const std::string& aName, ParameterType aType, void* aValue,const char* someExtensions);

		void EditorListVariable(const std::string& aName, ParameterType aType, void* aValue);
		void EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, std::function<std::string(uint32_t)> aToStringFunction, int myEnumCount);
		void EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, const std::vector<std::string>& someEnumNames);
		void EditorListVariable(const std::string& aName, ParameterType aType, void* aValue,float aIncrement, float aMin, float aMax);
		void EditorListVariable(const std::string& aName, ParameterType aType, void* aValue, const std::string& someExtensions);

		//adds a tooltip the the last added variable
		void EditorTooltip(const std::string& aToolTip);
		void EditorListDefaultOpen();
		void EditorHideVariable();
		void EditorHeader(const std::string& aHeaderText);
		void EditorEndHeader();
		void EditorButton(const std::string& aButtonText, std::function<void()> aFunctionToCall);

		std::vector<Variable> mySerializedVariables;
	private:
		friend class Entity;
		friend class ::InspectorWindow;

		std::string myName;
		bool myIsActive;
	};

	template<class T>
	inline Ptr<T> Component::GetComponent()
	{
		return myEntity->GetComponent<T>();
	}
}
