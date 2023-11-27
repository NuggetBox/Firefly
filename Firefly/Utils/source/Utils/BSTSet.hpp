#pragma once

namespace Utils
{
	template<class T>
	struct TreeNode
	{
		~TreeNode()
		{
			/*delete myLeft;
			myLeft = nullptr;
			delete myRight;
			myRight = nullptr;*/
		}

		int AmountOfChildren()
		{
			int childCount = 0;
			if (myLeft != nullptr)
			{
				childCount++;
			}
			if (myRight != nullptr)
			{
				childCount++;
			}
			return childCount;
		}

		TreeNode<T>* myLeft;
		TreeNode<T>* myRight;
		T myValue;
	};

	template<class T>
	bool operator==(const TreeNode<T>& aFirst, const T& aSecond)
	{
		return !(aFirst.myValue < aSecond) && !(aSecond < aFirst.myValue);
	}

	template<class T>
	class BSTSet
	{
	public:
		BSTSet();
		~BSTSet();

		//Returnerar true om elementet finns i mängden, och false annars.
		bool HasElement(const T& aValue);

		//Stoppar in elementet i mängden, om det inte redan finns där. Gör ingenting annars.
		void Insert(const T& aValue);

		//Plockar bort elementet ur mängden, om det finns. Gör ingenting annars.
		void Remove(const T& aValue);

		int GetSize();

	private:
		bool HasElement(TreeNode<T>* aRoot, const T& aValue);

		void Insert(TreeNode<T>*& aRoot, const T& aValue);

		void DeleteAndMerge(TreeNode<T>*& aNode);

		TreeNode<T>* myRoot;
		unsigned int mySize;
	};

	template<class T>
	BSTSet<T>::BSTSet()
	{
		myRoot = nullptr;
		mySize = 0;
	}

	template<class T>
	BSTSet<T>::~BSTSet()
	{
		while (mySize > 0)
		{
			Remove(myRoot->myValue);
		}
	}

	template<class T>
	bool BSTSet<T>::HasElement(const T& aValue)
	{
		if (myRoot != nullptr)
		{
			return HasElement(myRoot, aValue);
		}

		return false;
	}

	template<class T>
	void BSTSet<T>::Insert(const T& aValue)
	{
		Insert(myRoot, aValue);
	}

	template<class T>
	void BSTSet<T>::Remove(const T& aValue)
	{
		TreeNode<T>* node = myRoot;
		TreeNode<T>* previous = nullptr;

		while (node != nullptr)
		{
			if (node->myValue == aValue)
			{
				break;
			}

			previous = node;

			if (aValue < node->myValue)
			{
				node = node->myLeft;
			}
			else
			{
				node = node->myRight;
			}
		}

		if (node != nullptr && node->myValue == aValue)
		{
			if (node == myRoot)
			{
				DeleteAndMerge(myRoot);
			}
			else if (previous->myLeft == node)
			{
				DeleteAndMerge(previous->myLeft);
			}
			else
			{
				DeleteAndMerge(previous->myRight);
			}
		}
		else if (myRoot != nullptr)
		{
			// Element is not in the tree
		}
		else
		{
			// Tree empty
		}
	}

	template<class T>
	int BSTSet<T>::GetSize()
	{
		return mySize;
	}

	template<class T>
	bool BSTSet<T>::HasElement(TreeNode<T>* aRoot, const T& aValue)
	{
		if (aValue < aRoot->myValue)
		{
			if (aRoot->myLeft == nullptr)
			{
				return false;
			}

			return HasElement(aRoot->myLeft, aValue);
		}

		if (aRoot->myValue < aValue)
		{
			if (aRoot->myRight == nullptr)
			{
				return false;
			}

			return HasElement(aRoot->myRight, aValue);
		}

		return true;
	}

	template<class T>
	void BSTSet<T>::Insert(TreeNode<T>*& aRoot, const T& aValue)
	{
		if (aRoot == nullptr)
		{
			aRoot = new TreeNode<T>();
			aRoot->myValue = aValue;
			mySize++;
		}
		else
		{
			if (aValue < aRoot->myValue)
			{
				Insert(aRoot->myLeft, aValue);
			}
			else
			{
				if (aRoot->myValue < aValue)
				{
					Insert(aRoot->myRight, aValue);
				}
			}
		}
	}

	template<class T>
	void BSTSet<T>::DeleteAndMerge(TreeNode<T>*& aNode)
	{
		TreeNode<T>* temp = aNode;

		if (aNode != nullptr)
		{
			if (aNode->myLeft == nullptr)
			{
				aNode = aNode->myRight;
			}
			else if (aNode->myRight == nullptr)
			{
				aNode = aNode->myLeft;
			}
			else
			{
				temp = aNode->myLeft;

				while (temp->myRight != nullptr)
				{
					temp = temp->myRight;
				}

				temp->myRight = aNode->myRight;

				temp = aNode;
				aNode = aNode->myLeft;
			}

			mySize--;
			delete temp;
		}
	}
}
