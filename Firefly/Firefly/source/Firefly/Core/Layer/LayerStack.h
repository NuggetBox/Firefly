#pragma once
#include "Layer.h"
#include <vector>

namespace Firefly
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void Push(Layer* aLayer);
		void PushOverlay(Layer* aLayer);
		void Pop(Layer* aLayer);
		void PopOverlay(Layer* aLayer);

		std::vector<Layer*>::iterator begin() { return myLayerStack.begin(); }
		std::vector<Layer*>::iterator end() { return myLayerStack.end(); }

	private:
		std::vector<Layer*> myLayerStack;
		std::vector<Layer*>::iterator myLayerStackIterator;
	};
}