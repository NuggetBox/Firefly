#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "Firefly/Rendering/Shader/Shader.h"
namespace Firefly
{

	class ShaderLibrary
	{
		friend class Shader;
	public:
		static std::string GetPathToTemplate(uint32_t aIndex, ShaderType aType);
		// Adds a new set of shaders to the library, identified by the given key.
		static void Add(const std::string& aKey, std::initializer_list<Ref<Shader>> someShaders);
		static void Add(const std::string& aKey, const std::vector<Ref<Shader>>& someShaders);

		// Returns the shader of the specified type from the set identified by the given key.
		static Ref<Shader> GetShader(const std::string& aKey, const ShaderType& aType);

		static std::string GetKeyFromPath(const std::filesystem::path& aPath);

		static bool KeyHasCompiled(const std::string& aKey);
		static bool KeyHasCompiled(size_t aKey);

		static bool IsHash(const std::string& key);

		static size_t GetHash(const std::string& key);

		// Recompiles all shaders in the library.
		static void ReCompileAllShaders(uint32_t* aNumberOfCompiledShaders, uint32_t* aTotalNumberOfShaders, std::string* aCurrentCompilingString);

		// Returns a list of all keys in the shader library.
		static std::vector<std::string> GetAllShaderKeys();

		// Recompiles the set of shaders identified by the given key.
		static void Recompile(const std::string& aKey, bool aSendEvent = true);

		// Binds the set of shaders identified by the given key.
		static void Bind(const std::string& aKey, ID3D11DeviceContext* aContext);

		// Removes the set of shaders identified by the given key from the library.
		static void Remove(const std::string& aKey);

		// Unbinds the set of shaders identified by the given key.
		static void Unbind(const std::string& aKey, ID3D11DeviceContext* aContext);

		// --- Hashed versions --- //

		// Adds a new set of shaders to the library, identified by the given hash.
		static void Add(const size_t& aHash, std::initializer_list<Ref<Shader>> someShaders);
		static void Add(const size_t& aHash, const std::vector<Ref<Shader>>& someShaders);

		// Returns the shader of the specified type from the set identified by the given hash.
		static Ref<Shader> GetShader(const size_t& aHash, const ShaderType& aType);

		// Recompiles the set of shaders identified by the given hash.
		static void Recompile(const size_t& aHash, bool aSendEvent = true);

		// Binds the set of shaders identified by the given hash.
		static void Bind(const size_t& aHash, ID3D11DeviceContext* aContext);

		// Removes the set of shaders identified by the given hash from the library.
		static void Remove(const size_t& aHash);

		// Unbinds the set of shaders identified by the given hash.
		static void Unbind(const size_t& aHash, ID3D11DeviceContext* aContext);

		static void BuildShaders();

	private:
		static void PushBuildFunction(std::function<void()>&& aFunc);

		inline static std::unordered_map<std::string, std::vector<Ref<Shader>>> myMap;
		inline static std::unordered_map<std::string, bool> myIsHash;

		inline static std::vector<std::function<void()>> myCompiledShaderStack;
		inline static std::mutex myPushCompiledShaderMutex;

		inline static std::string myCurrentKey;
		inline static std::condition_variable myCompileThread;
		inline static std::mutex myMutex;
		inline static std::atomic<bool> myReadyToCopy = false;
		inline static bool myDone = false;
	};
}