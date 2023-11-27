#include "FFpch.h"
#include "ShaderLibrary.h"
#include <Firefly/Rendering/RenderCommands.h>
#include <execution>
#include <future>

// adds logic to handle a GPU hang by crashing the program before it can hang the gpu. Even if this is good to allways have it should be disabled when not debugging.
#define GPU_HANG_GUARD 1

namespace Firefly
{
	std::string ShaderLibrary::GetPathToTemplate(uint32_t aIndex, ShaderType aType)
	{
		static std::unordered_map<ShaderType, std::map<uint32_t, std::string>> lut;
		// index 0 = Graphics, 1 = postProcess, 2 = Decal, 3 = Particle

		lut[ShaderType::Vertex][0] = "Editor/Templates/FireflyVertexShaderTemplate.txt";
		lut[ShaderType::Vertex][1] = "Editor/Templates/FireflyPostProcessVertexShaderTemplate.txt";
		lut[ShaderType::Vertex][2] = "Editor/Templates/FireflyDecalVertexShaderTemplate.txt";
		lut[ShaderType::Vertex][3] = "Editor/Templates/FireflyParticleVertexTemplate.txt";

		lut[ShaderType::Pixel][0] = "Editor/Templates/FireflyPixelShaderTemplate.txt";
		lut[ShaderType::Pixel][1] = "Editor/Templates/FireflyPostProcessPixelShaderTemplate.txt";
		lut[ShaderType::Pixel][2] = "Editor/Templates/FireflyDecalPixelShaderTemplate.txt";
		lut[ShaderType::Pixel][3] = "Editor/Templates/FireflyParticlePixelTemplate.txt";

		lut[ShaderType::Geometry][0] = "Editor/Templates/FireflyGeometryShaderTemplate.txt";
		lut[ShaderType::Geometry][1] = "Editor/Templates/FireflyGeometryShaderTemplate.txt";
		lut[ShaderType::Geometry][2] = "Editor/Templates/FireflyGeometryShaderTemplate.txt";
		lut[ShaderType::Geometry][3] = "Editor/Templates/FireflyParticleGeometryTemplate.txt";

		return lut[aType][aIndex];
	}
	void ShaderLibrary::Add(const std::string& aKey, std::initializer_list<Ref<Shader>> someShaders)
	{
		myMap[aKey] = someShaders;
		myIsHash[aKey] = false;
	}

	void ShaderLibrary::Add(const std::string& aKey, const std::vector<Ref<Shader>>& someShaders)
	{
		myMap[aKey] = someShaders;
		myIsHash[aKey] = false;
	}

	bool ShaderLibrary::KeyHasCompiled(const std::string& aKey)
	{
		if (myMap.contains(aKey) == false)
		{
			return false;
		}
		uint32_t hasCompiled = 0;
		for (auto& shader : myMap[aKey])
		{
			if (shader->IsCompiled())
			{
				hasCompiled++;
			}
		}

		return hasCompiled >= myMap[aKey].size();
	}

	bool ShaderLibrary::KeyHasCompiled(size_t aKey)
	{
		return KeyHasCompiled(std::to_string(aKey));
	}

	Ref<Shader> ShaderLibrary::GetShader(const std::string& aKey, const ShaderType& aType)
	{
		if (myMap.contains(aKey) == false)
		{
			return nullptr;
		}
		for (auto& shader : myMap[aKey])
		{
			if (shader->GetShaderType() == aType)
			{
				return shader;
			}
		}
		return nullptr;
	}

	std::string ShaderLibrary::GetKeyFromPath(const std::filesystem::path& aPath)
	{
		for (auto& shader : myMap)
		{
			for (auto& s : shader.second)
			{
				if (aPath == s->GetPath())
				{
					return shader.first;
				}
			}
		}
		return "";
	}

