#pragma once
#include "Firefly/Core/Core.h"
#include "Firefly/Core/DXHelpers.h"

#include <filesystem>
#include <Firefly/Rendering/Shader/IncludeHandler.h>

namespace Firefly
{
	namespace wrl = Microsoft::WRL;

	enum class ShaderType : uint32_t
	{ 
		Vertex = (1 << 0),
		Pixel = (1 << 1),
		Geometry = (1 << 2),
		Hull = (1 << 3),
		Domain = (1 << 4),
		Compute = (1 << 5)
	};

	inline ShaderType StringToShaderStage(const std::string& str)
	{
		static std::unordered_map<std::string, ShaderType> map{};
		if (map.empty()) 
		{
			map["Vertex"] = ShaderType::Vertex;
			map["Pixel"] = ShaderType::Pixel;
			map["Geometry"] = ShaderType::Geometry;
			map["Hull"] = ShaderType::Hull;
			map["Domain"] = ShaderType::Domain;
			map["Compute"] = ShaderType::Compute;
		}
		return map.at(str);
	}

	inline std::string ShaderStageToString(ShaderType value)
	{
		switch (value)
		{
		case ShaderType::Vertex: return "Vertex";
		case ShaderType::Pixel: return "Pixel";
		case ShaderType::Geometry: return "Geometry";
		case ShaderType::Hull: return "Hull";
		case ShaderType::Domain: return "Domain";
		case ShaderType::Compute: return "Compute";
		default: return "INVALID_SHADER_TYPE";
		}
	}

	constexpr ShaderType operator|(ShaderType a, ShaderType b)
	{
		return static_cast<ShaderType>(static_cast<std::underlying_type_t<ShaderType>>(a) | static_cast<std::underlying_type_t<ShaderType>>(b));
	}

	constexpr bool operator&(ShaderType a, ShaderType b)
	{
		return static_cast<bool>(static_cast<std::underlying_type_t<ShaderType>>(a) & static_cast<std::underlying_type_t<ShaderType>>(b));
	}

	constexpr ShaderType operator^(ShaderType a, ShaderType b)
	{
		return static_cast<ShaderType>(static_cast<std::underlying_type_t<ShaderType>>(a) ^ static_cast<std::underlying_type_t<ShaderType>>(b));
	}

	constexpr ShaderType& operator|=(ShaderType& a, ShaderType b)
	{
		return a = a | b;
	}

	

	constexpr ShaderType& operator^=(ShaderType& a, ShaderType b)
	{
		return a = a ^ b;
	}

	constexpr ShaderType operator~(ShaderType a)
	{
		return static_cast<ShaderType>(~static_cast<std::underlying_type_t<ShaderType>>(a));
	}
	





	enum class BoundType
	{
		ContantBuffer = D3D_SHADER_INPUT_TYPE::D3D10_SIT_CBUFFER,
		Texture = D3D_SHADER_INPUT_TYPE::D3D10_SIT_TEXTURE,
		Sampler = D3D_SHADER_INPUT_TYPE::D3D10_SIT_SAMPLER,
	};

	enum class VarType
	{
		Drag,
		Slider,
	};

	struct ReflectedValue
	{
		ReflectedValue() { Name = ""; VariableType = Value::Bool; DefaultData = nullptr; Type = VarType::Drag; }
		ReflectedValue(std::string aName, Value aValue)
		{
			Name = aName;
			VariableType = aValue;
			DefaultData = nullptr;
			Type = VarType::Drag;
		}
		ReflectedValue(std::string aName, Value aValue, void* aDefaultValue, VarType aType)
		{
			Name = aName;
			VariableType = aValue;
			DefaultData = aDefaultValue;
			Type = aType;
		}
		ReflectedValue(const ReflectedValue& other)
		{
			Name = other.Name;
			VariableType = other.VariableType;
			DefaultData = other.DefaultData;
			Type = other.Type;
		}
		std::string Name = "";
		Value VariableType = Value::Float;
		void* DefaultData = nullptr;
		VarType Type = VarType::Drag;
	};

	struct BoundResource
	{
		std::string name;
		uint32_t bindPoint;
		BoundType type;
		std::vector<ReflectedValue> varibles;
		size_t size;
	};

	struct ShaderDefine
	{
		std::string Name;
		std::string Value;
	};

	class Shader
	{
	public:
		Shader(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent, std::initializer_list<ShaderDefine> aDefines = {});
		~Shader();
		void Bind(ID3D11DeviceContext* aContext);
		void Recompile(bool aSendEvent = true);
		void Unbind(ID3D11DeviceContext* aContext);
		bool IsValid();
		std::filesystem::path& GetPath() { return myPath; };
		ShaderType GetShaderType() { return myType; }
		bool IsCompiled() { return myHasCompiled; }
		void ReflectShader();

		std::vector<BoundResource>& GetBoundResources() { return myBoundResources; }
		static Ref<Shader> Create(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent = true, std::initializer_list<ShaderDefine> aDefines = {});
	private:
		void CreateBlob(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent = true);
		void HandleCompileError(bool aSendEvent, ID3D10Blob* errorMsg);
		void DumpBlob(const std::filesystem::path& aPath);
		std::filesystem::path myPath;
		std::vector<BoundResource> myBoundResources;
		wrl::ComPtr<ID3D11DeviceChild> myShader;
		wrl::ComPtr<ID3D11InputLayout> myInputLayout;
		wrl::ComPtr<ID3D10Blob> myBuffer;
		std::vector<D3D_SHADER_MACRO> myShaderDefines;
		bool myHasDefines = false;
		ShaderType myType;
		std::mutex myShaderMutex;

		IncludeHandler myIncludeHandler;
		std::atomic<bool> myHasCompiled = true;
		static inline std::unordered_map<std::filesystem::path, Ref<Shader>> myShaderMap;

	};
}