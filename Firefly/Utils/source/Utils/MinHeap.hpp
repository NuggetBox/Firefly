#pragma once
#include <cassert>

namespace Utils
{
	template<class T>
	class MinHeap
	{
	public:
		MinHeap(int aCapacity = 16);
		~MinHeap();

		int GetSize() const;
		bool IsEmpty() const;
		void Enqueue(const T& anElement);
		const T& GetTop() const;
		T Dequeue();

	private:
		void BubbleUp(int anIndex);
		void BubbleDown(int anIndex);
		bool IsFull() const;
		void DoubleCapacity();
		int GetIndexOfSmallestChild(int anIndex) const;
		T Last() const;

		T* myHeap;
		int myCapacity;
		int mySize;
	};

	template<class T>
	MinHeap<T>::MinHeap(int aCapacity)
	{
		mySize = 0;
		myHeap = new T[myCapacity = aCapacity];
	}

	template<class T>
	MinHeap<T>::~MinHeap()
	{
		delete[] myHeap;
		mySize = 0;
		myCapacity = 0;
	}

	template<class T>
	int MinHeap<T>::GetSize() const
	{
		return mySize;
	}

	template<class T>
	bool MinHeap<T>::IsEmpty() const
	{
		return mySize <= 0;
	}

	template<class T>
	void MinHeap<T>::Enqueue(const T& anElement)
	{
		if (IsFull())
		{
			DoubleCapacity();
		}

		myHeap[mySize++] = anElement;

		BubbleUp(mySize - 1);
	}

	template<class T>
	const T& MinHeap<T>::GetTop() const
	{
		assert(!IsEmpty() && L"Tried to get top from an empty heap");

		return myHeap[0];
	}

	template<class T>
	T MinHeap<T>::Dequeue()
	{
		assert(!IsEmpty() && L"Tried to dequeue from an empty heap");

		T root = myHeap[0];
		myHeap[0] = Last();
		mySize--;

		BubbleDown(0);

		return root;
	}

	template<class T>
	void MinHeap<T>::BubbleUp(const int anIndex)
	{
		const int parentIndex = (anIndex - 1) / 2;

		T parent = myHeap[parentIndex];
		T bubbler = myHeap[anIndex];

		if (bubbler < parent)
		{
			myHeap[parentIndex] = bubbler;
			myHeap[anIndex] = parent;
			BubbleUp(parentIndex);
		}
	}

	template<class T>
	void MinHeap<T>::BubbleDown(const int anIndex)
	{
		const int indexOfSmallestChild = GetIndexOfSmallestChild(anIndex);

		if (indexOfSmallestChild > 0)
		{
			T bubbler = myHeap[anIndex];
			myHeap[anIndex] = myHeap[indexOfSmallestChild];
			myHeap[indexOfSmallestChild] = bubbler;
			BubbleDown(indexOfSmallestChild);
		}
	}

	template<class T>
	bool MinHeap<T>::IsFull() const
	{
		return mySize == myCapacity;
	}

	template<class T>
	void MinHeap<T>::DoubleCapacity()
	{
		myCapacity *= 2;
		T* newHeap = new T[myCapacity];

		for (int i = 0; i < mySize; ++i)
		{
			newHeap[i] = myHeap[i];
		}
		
		delete[] myHeap;
		myHeap = newHeap;
	}

	template<class T>
	int MinHeap<T>::GetIndexOfSmallestChild(const int anIndex) const
	{
		T bubbler = myHeap[anIndex];

		const int leftChildIndex = 2 * anIndex + 1;
		const int rightChildIndex = leftChildIndex + 1;

		int smallestChildIndex = -1;

		if (leftChildIndex < mySize)
		{
			T leftChild = myHeap[leftChildIndex];

			if (leftChild < bubbler)
			{
				smallestChildIndex = leftChildIndex;
			}

			if (rightChildIndex < mySize)
			{
				T rightChild = myHeap[rightChildIndex];

				if (rightChild < bubbler && rightChild < leftChild)
				{
					smallestChildIndex = rightChildIndex;
				}
			}
		}

		return smallestChildIndex;
	}

	template<class T>
	T MinHeap<T>::Last() const
	{
		return myHeap[mySize - 1];
	}
}