#pragma once
#include <cassert>

namespace Utils
{
	template<class T>
	class Queue
	{
	public:
		Queue(int aSize = 16);
		~Queue();

		Queue(const Queue& aQueue);

		void Initialize(int aSize);

		void Clear();

		int GetSize() const;
		bool IsEmpty() const;

		const T& GetFront() const;
		T& GetFront();

		void Enqueue(const T& aValue);
		//Enqueue's element without checking if capacity is full
		void EnqueueUnsafe(const T& aValue);
		T Dequeue();

	private:
		bool IsFull() const;

		T* myQueue;

		int myFirst;
		int myLast;
		int myCapacity;
	};

	template<class T>
	Queue<T>::Queue(int aSize)
	{
		myFirst = -1;
		myLast = -1;
		myQueue = new T[myCapacity = aSize];
	}

	template<class T>
	Queue<T>::~Queue()
	{
		if (myQueue != nullptr)
		{
			delete[] myQueue;
		}
	}

	template<class T>
	Queue<T>::Queue(const Queue& aQueue)
	{
		myCapacity = aQueue.myCapacity;
		myFirst = aQueue.myFirst;
		myLast = aQueue.myLast;

		T* newQueue = new T[myCapacity];

		for (int i = 0; i < myCapacity; ++i)
		{
			newQueue[i] = aQueue.myQueue[i];
		}

		myQueue = newQueue;
	}

	template<class T>
	void Queue<T>::Initialize(int aSize)
	{
		if (myQueue != nullptr)
		{
			delete[] myQueue;
		}

		myQueue = new T[myCapacity = aSize];
		Clear();
	}

	template<class T>
	void Queue<T>::Clear()
	{
		myFirst = -1;
		myLast = -1;
	}

	template<class T>
	int Queue<T>::GetSize() const
	{
		if (IsEmpty())
		{
			return 0;
		}

		if (IsFull())
		{
			return myCapacity;
		}

		return (myLast - myFirst + 1 + myCapacity) % myCapacity;
	}

	template<class T>
	bool Queue<T>::IsEmpty() const
	{
		return myFirst == -1;
	}

	template<class T>
	const T& Queue<T>::GetFront() const
	{
		assert(!IsEmpty() && L"Tried to get front from an empty queue");
		return myQueue[myFirst];
	}

	template<class T>
	T& Queue<T>::GetFront()
	{
		assert(!IsEmpty() && L"Tried to get front from an empty queue");
		return myQueue[myFirst];
	}

	template<class T>
	void Queue<T>::Enqueue(const T& aValue)
	{
		if (IsFull())
		{
			T* temp = new T[myCapacity * 2];

			for (int i = 0; i < myCapacity; ++i)
			{
				temp[i] = myQueue[(myFirst + i) % myCapacity];
			}

			myFirst = 0;
			myLast = myCapacity;
			temp[myLast] = aValue;

			myCapacity *= 2;

			delete[] myQueue;
			myQueue = temp;
		}
		else
		{
			if (IsEmpty())
			{
				myFirst++;
			}

			myLast = (myLast + 1) % myCapacity;
			myQueue[myLast] = aValue;
		}
	}

	template<class T>
	void Queue<T>::EnqueueUnsafe(const T& aValue)
	{
		if (IsEmpty())
		{
			myFirst++;
		}

		myLast = (myLast + 1) % myCapacity;
		myQueue[myLast] = aValue;
	}

	template<class T>
	T Queue<T>::Dequeue()
	{
		assert(!IsEmpty() && L"Tried to dequeue from already empty queue");

		T remove = myQueue[myFirst];

		if (myFirst == myLast)
		{
			myFirst = -1;
			myLast = -1;
		}
		else
		{
			myFirst = (myFirst + 1) % myCapacity;
		}

		return remove;
	}

	template<class T>
	bool Queue<T>::IsFull() const
	{
		if (myFirst == 0 && myLast == myCapacity - 1)
		{
			return true;
		}

		if (myFirst == myLast + 1)
		{
			return true;
		}

		return false;
	}
}