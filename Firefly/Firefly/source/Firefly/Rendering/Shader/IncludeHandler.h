#pragma once
#include <d3d11_1.h>
#include "Firefly/Core/DXHelpers.h"
namespace Firefly
{
	class IncludeHandler : public ID3DInclude
	{
	public:
		IncludeHandler(const char* shaderDir, const char* systemDir) :
			myShaderDir(shaderDir),
			mySystemDir(systemDir)
		{
		}

		HRESULT __stdcall Open(
			D3D_INCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID* ppData,
			UINT* pBytes);

		HRESULT __stdcall Close(LPCVOID pData);

	private:
		std::string myShaderDir;
		std::string mySystemDir;
	};
}
