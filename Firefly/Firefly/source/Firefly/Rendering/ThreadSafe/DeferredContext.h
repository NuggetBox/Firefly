#pragma once
#include "Firefly/Core/DXHelpers.h"
struct ID3D11DeviceContext;
struct ID3DUserDefinedAnnotation;
namespace Firefly
{
	class DeferredContext
	{
	public:
		DeferredContext();

		ID3D11DeviceContext* Get();

		// adds a GPU event. Makes it so we can have custom events in something like RenderDoc.
		void BeginEvent(std::string_view aName);
		void EndEvent();
		~DeferredContext();
	private:
		WinRef<ID3D11DeviceContext> myContext;
		WinRef<ID3DUserDefinedAnnotation> myAnnotaion;
	};
}
