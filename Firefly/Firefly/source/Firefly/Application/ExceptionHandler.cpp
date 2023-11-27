#include "FFpch.h"
#include "ExceptionHandler.h"

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h>
#include <Lmcons.h>
#include <filesystem>

#include <dpp/cluster.h>

#include "Firefly/Core/Log/DebugLogger.h"

#include "Utils/Timer.h"

void ExceptionHandler::QueueCrashFunc(std::function<void()>&& aFunction)
{
	myCrashQueue.Enqueue(aFunction);
}

LONG WINAPI ExceptionHandler::HandleException(_EXCEPTION_POINTERS* aExceptionP)
{
	std::string crashLogFile = WriteCrashLog();
	CreateMiniDump(aExceptionP);

	//Do crash functions after CrashLog and CrashDump, for safety?
	CallCrashFunctions();

	SendDiscordBotCrashMessage(crashLogFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

void ExceptionHandler::CreateMiniDump(EXCEPTION_POINTERS* someExceptionPointers)
{
	HANDLE dumpFile;
	BOOL dumpSuccessful;

	const std::filesystem::path dumpPath(std::filesystem::current_path().wstring() + L"\\CrashDumps\\");

	if (!std::filesystem::exists(dumpPath))
	{
		std::filesystem::create_directory(dumpPath);
	}

	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);

	WCHAR usernameW[UNLEN + 1];
	DWORD usernameLength = UNLEN + 1;
	GetUserName(usernameW, &usernameLength);
	std::wstring username(usernameW);

	WCHAR fullPathW[MAX_PATH];

	StringCchPrintf(fullPathW, MAX_PATH, L"%s%s_%04d-%02d-%02d_%02d-%02d-%02d.dmp",
		dumpPath.wstring().c_str(), usernameW,
		systemTime.wYear, systemTime.wMonth, systemTime.wDay,
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

	dumpFile = CreateFile(fullPathW, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	MINIDUMP_EXCEPTION_INFORMATION ExpParam;
	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = someExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	dumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		dumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

	assert(dumpSuccessful && L"Couldn't write the Crash Dump");

	const std::wstring fullPath(fullPathW);
	const std::wstring message(L"Program crashed! A CrashDump was created at " + fullPath + L", send it to a programmer for debugging :)");
	MessageBox(NULL, message.c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);
}

void ExceptionHandler::CallCrashFunctions()
{
	while (!myCrashQueue.IsEmpty())
	{
		myCrashQueue.Dequeue()();
	}
}

void ExceptionHandler::SendDiscordBotCrashMessage(const std::filesystem::path& aFile)
{
#ifdef FF_RELEASE
	dpp::cluster bot("");
	dpp::webhook wh("https://discord.com/api/webhooks/1044623474108399706/b3Ef6oDVWM2VsxPZqKnLkbfr5XT2ObQ5fp1mXOjU5NshvU74NuLKScWvSfmkm_n5J9hB");

	char name[257];
	DWORD size = 257;
	GetUserNameA(name, &size);

	std::string nameStr = name;

	std::string msg = "I was killed by " + nameStr + " :skull:";

	auto message = dpp::message(msg.c_str()).add_file(aFile.string(), DebugLogger::myLog.str());
	bot.execute_webhook(wh, message);
#endif
}

std::string ExceptionHandler::WriteCrashLog()
{
	WCHAR usernameW[UNLEN + 1];
	DWORD usernameLength = UNLEN + 1;
	GetUserName(usernameW, &usernameLength);
	std::wstring usernameWStr(usernameW);
	std::string username(usernameWStr.begin(), usernameWStr.end());

	std::string filename = "CrashLogs/" + username + "_" + Utils::Timer::GetTimeStamp();
	std::ranges::replace(filename, ':', '-');
	std::ranges::replace(filename, '.', '-');
	filename.append(".txt");

	DebugLogger::WriteLogFile(filename);
	return filename;
}