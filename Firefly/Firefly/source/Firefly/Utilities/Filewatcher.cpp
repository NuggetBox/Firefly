#include "FFpch.h"
#include "Filewatcher.h"

namespace Firefly
{
	void FileWatcher::Initialize(FileWatcherInfo aInfo)
	{
#ifndef FF_SHIPIT
		myFileWatcher = CreateRef<efsw::FileWatcher>();
		myListener = CreateRef<FileListener>();

		const size_t amountOfFolders = aInfo.Folders.size();
		myWatchIds.resize(amountOfFolders);
		size_t currentIndex = 0;
		for (auto& watch : myWatchIds)
		{
			watch = myFileWatcher->addWatch(aInfo.Folders[currentIndex].string(), myListener.get(), true);
			currentIndex++;
		}
#endif
	}
	void FileWatcher::Watch()
	{
#ifndef FF_SHIPIT
		myFileWatcher->watch();
#endif
	}
	void FileWatcher::Shutdown()
	{
	}
	void FileWatcher::AddEntry(const std::filesystem::path& aPath)
	{
		myFileEvents.emplace_back(aPath);
	}
}