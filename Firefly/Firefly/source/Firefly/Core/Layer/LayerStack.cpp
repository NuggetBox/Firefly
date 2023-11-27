#include "FFpch.h"
#include "LayerStack.h"
#include <sstream>
namespace Firefly
{
	LayerStack::LayerStack()
	{
		myLayerStackIterator = myLayerStack.begin();
	}

	LayerStack::~LayerStack()
	{
		for (auto layer : myLayerStack)
		{
			layer->OnDetach();
			delete layer;
			layer = nullptr;
		}
	}

	void LayerStack::Push(Layer* aLayer)
	{
		myLayerStackIterator = myLayerStack.emplace(myLayerStackIterator, aLayer);
		aLayer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer* aLayer)
	{
		myLayerStack.emplace_back(aLayer);
		aLayer->OnAttach();
	}

	void LayerStack::Pop(Layer* aLayer)
	{
		const auto it = std::ranges::find(myLayerStack, aLayer);
		if (it != myLayerStack.end())
		{
			myLayerStack.erase(it);
			aLayer->OnDetach();
			--myLayerStackIterator;
		}
	}

	void LayerStack::PopOverlay(Layer* aLayer)
	{
		const auto it = std::ranges::find(myLayerStack, aLayer);
		if (it != myLayerStack.end())
		{
			myLayerStack.erase(it);
			aLayer->OnDetach();
		}
	}
}