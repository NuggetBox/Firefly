#include "FFpch.h"
#include "Shader.h"

#include <d3dcompiler.h>
#include <Firefly/Utilities/BinaryFileUtils.h>
#include "Firefly/Application/Application.h"
#include "Firefly/Event/EngineEvents.h"
#include "Firefly/Rendering/GraphicsContext.h"
#include "Firefly/Rendering/Shader/ShaderLibrary.h"

#include <filesystem>

const std::string CompiledShaderEntry = "FireflyEngine/Shaders-bin/";
#ifdef FF_DEBUG
#define COMPILE_FLAG D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG
#else
#define COMPILE_FLAG 0
#endif
namespace Firefly
{
	Shader::Shader(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent, std::initializer_list<ShaderDefine> aDefines) : myIncludeHandler(aPath.parent_path().string().c_str(), "FireflyEngine/Shaders")
	{
		std::filesystem::path binaryPath = CompiledShaderEntry;
		myType = aType;
#ifdef FF_DEBUG
		binaryPath += "Debug/";
#else
		binaryPath += "Optimized/";
#endif // FF_DEBUG

		binaryPath += aPath.stem();
		binaryPath += ".ffcs";
		myHasDefines = aDefines.size();
		for (auto& def : aDefines)
		{
			auto& define = myShaderDefines.emplace_back(def.Name.c_str(), def.Value.c_str());
		}

		if (std::filesystem::exists(binaryPath))
		{
			CreateBlob(binaryPath, aType, aSendEvent);
		}
		else
		{
			CreateBlob(aPath, aType, aSendEvent);
			if (myBuffer)
			{
				DumpBlob(binaryPath);
			}
		}

		myPath = aPath;
		if (myBuffer)
		{

			switch (myType)
			{
			case ShaderType::Vertex:
				FF_DX_ASSERT(GraphicsContext::Device()->CreateVertexShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), NULL, reinterpret_cast<ID3D11VertexShader**>(myShader.GetAddressOf())));
				break;
			case ShaderType::Pixel:
				FF_DX_ASSERT(GraphicsContext::Device()->CreatePixelShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), NULL, reinterpret_cast<ID3D11PixelShader**>(myShader.GetAddressOf())));
				break;
			case ShaderType::Geometry:
				FF_DX_ASSERT(GraphicsContext::Device()->CreateGeometryShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), NULL, reinterpret_cast<ID3D11GeometryShader**>(myShader.GetAddressOf())));
				break;
			case ShaderType::Compute:
				FF_DX_ASSERT(GraphicsContext::Device()->CreateComputeShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), NULL, reinterpret_cast<ID3D11ComputeShader**>(myShader.GetAddressOf())));
				break;
			}
			ReflectShader();
		}
	}

	Shader::~Shader()
	{
	}

	void Shader::Bind(ID3D11DeviceContext* aContext)
	{
		std::scoped_lock lock(myShaderMutex);
		switch (myType)
		{
		case ShaderType::Vertex:
			aContext->IASetInputLayout(myInputLayout.Get());
			aContext->VSSetShader(reinterpret_cast<ID3D11VertexShader*>(myShader.Get()), nullptr, 0);
			break;
		case ShaderType::Pixel:
			aContext->PSSetShader(reinterpret_cast<ID3D11PixelShader*>(myShader.Get()), nullptr, 0);
			break;
		case ShaderType::Geometry:
			aContext->GSSetShader(reinterpret_cast<ID3D11GeometryShader*>(myShader.Get()), nullptr, 0);
			break;
		case ShaderType::Compute:
			aContext->CSSetShader(reinterpret_cast<ID3D11ComputeShader*>(myShader.Get()), nullptr, 0);
			break;
		}
	}
	void Shader::Recompile(bool aSendEvent)
	{
		LOGINFO("Recompiling {}...", myPath.filename().string());
		std::filesystem::path binaryPath = CompiledShaderEntry;
#ifdef FF_DEBUG
		binaryPath += "Debug/";
#else
		binaryPath += "Optimized/";
#endif // FF_DEBUG
		binaryPath += myPath.stem();
		binaryPath += ".ffcs";
		myHasCompiled = false;

		CreateBlob(myPath, myType, aSendEvent);
		if (myBuffer)
		{

			DumpBlob(binaryPath);

			ShaderLibrary::PushBuildFunction([this]
				{
					switch (myType)
					{
					case ShaderType::Vertex:
						FF_DX_ASSERT(GraphicsContext::Device()->CreateVertexShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), nullptr, reinterpret_cast<ID3D11VertexShader**>(myShader.GetAddressOf())));
						break;
					case ShaderType::Pixel:
						FF_DX_ASSERT(GraphicsContext::Device()->CreatePixelShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), NULL, reinterpret_cast<ID3D11PixelShader**>(myShader.GetAddressOf())));
						break;
					case ShaderType::Geometry:
						FF_DX_ASSERT(GraphicsContext::Device()->CreateGeometryShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), nullptr, reinterpret_cast<ID3D11GeometryShader**>(myShader.GetAddressOf())));
						break;
					case ShaderType::Compute:
						FF_DX_ASSERT(GraphicsContext::Device()->CreateComputeShader(myBuffer.Get()->GetBufferPointer(), static_cast<uint32_t>(myBuffer.Get()->GetBufferSize()), nullptr, reinterpret_cast<ID3D11ComputeShader**>(myShader.GetAddressOf())));
						break;
					}
					myBoundResources.clear();
					ReflectShader();
					myHasCompiled = true;
				});
		}
		myHasCompiled = true;
	}
	void Shader::Unbind(ID3D11DeviceContext* aContext)
	{
		switch (myType)
		{
		case ShaderType::Vertex:
			aContext->VSSetShader(nullptr, nullptr, 0);
			break;
		case ShaderType::Pixel:
			aContext->PSSetShader(nullptr, nullptr, 0);
			break;
		case ShaderType::Geometry:
			aContext->GSSetShader(nullptr, nullptr, 0);
			break;
		case ShaderType::Compute:
			aContext->CSSetShader(nullptr, nullptr, 0);
			break;
		}
	}
	bool Shader::IsValid()
	{
		return myShader;
	}
	Ref<Shader> Shader::Create(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent, std::initializer_list<ShaderDefine> aDefines)
	{
		if (!myShaderMap.contains(aPath.filename()))
		{
			myShaderMap[aPath.filename()] = CreateRef<Shader>(aPath, aType, aSendEvent, aDefines);
		}
		return myShaderMap[aPath.filename()];
	}
	void Shader::CreateBlob(const std::filesystem::path& aPath, ShaderType aType, bool aSendEvent)
	{
		if (myShaderDefines.empty())
		{
			myShaderDefines.emplace_back();
		}
		const D3D_SHADER_MACRO defines[]
		{
			myShaderDefines[0].Name, myShaderDefines[0].Definition, NULL, NULL
		};
		if (aPath.extension() == ".hlsl")
		{
			ComPtr<ID3D10Blob> compileBlob;
			ID3D10Blob* errorMsg = nullptr;
			switch (myType)
			{
			case ShaderType::Vertex:
				D3DCompileFromFile(aPath.c_str(), myHasDefines ? defines : nullptr, &myIncludeHandler, "main", "vs_5_0", COMPILE_FLAG, COMPILE_FLAG, compileBlob.GetAddressOf(), &errorMsg);
				if (errorMsg != nullptr)
				{

					char* error = (char*)errorMsg->GetBufferPointer();
					LOGERROR("{}", error);
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(false, (char*)errorMsg->GetBufferPointer());
						Application::Get().OnEvent(e);
					}
				}
				else
				{
					LOGINFO("Successful compilation!");
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(true, nullptr);
						Application::Get().OnEvent(e);
					}
				}
				break;
			case ShaderType::Pixel:
				(D3DCompileFromFile(aPath.c_str(), myHasDefines ? defines : nullptr, &myIncludeHandler, "main", "ps_5_0", COMPILE_FLAG, COMPILE_FLAG, compileBlob.GetAddressOf(), &errorMsg));
				if (errorMsg != nullptr)
				{

					char* error = (char*)errorMsg->GetBufferPointer();
					LOGERROR("{}", error);
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(false, (char*)errorMsg->GetBufferPointer());
						Application::Get().OnEvent(e);
					}
				}
				else
				{
					LOGINFO("Successful compilation!");
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(true, nullptr);
						Application::Get().OnEvent(e);
					}
				}
				break;
			case ShaderType::Geometry:
				(D3DCompileFromFile(aPath.c_str(), myHasDefines ? defines : nullptr, &myIncludeHandler, "main", "gs_5_0", COMPILE_FLAG, COMPILE_FLAG, compileBlob.GetAddressOf(), &errorMsg));
				if (errorMsg != nullptr)
				{

					char* error = (char*)errorMsg->GetBufferPointer();
					LOGERROR("{}", error);
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(false, (char*)errorMsg->GetBufferPointer());
						Application::Get().OnEvent(e);
					}
				}
				else
				{
					LOGINFO("Successful compilation!");
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(true, nullptr);
						Application::Get().OnEvent(e);
					}
				}
				break;
			case ShaderType::Compute:
				(D3DCompileFromFile(aPath.c_str(), myHasDefines ? defines : nullptr, &myIncludeHandler, "main", "cs_5_0", COMPILE_FLAG, COMPILE_FLAG, compileBlob.GetAddressOf(), &errorMsg));
				if (errorMsg != nullptr)
				{

					char* error = (char*)errorMsg->GetBufferPointer();
					LOGERROR("{}", error);
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(false, (char*)errorMsg->GetBufferPointer());
						Application::Get().OnEvent(e);
					}
				}
				else
				{
					LOGINFO("Successful compilation!");
					if (aSendEvent)
					{
						ShaderCompileErrorEvent e(true, nullptr);
						Application::Get().OnEvent(e);
					}
				}
				break;
			}
			if (compileBlob)
			{
				std::scoped_lock lock(myShaderMutex);
				myBuffer = compileBlob;
			}
		}
		else
		{
			FF_DX_ASSERT(D3DReadFileToBlob(aPath.c_str(), myBuffer.GetAddressOf()));
		}
	}
	void Shader::HandleCompileError(bool aSendEvent, ID3D10Blob* errorMsg)
	{

		if (errorMsg != nullptr)
		{

			char* error = (char*)errorMsg->GetBufferPointer();
			LOGERROR("{}", error);
			if (aSendEvent)
			{
				ShaderCompileErrorEvent e(false, (char*)errorMsg->GetBufferPointer());
				Application::Get().OnEvent(e);
			}
		}
		else
		{
			LOGINFO("Successful compilation!");
			if (aSendEvent)
			{
				ShaderCompileErrorEvent e(true, nullptr);
				Application::Get().OnEvent(e);
			}
		}
	}
	void Shader::DumpBlob(const std::filesystem::path& aPath)
	{
		std::vector<uint8_t> bytes(myBuffer->GetBufferSize());
		memcpy(bytes.data(), myBuffer->GetBufferPointer(), myBuffer->GetBufferSize() * sizeof(uint8_t));
		WriteBinary(aPath, bytes);
	}
	void Shader::ReflectShader()
	{
		ID3D11ShaderReflection* pReflector = nullptr;
		FF_DX_ASSERT(D3DReflect(myBuffer->GetBufferPointer(), myBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector));
		if (pReflector == nullptr)
		{
			return;
		}
		D3D11_SHADER_DESC decs;
		pReflector->GetDesc(&decs);
		if (myType == ShaderType::Vertex)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
			for (UINT i = 0; i < decs.InputParameters; i++)
			{
				D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
				pReflector->GetInputParameterDesc(i, &paramDesc);
				std::string f3C(paramDesc.SemanticName);
				f3C = f3C.substr(0, 3);
				if (f3C == "SV_")
				{
					continue;
				}
				// Fill out input element desc
				D3D11_INPUT_ELEMENT_DESC elementDesc;
				elementDesc.SemanticName = paramDesc.SemanticName;
				elementDesc.SemanticIndex = paramDesc.SemanticIndex;
				elementDesc.InputSlot = 0;
				elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elementDesc.InstanceDataStepRate = 0;
				// determine DXGI format
				if (paramDesc.Mask == 1)
				{
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
						elementDesc.Format = DXGI_FORMAT_R32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
						elementDesc.Format = DXGI_FORMAT_R32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
						elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
				}
				else if (paramDesc.Mask <= 3)
				{
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
						elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
				}
				else if (paramDesc.Mask <= 7)
				{
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				}
				else if (paramDesc.Mask <= 15)
				{
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
						elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				}

				// Save element desc
				inputLayoutDesc.push_back(elementDesc);
			}
			if (!inputLayoutDesc.empty())
				FF_DX_ASSERT(GraphicsContext::Device()->CreateInputLayout(
					inputLayoutDesc.data(),
					static_cast<UINT>(inputLayoutDesc.size()),
					myBuffer->GetBufferPointer(),
					myBuffer->GetBufferSize(),
					myInputLayout.GetAddressOf()));
		}


		std::vector<D3D11_SHADER_INPUT_BIND_DESC> bindDescs;
		for (size_t i = 0; i < decs.BoundResources; ++i)
		{

			auto& de = bindDescs.emplace_back();
			pReflector->GetResourceBindingDesc(i, &de);
			if ((BoundType)de.Type == BoundType::Texture)
			{

				std::string str(de.Name);
				if (str.size() > 7)
				{
					auto ending = str.find("_Engine");
					if (ending != std::string::npos)
					{
						continue;
					}
				}
				auto& boundR = myBoundResources.emplace_back();
				boundR.name = de.Name;
				boundR.bindPoint = de.BindPoint;
				boundR.type = (BoundType)de.Type;

			}
			else
			{
				auto& boundR = myBoundResources.emplace_back();
				boundR.name = de.Name;
				boundR.bindPoint = de.BindPoint;
				boundR.type = (BoundType)de.Type;
			}
		}
		for (auto& br : myBoundResources)
		{
			if (br.name == "MaterialInfo")
			{
				auto cbuff = pReflector->GetConstantBufferByName("MaterialInfo");
				D3D11_SHADER_BUFFER_DESC sBuf;
				auto hr = cbuff->GetDesc(&sBuf);
				if (FAILED(hr))
				{
					return;
				}
				for (size_t i = 0; i < sBuf.Variables; ++i)
				{
					auto var = cbuff->GetVariableByIndex(i);
					D3D11_SHADER_VARIABLE_DESC sVar;
					var->GetDesc(&sVar);
					auto type = var->GetType();
					D3D11_SHADER_TYPE_DESC tVar;
					type->GetDesc(&tVar);

					std::string str(sVar.Name);

					bool isColor = false;
					bool isSlider = false;

					if (str.size() > 6)
					{
						auto ending = str.substr(str.size() - 6);
						if (ending == "_Color")
						{
							isColor = true;
						}
					}

					if (str.size() > 7)
					{
						auto ending = str.substr(str.size() - 7);
						if (ending == "_Slider")
						{
							isSlider = true;
						}
					}

					br.size += sVar.Size;
					switch (tVar.Type)
					{
					case D3D_SVT_BOOL:
					{
						if (tVar.Columns == 1)
						{

							br.varibles.emplace_back(sVar.Name, Value::Bool);
						}
						break;
					}
					case D3D_SVT_INT:
					{
						if (tVar.Columns == 1)
						{
							br.varibles.emplace_back(sVar.Name, Value::UInt);
						}
						else if (tVar.Columns == 2)
						{
							br.varibles.emplace_back(sVar.Name, Value::UInt2);
						}
						else if (tVar.Columns == 3)
						{
							br.varibles.emplace_back(sVar.Name, Value::UInt3);
						}
						else if (tVar.Columns == 4)
						{
							br.varibles.emplace_back(sVar.Name, Value::UInt4);
						}
						break;
					}
					case D3D_SVT_FLOAT:
					{
						if (tVar.Columns == 1)
						{
							br.varibles.emplace_back(sVar.Name, Value::Float);
						}
						else if (tVar.Columns == 2)
						{
							br.varibles.emplace_back(sVar.Name, Value::Float2);
						}
						else if (tVar.Columns == 3)
						{
							br.varibles.emplace_back(sVar.Name, isColor ? Value::Color3 : Value::Float3);
						}
						else if (tVar.Columns == 4)
						{
							br.varibles.emplace_back(sVar.Name, isColor ? Value::Color4 : Value::Float4);
						}
						break;
					}

					default:
						break;
					}

					void* defaultValue = nullptr;
					if (sVar.DefaultValue)
					{
						defaultValue = sVar.DefaultValue;
					}
					if (isSlider)
					{
						br.varibles[i].Type = VarType::Slider; 
					}
					br.varibles[i].DefaultData = defaultValue;
				}
				break;
			}
		}

	}
}
