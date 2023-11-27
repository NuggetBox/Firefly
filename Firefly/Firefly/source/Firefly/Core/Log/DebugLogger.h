#pragma once
#include <format>
#include <functional>
#include <mutex>
#include <sstream>

//Example use: LOGINFO("Hello world!");
//Example use: LOGINFO("Model with entity id: {} and modelpath: {} loaded", entity.id, entity.modelPath);
//Output: Model with entity id: 32 and modelpath: Models/Niklas.fbx loaded
/**
 * \brief Log info is used to be able to log to the console\n
Example use: LOGINFO("Hello world!");\n
Example use: LOGINFO("Model with entity id: {} and modelpath: {} loaded", entity.id, entity.modelPath);\n
Output: Model with entity id: 32 and modelpath: Models/Niklas.fbx loaded
 */

#ifndef FF_INLAMNING
#define LOGINFO(...) DebugLogger::FormattedLog(MessageType::Info, __FILE__, __func__, std::to_string(__LINE__), __VA_ARGS__)
#define LOGWARNING(...) DebugLogger::FormattedLog(MessageType::Warning, __FILE__, __func__, std::to_string(__LINE__), __VA_ARGS__)
#define LOGERROR(...) DebugLogger::FormattedLog(MessageType::Error, __FILE__, __func__, std::to_string(__LINE__), __VA_ARGS__)
#else
#define LOGINFO(...)
#define LOGWARNING(...)
#define LOGERROR(...)
#endif

enum class MessageType
{
	Info,
	Warning,
	Error
};

namespace std::filesystem
{
	class path;
}
using DetailedLogFunc = std::function<void(
	MessageType			/*MessageType*/,
	const std::string&	/*TimeSent*/,
	const std::string&	/*Filename*/,
	const std::string&	/*Function Name*/,
	const std::string&	/*CodeLine*/,
	const std::string&	/*Message*/)>;

class DebugLogger
{
public:
	template <typename... Args>
	static void FormattedLog(MessageType aMessageType, const std::string& aFilename = "", const std::string& aFunction = "", const std::string& aCodeLine = "", const std::string& aFormat = "", Args&&... someArgs);
	static void Log(const std::string& aMessage, MessageType aMessageType, const std::string& aFilename = "", const std::string& aFunction = "", const std::string& aCodeLine = "");

	static void Info(const std::string& aMessage, const std::string& aFilename = "", const std::string& aFunction = "", const std::string& aCodeLine = "");
	static void Warning(const std::string& aMessage, const std::string& aFilename = "", const std::string& aFunction = "", const std::string& aCodeLine = "");
	static void Error(const std::string& aMessage, const std::string& aFilename = "", const std::string& aFunction = "", const std::string& aCodeLine = "");

	static void QueueLogFunc(std::function<void(const char*)>&& aFunction);
	static void QueueDetailedLogFunc(DetailedLogFunc&& aFunction);

	static void WriteLogFile(const std::filesystem::path& aFilePath);

private:
	friend class ExceptionHandler;

	inline static std::vector<std::function<void(const char*)>> myLogQueue;

	inline static std::vector<DetailedLogFunc> myDetailedLogQueue;

	inline static std::stringstream myLog;

	inline static std::mutex myMutex;
};

template<typename... Args>
void DebugLogger::FormattedLog(MessageType aMessageType, const std::string& aFilename, const std::string& aFunction, const std::string& aCodeLine, const std::string& aFormat, Args&&... someArgs)
{
	const std::string message = std::vformat(aFormat, std::make_format_args(someArgs...));
	Log(message, aMessageType, aFilename, aFunction, aCodeLine);
}