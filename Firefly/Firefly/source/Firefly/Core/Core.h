#pragma once
#include <memory>
#include <cassert>

#include "Log/DebugLogger.h"

#define FF_MAX_DIRLIGHTS 1
#define FF_MAX_POINTLIGHTS 128
#define FF_MAX_POINTLIGHT_SHADOWS 4
#define FF_MAX_SPOTLIGHTS 64
#define FF_MAX_SPOTLIGHT_SHADOWS 4

#define FF_SAFE_DESTROY(x) {if(x != nullptr){delete x; x = nullptr;}}
#define FF_ASSERT(x) { if ((x) == NULL) LOGERROR("assert called"); assert(x); } 
#define FF_DX_ASSERT(x) {HRESULT result = (x); if (result != S_OK) LOGERROR("Error HRESULT recieved: " + GraphicsContext::HResultToString(result)); assert((result) == S_OK);}

namespace Firefly
{
	enum class ShadowResolutions : uint32_t
	{
		res512,
		res1024,
		res2048,
		res4098,
		res8192,
		res16384,
		COUNT
	};

	inline uint32_t GetResFromEnum(ShadowResolutions aRes)
	{
		switch (aRes)
		{
		case Firefly::ShadowResolutions::res512:
			return 512u;
			break;
		case Firefly::ShadowResolutions::res1024:
			return 1024u;
			break;
		case Firefly::ShadowResolutions::res2048:
			return 2048u;
			break;
		case Firefly::ShadowResolutions::res4098:
			return 4098u;
			break;
		case Firefly::ShadowResolutions::res8192:
			return 8192u;
			break;
		case Firefly::ShadowResolutions::res16384:
			return 16384u;
			break;
		default:
			return 0;
			break;
		}
	}

}

inline bool IsPowerOfTwo(size_t aValue)
{
	return (aValue & (aValue - 1)) == 0;
}

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename ... Args>
Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T> 
using Ptr = std::weak_ptr<T>;

template<typename T, typename ... Args>
Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
