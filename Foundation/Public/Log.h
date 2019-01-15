#pragma once
#include "FoundationAPI.h"
#include <string>
#include <vector>
#include <cstdarg>

namespace tc
{
enum class LogLevels : uint8_t
{
    All,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off
};

class ILogListener
{
public:
	virtual ~ILogListener() {}
	virtual LogLevels GetLogLevel() = 0;
	virtual void PrintBuffer(LogLevels level, const char* buffer, size_t size) = 0;
	virtual void PrintString(LogLevels level, const char* line) = 0;
};

class FOUNDATION_API FLog
{
public:
	static void AddListener(ILogListener* l);
	static bool RemoveListener(ILogListener* l);
    static void DispatchLog(LogLevels level, const char* fmt, ...);

protected:
	static std::vector<ILogListener*> Listeners;
};
}

#ifndef NDEBUG
#define LOGDEBUG(...) do{tc::FLog::DispatchLog(tc::LogLevels::Debug, __VA_ARGS__);}while(false)
#else
#define LOGDEBUG(...)
#endif

#define LOGINFO(...) do{tc::FLog::DispatchLog(tc::LogLevels::Info, __VA_ARGS__);}while(false)
#define LOGWARN(...) do{tc::FLog::DispatchLog(tc::LogLevels::Warn, __VA_ARGS__);}while(false)
#define LOGERROR(...) do{tc::FLog::DispatchLog(tc::LogLevels::Error, __VA_ARGS__);}while(false)
#define LOGFATAL(...) do{tc::FLog::DispatchLog(tc::LogLevels::Fatal, __VA_ARGS__);}while(false)

//Compatibility with ALOG

#ifndef LOG_ALWAYS_FATAL_IF
#define LOG_ALWAYS_FATAL_IF(cond, ...) \
    ( (cond) \
    ? ((void)tc::FLog::DispatchLog(tc::LogLevels::Fatal, __VA_ARGS__)) \
    : (void)0 )
#endif

#define ALOG_ASSERT(cond, ...)

#define ALOGW LOGWARN
