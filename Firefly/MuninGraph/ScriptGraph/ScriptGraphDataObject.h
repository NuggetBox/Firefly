#pragma once
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif
#include <cassert>

class ScriptGraphType;

struct ScriptGraphDataObject : public Munin::ObjectUUID<ScriptGraphDataObject>
{
private:
	void SetDataInternal(const void* aValue, size_t aSize);
	void GetDataInternal(void* aValue, size_t aSize) const;
public:

	std::shared_ptr<const ScriptGraphType> TypeData;

	void* Ptr = nullptr;

	template<typename Type>
	static ScriptGraphDataObject Create()
	{
		ScriptGraphDataObject result(typeid(Type));
		result.Ptr = malloc(sizeof(Type));
		::new (result.Ptr) Type();
		return result;
	}

	template<typename Type>
	static ScriptGraphDataObject Create(const Type& aValue)
	{
		ScriptGraphDataObject result = Create<Type>();
		result.SetData(aValue);
		return result;
	}

	template<typename T>
	void SetData(const T& aValue)
	{
		assert(Ptr);
		SetDataInternal(&aValue, sizeof(T));
	}

	void SetDataRaw(const void* aValue, size_t aSize)
	{
		assert(Ptr);
		SetDataInternal(aValue, aSize);
	}

	template<typename T>
	T GetData() const
	{
		assert(Ptr);
		T result;
		GetDataInternal(&result, sizeof(T));
		return result;
	}

	ScriptGraphDataObject() = default;
	ScriptGraphDataObject(const std::type_index& aType);

	// No copying the Pin Data!
	ScriptGraphDataObject(const ScriptGraphDataObject&) = delete;

	// Moving is OK though!
	ScriptGraphDataObject(ScriptGraphDataObject&& other) noexcept;

	~ScriptGraphDataObject();

	ScriptGraphDataObject& operator=(const ScriptGraphDataObject& other);

	bool operator!() const
	{
		return TypeData && Ptr;
	}
};
