#include "FFpch.h"
#include "IncludeHandler.h"
#include <Firefly/Core/Core.h>

namespace Firefly
{
	HRESULT __stdcall IncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		std::filesystem::path finalPath;
		switch (IncludeType)
		{
		case D3D_INCLUDE_TYPE::D3D_INCLUDE_LOCAL: // This is #include "FILE.h"
			finalPath = myShaderDir + "\\" + pFileName;
			break;
		case D3D_INCLUDE_TYPE::D3D_INCLUDE_SYSTEM: // This is #include <FILE.h>
			finalPath = mySystemDir + "\\" + pFileName;
			break; 
		default:
			FF_ASSERT(false);
			break;
		}
		std::ifstream fin(finalPath, std::ios::ate | std::ios::binary);
		assert(fin.good() && L"No file exists at include path");
		const auto fileSize = static_cast<size_t>(fin.tellg());

		char* buffer = new char[fileSize];

		fin.seekg(0);
		fin.read(buffer, fileSize);
		fin.close();
		if (buffer)
		{
			*ppData = buffer;
			*pBytes = fileSize;
		}
		else
		{
			*ppData = nullptr;
			*pBytes = 0;
		}
		return S_OK;
	}
	HRESULT __stdcall IncludeHandler::Close(LPCVOID pData)
	{
		char* buf = (char*)pData;
		delete[] buf;
		return S_OK;
	}
}