	void ShaderLibrary::ReCompileAllShaders(uint32_t* aNumberOfCompiledShaders, uint32_t* aTotalNumberOfShaders, std::string* aCurrentCompilingString)
	{
		LOGINFO("Started Recompiling");
		// destroy cache!
		*aTotalNumberOfShaders = 0;

		for (auto& shaderPack : myMap)
		{
			for (auto& shader : shaderPack.second)
			{
				(*aTotalNumberOfShaders)++;
			}
		}

		auto thread = std::thread([aNumberOfCompiledShaders, aTotalNumberOfShaders, aCurrentCompilingString] {

			for (auto& shaderPack : myMap)
			{
				for (auto& shader : shaderPack.second)
				{
					*aCurrentCompilingString = shader->GetPath().filename().string();
					shader->Recompile(false);
					(*aNumberOfCompiledShaders)++;
				}
			}

			});
		thread.detach();


		LOGINFO("Completed Recompiling");
	}
	std::vector<std::string> ShaderLibrary::GetAllShaderKeys()
	{
		std::vector<std::string> keys;
		for (auto& mapVar : myMap)
		{
			keys.emplace_back(mapVar.first);
		}
		return keys;
	}
	void ShaderLibrary::Recompile(const std::string& aKey, bool aSendEvent)
	{
		for (auto& shader : myMap[aKey])
		{
			shader->Recompile(aSendEvent);
		}
	}
	void ShaderLibrary::Bind(const std::string& aKey, ID3D11DeviceContext* aContext)
	{
		myCompileThread.notify_one();
		if (myReadyToCopy)
		{
			std::unique_lock lk(myMutex);
			myCompileThread.wait(lk, [&] {return myDone; });
			myReadyToCopy = false;
		}
		myCurrentKey = aKey;
#ifdef GPU_HANG_GUARD
		bool shouldCrash = true;
#endif // GPU_HANG_GUARD

		for (const auto& i : myMap[aKey])
		{
			globalRendererStats.shaderBinds++;
#ifdef GPU_HANG_GUARD
			if (shouldCrash)
			{
				if (i->GetShaderType() == ShaderType::Vertex)
				{
					shouldCrash = false;
				}
				if (i->GetShaderType() == ShaderType::Compute)
				{
					shouldCrash = false;
				}
			}
#endif // GPU_HANG_GUARD

			i->Bind(aContext);
		}

#ifdef GPU_HANG_GUARD
		if (shouldCrash)
		{
			LOGERROR("GPU HANG GUARD ACTIVATED: Don't worry, this crash just saved you 30 seconds of waiting on the gpu dying.");
			int* crashPtr = nullptr;
			*crashPtr = 5;
		}
#endif // GPU_HANG_GUARD
	}
	void ShaderLibrary::Remove(const std::string& aKey)
	{
		myMap.erase(aKey);
		myIsHash.erase(aKey);

	}
	void ShaderLibrary::Unbind(const std::string& aKey, ID3D11DeviceContext* aContext)
	{
		myCurrentKey = "";
		for (const auto& i : myMap[aKey])
		{
			i->Unbind(aContext);
		}
	}

	bool ShaderLibrary::IsHash(const std::string& key)
	{
		return myIsHash[key];
	}

	size_t ShaderLibrary::GetHash(const std::string& key)
	{
		std::stringstream stream(key);

		// associating a string object with a stream
		size_t output;

		// to read something from the stringstream object
		stream >> output;
		return output;
	}


	void ShaderLibrary::Add(const size_t& aHash, std::initializer_list<Ref<Shader>> someShaders)
	{
		Add(std::to_string(aHash), someShaders);
		myIsHash[std::to_string(aHash)] = true;
	}

	void ShaderLibrary::Add(const size_t& aHash, const std::vector<Ref<Shader>>& someShaders)
	{
		Add(std::to_string(aHash), someShaders);
		myIsHash[std::to_string(aHash)] = true;
	}

	Ref<Shader> ShaderLibrary::GetShader(const size_t& aHash, const ShaderType& aType)
	{
		return GetShader(std::to_string(aHash), aType);
	}

	void ShaderLibrary::Recompile(const size_t& aHash, bool aSendEvent)
	{
		Recompile(std::to_string(aHash), aSendEvent);
	}

	void ShaderLibrary::Bind(const size_t& aHash, ID3D11DeviceContext* aContext)
	{
		Bind(std::to_string(aHash), aContext);
	}

	void ShaderLibrary::Remove(const size_t& aHash)
	{
		Remove(std::to_string(aHash));
	}

	void ShaderLibrary::Unbind(const size_t& aHash, ID3D11DeviceContext* aContext)
	{
		Unbind(std::to_string(aHash), aContext);
	}
	void ShaderLibrary::BuildShaders()
	{
		std::scoped_lock lock(myPushCompiledShaderMutex);
		for (auto& func : myCompiledShaderStack)
		{
			func();
		}
		myCompiledShaderStack.clear();
	}
	void ShaderLibrary::PushBuildFunction(std::function<void()>&& aFunc)
	{
		std::scoped_lock lock(myPushCompiledShaderMutex);
		myCompiledShaderStack.emplace_back(aFunc);
	}
}