#pragma once
#include <windows.h>
#include <string>
#include <mutex>

enum class LOG_LEVEL { INFO, OK, WARN, ERROR };

class Logger {
public:
    static void Init(const std::wstring& path);
    static void Log(LOG_LEVEL level, const std::wstring& msg);
    static void Close();

private:
    static HANDLE hFile;
    static std::mutex mtx;
    static std::wstring GetTimeStamp();
    static std::wstring LevelToString(LOG_LEVEL level);
};