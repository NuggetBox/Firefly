#pragma once
#include <unordered_map>
#include "Firefly/Core/Core.h"
#include "Utils/Math/Vector3.hpp"
struct PostProcessComponent;

namespace Firefly
{
	struct VignetteSettings
	{
		Utils::Vec3 color;
		float Intensity;
		float size;
	};

	class PostprocessManager
	{
	public:
		[[nodiscard]] static size_t AddPostProcess(Ptr<PostProcessComponent> aInfo);
		static void RemovePostProcess(size_t id);

		static void FindPostProcessValue();
		static void OverrideVignette(VignetteSettings& aSettings);

	private:
		static void LerpAndSubmit(Ref<PostProcessComponent> aFirst, Ref<PostProcessComponent> aSecond, float aFactor);

		static inline bool myOverrideVignette = false;
		static inline VignetteSettings myVignetteSettings;
		static inline size_t myID = 0;
		static inline std::unordered_map<size_t, Ptr<PostProcessComponent>> myPostprocessMap;
	};
}
