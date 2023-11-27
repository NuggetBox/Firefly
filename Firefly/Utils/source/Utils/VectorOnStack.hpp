#pragma once
#include <cassert>
#include <initializer_list>

namespace Utils
{
	template <typename Type, int size, typename CountType = unsigned short, bool UseSafeModeFlag = true>
	class VectorOnStack
	{
	public:
		VectorOnStack();
		VectorOnStack(const std::initializer_list<Type>& anInitializerList);
		VectorOnStack(const VectorOnStack& aVectorOnStack);
		~VectorOnStack();
		VectorOnStack& operator=(const VectorOnStack& aVectorOnStack);
		inline const Type& operator[](const CountType anIndex) const;
		inline Type& operator[](const CountType anIndex);
		inline void Add(const Type& anObject);
		inline void Insert(const CountType anIndex, const Type& anObject);
		inline void RemoveCyclic(const Type& anObject);
		inline void RemoveCyclicAtIndex(const CountType anIndex);
		inline void Clear();
		__forceinline CountType Size() const;

	private:
		Type myArray[size];
		CountType myCurrentSize;
	};

	// Default constructor
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	VectorOnStack<Type, size, CountType, UseSafeModeFlag>::VectorOnStack()
	{
		myCurrentSize = 0;
	}

	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	VectorOnStack<Type, size, CountType, UseSafeModeFlag>::VectorOnStack(const std::initializer_list<Type>& anInitializerList)
	{
		assert(anInitializerList.size() <= size && "VectorOnStack: Initializer list initialization failed, list was larger than VectorOnStack maximum size");

		myCurrentSize = 0;

		for (const Type& object : anInitializerList)
		{
			Add(object);
		}
	}

	//Copy constructor
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	VectorOnStack<Type, size, CountType, UseSafeModeFlag>::VectorOnStack(const VectorOnStack& aVectorOnStack)
	{
		//assert below should not be needed since VectorOnStack with different maximum sizes should not be able to use the copy constructor on each other in the first place.
		//assert(size <= aVectorOnStack.Size() && "VectorOnStack: Tried to use copy constructor with a VectorOnStack that is larger than maximum allowed size for the other VectorOnStack");

		myCurrentSize = aVectorOnStack.Size();

		for (CountType i = 0; i < myCurrentSize; ++i)
		{
			myArray[i] = aVectorOnStack[i];
		}
	}

	//Destructor
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	VectorOnStack<Type, size, CountType, UseSafeModeFlag>::~VectorOnStack()
	{
		myCurrentSize = 0;
	}

	// = operator
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	VectorOnStack<Type, size, CountType, UseSafeModeFlag>& VectorOnStack<Type, size, CountType, UseSafeModeFlag>::operator=(const VectorOnStack& aVectorOnStack)
	{
		//assert below should not be needed since VectorOnStack with different maximum sizes should not be able to use the assignment operator on each other in the first place.
		//assert(size <= aVectorOnStack.Size() && "Tried to use assignment operator= with a VectorOnStack that is larger than maximum allowed size for the other VectorOnStack");

		myCurrentSize = aVectorOnStack.Size();

		for (CountType i = 0; i < myCurrentSize; ++i)
		{
			myArray[i] = aVectorOnStack[i];
		}

		return *this;
	}

	// [] Read
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	const Type& VectorOnStack<Type, size, CountType, UseSafeModeFlag>::operator[](const CountType anIndex) const
	{
		assert(anIndex >= 0 && anIndex < myCurrentSize && "VectorOnStack: Index out of bounds while trying to use the operator[] to read value");

		return myArray[anIndex];
	}

	// [] Assign
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	Type& VectorOnStack<Type, size, CountType, UseSafeModeFlag>::operator[](const CountType anIndex)
	{
		assert(anIndex >= 0 && anIndex < myCurrentSize && "VectorOnStack: Index out of bounds while trying to use the assignment operator[]");

		return myArray[anIndex];
	}

	// Add function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	void VectorOnStack<Type, size, CountType, UseSafeModeFlag>::Add(const Type& anObject)
	{
		assert(myCurrentSize < size && "VectorOnStack: Tried to add an object to an already full VectorOnStack");

		myArray[myCurrentSize++] = anObject;
	}

	// Insert function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	void VectorOnStack<Type, size, CountType, UseSafeModeFlag>::Insert(const CountType anIndex, const Type& anObject)
	{
		assert(myCurrentSize < size && "VectorOnStack: Tried to insert an object to an already full VectorOnStack");
		assert(anIndex >= 0 && anIndex <= myCurrentSize && "VectorOnStack: Tried to insert an object out of bounds");

		for (CountType i = ++myCurrentSize; i > anIndex; --i)
		{
			myArray[i] = myArray[i - 1];
		}

		myArray[anIndex] = anObject;
	}

	//RemoveCyclic function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	void VectorOnStack<Type, size, CountType, UseSafeModeFlag>::RemoveCyclic(const Type& anObject)
	{
		for (CountType i = 0; i < myCurrentSize; ++i)
		{
			if (myArray[i] == anObject)
			{
				myArray[i] = myArray[--myCurrentSize];
				return;
			}
		}

		//TODO: Error message for object not found?
	}

	//RemoveCyclicAtIndex function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	void VectorOnStack<Type, size, CountType, UseSafeModeFlag>::RemoveCyclicAtIndex(const CountType anIndex)
	{
		assert(anIndex >= 0 && anIndex < myCurrentSize && "VectorOnStack: Tried to RemoveCyclicAtIndex out of bounds");

		myArray[anIndex] = myArray[--myCurrentSize];
	}

	// Clear function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	void VectorOnStack<Type, size, CountType, UseSafeModeFlag>::Clear()
	{
		myCurrentSize = 0;
	}

	//Size function
	template <typename Type, int size, typename CountType, bool UseSafeModeFlag>
	CountType VectorOnStack<Type, size, CountType, UseSafeModeFlag>::Size() const
	{
		return myCurrentSize;
	}
}
