#pragma once
#include <d3d11_1.h>
#include <wrl.h>

namespace Firefly
{
	enum class Value
	{
		Float4 = 2,
		UInt4 = 3,

		Float3 = 6,
		UInt3 = 7,

		Float2 = 16,
		UInt2 = 17,

		Float = 41,
		UInt = 42,
		Color4 = 4,
		UInt4X4 = UInt4,
		Color3 = 5,
		UInt3X3 = UInt3,
		Bool = 99999
	};

	inline size_t SizeOfReflectedValue(Value& aReflectedValue)
	{
		size_t size = 0;
		switch (aReflectedValue)
		{
		case Firefly::Value::Bool:
		{
			size = 4; // size of bool on the gpu.
			break;
		}
		case Firefly::Value::Float:
		{
			size = sizeof(float);
			break;
		}
		case Firefly::Value::Float2:
		{
			size = sizeof(float) * 2;
			break;
		}
		case Firefly::Value::Float3:
		{
			size = sizeof(float) * 3;
			break;
		}
		case Firefly::Value::Float4:
		{
			size = sizeof(float) * 4;
			break;
		}
		case Firefly::Value::Color3:
		{
			size = sizeof(float) * 3;
			break;
		}
		case Firefly::Value::Color4:
		{
			size = sizeof(float) * 4;
			break;
		}
		case Firefly::Value::UInt:
		{
			size = sizeof(int32_t);
			break;
		}

		default:
			break;
		}

		return size;
	}

	enum class ImageFormat
	{
		RGBA32F = 2,
		RGBA32U = 3,
		RGBA32S = 4,
		RGB32F = 6,
		RGB32U = 7,
		RGB32S = 8,
		RGBA16F = 10,
		RGBA16UN = 11,
		RGBA16UI = 12,
		RGBA16SN = 13,
		RGBA16SI = 14,
		RG32F = 16,
		RG32U = 17,
		RG32S = 18,
		R8G8B8A8UN = 28,
		Depth32 = 39,
		R32UI = DXGI_FORMAT_R32_UINT,
		R32F = DXGI_FORMAT_R32_FLOAT,
		RG32UI = DXGI_FORMAT_R32G32_UINT,
		Depth16 = 53,
		Depth24 = 45,
		R8UN = 61,
	};

	template<typename T>
	using WinRef = Microsoft::WRL::ComPtr<T>;

	inline DXGI_FORMAT FormatToDXFormat(const ImageFormat& aFormat)
	{
		return static_cast<DXGI_FORMAT>(aFormat);
	}
	inline ImageFormat DXFormatToFormat(const DXGI_FORMAT& aFormat)
	{
		return static_cast<ImageFormat>(aFormat);
	}

	inline bool IsDepth(ImageFormat& aFormat)
	{
		if (aFormat == ImageFormat::Depth24 || aFormat == ImageFormat::Depth32 || aFormat == ImageFormat::Depth16)
		{
			return true;
		}
		return false;
	}
}
