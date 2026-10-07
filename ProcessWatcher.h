#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

class ProcessWatcher {
public:
    using Callback = std::function<void(const std::wstring&, DWORD)>;

    static bool Start(Callback cb);
    static void Stop();
    static bool IsRunning();

private:
    static void WatchLoop();

    static std::thread workerThread;
    static std::atomic<bool> running;
    static Callback callback;
    static std::vector<std::wstring> targetProcesses;
    static std::vector<DWORD> seenPids;
};