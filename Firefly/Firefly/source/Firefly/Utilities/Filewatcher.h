#pragma once
#include <filesystem>
#include <thread>
#include "Firefly/Core/Core.h"
#ifndef FF_SHIPIT 
#include "efsw/efsw.hpp"
#endif
#include "Firefly/Rendering/Shader/ShaderLibrary.h"
#include "Firefly/Event/EditorEvents.h"
namespace Firefly
{
	class FileListener;

	struct FileWatcherInfo
	{
		std::vector<std::filesystem::path> Folders;
	};


	class FileWatcher
	{
	public:
		static void Initialize(FileWatcherInfo aInfo);
		static void Watch();
		static void Shutdown();
		static void AddEntry(const std::filesystem::path& aPath);

		static std::vector<EditorFileEvent>& GetEvents() { return myFileEvents; };

	private:

		static inline std::vector<EditorFileEvent> myFileEvents;
#ifndef FF_SHIPIT 
		static inline Ref<efsw::FileWatcher> myFileWatcher;
		static inline Ref<FileListener> myListener;
		static inline std::vector<efsw::WatchID> myWatchIds;
	};

	class FileListener : public efsw::FileWatchListener
	{
	public:
		void handleFileAction(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action,
			std::string oldFilename) override
		{
			std::filesystem::path fullPath = dir + filename;

			if (!fullPath.has_extension())
			{
				return;
			}

			switch (action)
			{
			case efsw::Actions::Add:
				break;
			case efsw::Actions::Delete:

				break;
			case efsw::Actions::Modified:
			{
				FileWatcher::AddEntry(fullPath);
				if (fullPath.extension() == ".hlsl")
				{
					ShaderLibrary::Recompile(ShaderLibrary::GetKeyFromPath(fullPath));
				}
			}
			break;
			case efsw::Actions::Moved:

				break;
			default:
				break;
			}
		}
#endif

	};

}
