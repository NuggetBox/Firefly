#include "FFpch.h"
#include "DebugLogger.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include "Console/color.hpp"

#include "Utils/Timer.h"

void DebugLogger::Log(const std::string& aMessage, MessageType aMessageType, const std::string& aFilename, const std::string& aFunction, const std::string& aCodeLine)
{
	std::scoped_lock lock(myMutex);
	const std::filesystem::path path(aFilename);
	const std::string message = path.stem().string() + "::" + aFunction + ":" + aCodeLine + ": " + aMessage;

	//Write to Log
	auto timeString = Utils::Timer::GetTime();
	const std::string logTimeStamp = "[" + timeString + "] ";
	myLog << "[" << timeString << "] ";

	switch (aMessageType)
	{
		case MessageType::Info: myLog << "[MESSAGE"; break;
		case MessageType::Warning: myLog << "[WARNING"; break;
		case MessageType::Error: myLog << "[ERROR"; break;
	}

	if (aMessageType == MessageType::Error && !aFilename.empty())
	{
		myLog << " in " << path.filename().string() << ":" << aFunction << ":" << aCodeLine;
	}

	myLog << "] " << aMessage << std::endl;
	//

	//Write to console
	const std::string consoleTimeStamp = "[" + Utils::Timer::GetTimeStamp() + "] ";
	std::cout << dye::grey(consoleTimeStamp);
	switch (aMessageType)
	{
		case MessageType::Info: std::cout << dye::light_green(message); break;
		case MessageType::Warning: std::cout << dye::yellow(message); break;
		case MessageType::Error: std::cout << dye::light_red(message); break;
	}
	std::cout << std::endl;
	for (uint32_t i = 0; i < myLogQueue.size(); ++i)
	{
		myLogQueue[i](aMessage.c_str());
	}

	for (uint32_t i = 0; i < myDetailedLogQueue.size(); ++i)
	{
		myDetailedLogQueue[i](aMessageType, timeString, std::filesystem::path(aFilename).filename().string(), aFunction, aCodeLine, aMessage);
	}
	//
}

void DebugLogger::Info(const std::string& aMessage, const std::string& aFilename, const std::string& aFunction, const std::string& aCodeLine)
{
	Log(aMessage, MessageType::Info, aFilename, aFunction, aCodeLine);
}

void DebugLogger::Warning(const std::string& aMessage, const std::string& aFilename, const std::string& aFunction, const std::string& aCodeLine)
{
	Log(aMessage, MessageType::Warning, aFilename, aFunction, aCodeLine);
}

void DebugLogger::Error(const std::string& aMessage, const std::string& aFilename, const std::string& aFunction, const std::string& aCodeLine)
{
	Log(aMessage, MessageType::Error, aFilename, aFunction, aCodeLine);
}

void DebugLogger::QueueLogFunc(std::function<void(const char*)>&& aFunction)
{
	myLogQueue.push_back(aFunction);
}

void DebugLogger::QueueDetailedLogFunc(DetailedLogFunc&& aFunction)
{
	myDetailedLogQueue.push_back(aFunction);
}

void DebugLogger::WriteLogFile(const std::filesystem::path& aFilePath)
{
	if (!std::filesystem::exists(aFilePath.parent_path()))
	{
		std::filesystem::create_directory(aFilePath.parent_path());
	}

	std::ofstream outFile;
	outFile.open(aFilePath);
	outFile << myLog.rdbuf();
	outFile.close();
}