#pragma once
#include <filesystem>

struct ID3D11DeviceContext;

namespace Firefly
{
	class PipelineBase
	{
	public:
		virtual void Bind(ID3D11DeviceContext* aContext) = 0;
		virtual void UnBind(ID3D11DeviceContext* aContext) = 0;
		const size_t GetHash() const { return myHash; }
		virtual void Cache(const std::filesystem::path& aFilepath) = 0;
	protected:
		std::string_view myPath;
		size_t myHash{};
	};
}
