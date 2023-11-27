#pragma once
#include <vector>
#include <cassert>

namespace Utils
{
	template<class T>
	class Stack
	{
	public:
		Stack(int aSize = 16);

		int GetSize() const;
		bool IsEmpty() const;

		const T& GetTop() const;
		T& GetTop();

		void Push(const T& aValue);
		T Pop();

		void PopBottom();

	private:
		std::vector<T> myObjects;
	};

	template<class T>
	Stack<T>::Stack(int aSize)
	{
		myObjects.reserve(aSize);
	}

	template<class T>
	int Stack<T>::GetSize() const
	{
		return static_cast<int>(myObjects.size());
	}

	template<class T>
	bool Stack<T>::IsEmpty() const
	{
		return myObjects.empty();
	}

	template<class T>
	const T& Stack<T>::GetTop() const
	{
		assert(!myObjects.empty() && L"Tried to get the top object from an empty stack");
		return myObjects.back();
	}

	template<class T>
	T& Stack<T>::GetTop()
	{
		assert(!myObjects.empty() && L"Tried to get the top object from an empty stack");
		return myObjects.back();
	}

	template<class T>
	void Stack<T>::Push(const T& aValue)
	{
		myObjects.push_back(aValue);
	}

	template<class T>
	T Stack<T>::Pop()
	{
		assert(!myObjects.empty() && L"Tried to pop an item from an empty stack");

		T value = myObjects[myObjects.size() - 1];
		myObjects.pop_back();
		return value;
	}
	template<class T>
	inline void Stack<T>::PopBottom()
	{
		assert(!myObjects.empty() && L"Tried to pop an item from an empty stack");

		myObjects.erase(myObjects.begin());
	}
}
