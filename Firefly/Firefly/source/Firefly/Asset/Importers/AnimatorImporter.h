#pragma once
#include "Firefly/Core/Core.h"


namespace Firefly
{
	class Animator;
	class AnimatorImporter
	{
	public:
		AnimatorImporter() = default;
		~AnimatorImporter() = default;

		Ref<Animator> ImportAnimator(const std::filesystem::path& aPath);
		bool ImportAnimator(Ref<Animator> aAnimator);
		uint64_t ImportID(const std::filesystem::path& aPath);

	private:

	};
}