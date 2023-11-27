#pragma once
#include "Utils/Queue.hpp"

class ExceptionHandler
{
public:
	/**
	 * \brief Queue a function to be called on crash
	 * \param aFunction A Function to be called before Firefly closes after crashing
	 */
	static void QueueCrashFunc(std::function<void()>&& aFunction);

	static LONG WINAPI HandleException(_EXCEPTION_POINTERS* aExceptionP);
private:
	static void CreateMiniDump(EXCEPTION_POINTERS* someExceptionPointers);
	static void CallCrashFunctions();
	static void SendDiscordBotCrashMessage(const std::filesystem::path& aFile);
	static std::string WriteCrashLog();

	inline static Utils::Queue<std::function<void()>> myCrashQueue;
};