#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <utility>

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

#include "ScriptGraphDataObject.h"

enum class ScriptGraphNodeType : uint8_t
{
	Undefined,
	Function,
	Event,
};

struct ScriptGraphColor
{
	float R = 0;
	float G = 0;
	float B = 0;
	float A = 0;

	const static ScriptGraphColor White;
	const static ScriptGraphColor Black;

	ScriptGraphColor() = default;
	ScriptGraphColor(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {  }

	ScriptGraphColor AsBase255() const { return { R * 255, G * 255, B * 255, A * 255 }; }
	ScriptGraphColor AsNormalized() const { return { R / 255, G / 255, B / 255, A / 255 }; }

	uint32_t AsU32() const { return (static_cast<uint32_t>(A) << 24 | static_cast<uint32_t>(B) << 16 | static_cast<uint32_t>(G) << 8 | static_cast<uint32_t>(R) << 0); }
};

class ScriptGraphType;

class ScriptGraphDataTypeRegistry
{
	typedef std::unordered_map<std::type_index, std::shared_ptr<ScriptGraphType>> TypeMap;

	static TypeMap& MyTypesMap()
	{
		static TypeMap myMap;
		return myMap;
	}

	static inline std::unordered_map<std::string, std::type_index> myStringToType;
	static inline std::unordered_map<std::string, std::type_index> myFriendlyNameToType;

	// TODO: I need a better idea for this one :P
	static inline bool typeStringsSorted = false;
	static inline std::vector<std::string> myTypeStrings;

public:

	template<typename H, typename T>
	static bool Register()
	{
		assert(MyTypesMap().find(typeid(T)) == MyTypesMap().end() && "A handler for that type has already been registered!");
		std::shared_ptr<H> ptr = std::make_shared<H>();
		ptr->MakeDataObject = []() { return ScriptGraphDataObject::Create<T>(); };
		MyTypesMap().insert({ typeid(T), ptr });
		myStringToType.insert({ ptr->GetTypeName(), typeid(T) });
		myFriendlyNameToType.insert({ ptr->GetFriendlyName(), typeid(T) });
		myTypeStrings.push_back(ptr->GetFriendlyName());
		return true;
	}

	static std::shared_ptr<const ScriptGraphType> GetType(const std::type_index& aType);
	static std::shared_ptr<const ScriptGraphType> GetType(const std::string& aType);
	static std::shared_ptr<const ScriptGraphType> GetTypeFromFriendlyName(const std::string& aFriendlyName);

	static void RenderEditInPlaceWidget(const std::type_index& aType, const std::string& aContainerUUID, void* aDataPtr);
	static std::string GetString(const std::type_index& aType, void* aDataPtr);
	static void Serialize(const std::type_index& aType, const void* aDataPtr, std::vector<uint8_t>& outData);
	static void Deserialize(const std::type_index& aType, void* outDataPtr, const std::vector<uint8_t>& inData);

	static std::string GetFriendlyName(const std::type_index& aType);
	static std::type_index GetTypeFromString(const std::string& aType);

	static ScriptGraphDataObject GetDataObjectOfType(const std::type_index& aType);
	static ScriptGraphDataObject GetDataObjectOfType(const std::string& aType);
	static ScriptGraphColor GetDataTypeColor(const std::type_index aType);
	static ScriptGraphColor GetDataTypeColorNormalized(const std::type_index aType);
	static bool IsTypeInPlaceConstructible(const std::type_index& aType);

	FORCEINLINE static const std::vector<std::string>& GetRegisteredTypeNames() { if (!typeStringsSorted) { std::sort(myTypeStrings.begin(), myTypeStrings.end()); typeStringsSorted = true; }  return myTypeStrings; }
};

class ScriptGraphType
{
	friend class ScriptGraphDataTypeRegistry;

	std::type_index Type = typeid(std::nullptr_t);
	std::string TypeName = "std::nullptr_t";
	std::string SimpleTypeName = "nullptr";
	std::string FriendlyName = "Nullptr";
	size_t TypeSize = 0;

	ScriptGraphColor Color = ScriptGraphColor::White;
	bool CanConstructInPlace = false;
	
	// Determines the actual type name for reflection purposes.
	[[nodiscard]] std::string FetchTypeName(const std::type_index& aType) const;
	// Determines the simple type name (i.e. for TemplateClass<A, B, C, D> it returns TemplateClass).
	[[nodiscard]] std::string FetchSimpleTypeName(const std::string& aFullName) const;

	ScriptGraphType() = default;

public:
	typedef std::function<ScriptGraphDataObject()> MakeDataObjectFunc;
	const static ScriptGraphType NullType;
private:
	
	MakeDataObjectFunc MakeDataObject = nullptr;
	
public:

	ScriptGraphType(const std::type_index& aType, size_t aTypeSize, std::string aFriendlyName, const ScriptGraphColor& aColor, bool canConstructInPlace = false)
		: Type(aType), TypeName(FetchTypeName(Type)), SimpleTypeName(FetchSimpleTypeName(TypeName)),
		  FriendlyName(std::move(aFriendlyName)), TypeSize(aTypeSize), Color(aColor), CanConstructInPlace(canConstructInPlace)
	{  }

	virtual ~ScriptGraphType() = default;

	virtual void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) {}
	virtual std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const { return ""; }
	virtual void SerializeData(const void* aDataPtr, const ScriptGraphType& aTypeInfo, std::vector<uint8_t>& outData) const;
	virtual void DeserializeData(const std::vector<uint8_t>& inData, const ScriptGraphType& aTypeInfo, void* outDataPtr) const;

	FORCEINLINE const std::type_index& GetType() const { return Type; }
	FORCEINLINE const std::string& GetTypeName() const { return TypeName; }
	FORCEINLINE const std::string& GetSimpleTypeName() const { return SimpleTypeName; }
	FORCEINLINE size_t GetTypeSize() const { return TypeSize; }
	FORCEINLINE const std::string& GetFriendlyName() const { return FriendlyName; }

	bool operator==(const ScriptGraphType& other) const
	{
		return Type == other.Type;
	}

	bool operator!=(const ScriptGraphType& other) const
	{
		return !(Type == other.Type);
	}

	operator const std::type_index&() const
	{
		return Type;
	}
};

template<typename D, typename T>
struct ScriptGraphTypeHandler
{
	static inline bool IsRegistered = ScriptGraphDataTypeRegistry::Register<D, T>();
};

#define BeginDataTypeHandler(N, T, C, B) struct N##ScriptGraphType : public ScriptGraphType, public ScriptGraphTypeHandler<N##ScriptGraphType, T> { N##ScriptGraphType() : ScriptGraphType(typeid(T), sizeof(T), #N, C, B)  {  }
#define EndDataTypeHandler };