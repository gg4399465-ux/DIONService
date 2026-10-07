#include "Logger.h"
#include <vector>

HANDLE Logger::hFile = INVALID_HANDLE_VALUE;
std::mutex Logger::mtx;

void Logger::Init(const std::wstring& path) {
    // สร้าง directory ถ้ายังไม่มี
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        std::wstring dir = path.substr(0, pos);
        CreateDirectory(dir.c_str(), NULL);
    }

    hFile = CreateFile(path.c_str(),
        FILE_APPEND_DATA, FILE_SHARE_READ,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

void Logger::Log(LOG_LEVEL level, const std::wstring& msg) {
    std::lock_guard<std::mutex> lock(mtx);

    if (hFile == INVALID_HANDLE_VALUE) return;

    std::wstring line = GetTimeStamp() + L" [" + LevelToString(level) + L"] " + msg + L"\r\n";

    // OutputDebugString สำหรับ debug
    OutputDebugString(line.c_str());

    // Write to file
    DWORD written;
    WriteFile(hFile, line.c_str(), (DWORD)(line.size() * sizeof(wchar_t)), &written, NULL);
    FlushFileBuffers(hFile);
}

void Logger::Close() {
    std::lock_guard<std::mutex> lock(mtx);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

std::wstring Logger::GetTimeStamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[64];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return buf;
}

std::wstring Logger::LevelToString(LOG_LEVEL level) {
    switch (level) {
    case LOG_LEVEL::INFO:  return L"INFO";
    case LOG_LEVEL::OK:    return L"OK";
    case LOG_LEVEL::WARN:  return L"WARN";
    case LOG_LEVEL::ERROR: return L"ERROR";
    default: return L"?";
    }
}