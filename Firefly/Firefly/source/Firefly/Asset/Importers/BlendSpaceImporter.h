#pragma once
#include "Firefly/Core/Core.h"


namespace Firefly
{
	class BlendSpace;
	class BlendSpaceImporter
	{
	public:
		BlendSpaceImporter() = default;
		~BlendSpaceImporter() = default;

		Ref<BlendSpace> ImportBlendSpace(const std::filesystem::path& aPath);

		bool ImportBlendSpace(Ref<BlendSpace> aBlendSpace);

	private:

	};
}