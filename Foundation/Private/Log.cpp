#include <Log.h>
#include <Platform.h>
#include <iostream>
#include <mutex>
#include <cassert>

namespace tc
{

std::vector<ILogListener*> FLog::Listeners;

void FLog::AddListener(ILogListener* l)
{
	Listeners.push_back(l);
}

bool FLog::RemoveListener(ILogListener* l)
{
	for(auto iter = Listeners.begin(); iter != Listeners.end(); ++iter)
	{
		if (*iter == l)
		{
			Listeners.erase(iter);
			return true;
		}
	}
	return false;
}

static std::mutex DispatchLogMutex;
static char DispatchLogBuffer[4096];

void FLog::DispatchLog(LogLevels level, const char* fmt, ...)
{
    std::lock_guard<std::mutex> guard(DispatchLogMutex);

	va_list ap;
	va_start(ap, fmt);
#if TC_OS == TC_OS_WINDOWS_NT
	vsnprintf_s(DispatchLogBuffer, sizeof(DispatchLogBuffer), fmt, ap);
#else
    vsnprintf(DispatchLogBuffer, sizeof(DispatchLogBuffer), fmt, ap);
#endif
	va_end(ap);

	for (auto l : Listeners)
	{
		if (level > l->GetLogLevel())
			l->PrintString(level, DispatchLogBuffer);
	}

	assert(level != LogLevels::Error && level != LogLevels::Fatal);
}

class FStdioLogListener : public ILogListener
{
public:
	FStdioLogListener()
	{
		FLog::AddListener(this);
	}

	LogLevels GetLogLevel() override
	{
        return LogLevels::All;
	}

	void PrintBuffer(LogLevels level, const char* buffer, size_t size) override
    {
		std::cout << buffer;
	}

	void PrintString(LogLevels level, const char* line) override
    {
        printf("%s", line);
	}
};

static FStdioLogListener StdioLogger;

}
