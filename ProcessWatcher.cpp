#include "ProcessWatcher.h"
#include "Logger.h"
#include <tlhelp32.h>
#include <algorithm>

std::thread ProcessWatcher::workerThread;
std::atomic<bool> ProcessWatcher::running{ false };
ProcessWatcher::Callback ProcessWatcher::callback;
std::vector<std::wstring> ProcessWatcher::targetProcesses = {
    L"FiveM", L"FiveM_b", L"GTA5", L"RDR2", L"Valorant", L"CS2"
};
std::vector<DWORD> ProcessWatcher::seenPids;

bool ProcessWatcher::Start(Callback cb) {
    callback = cb;
    running = true;
    workerThread = std::thread(WatchLoop);
    return true;
}

void ProcessWatcher::Stop() {
    running = false;
    if (workerThread.joinable()) workerThread.join();
    seenPids.clear();
}

bool ProcessWatcher::IsRunning() {
    return running;
}

void ProcessWatcher::WatchLoop() {
    while (running) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        PROCESSENTRY32W pe = { 0 };
        pe.dwSize = sizeof(pe);

        if (Process32FirstW(hSnapshot, &pe)) {
            do {
                std::wstring name = pe.szExeFile;

                // Check if target
                for (const auto& target : targetProcesses) {
                    if (name.find(target) != std::wstring::npos) {
                        // Check if seen
                        if (std::find(seenPids.begin(), seenPids.end(), pe.th32ProcessID) == seenPids.end()) {
                            seenPids.push_back(pe.th32ProcessID);
                            if (callback) callback(name, pe.th32ProcessID);
                        }
                        break;
                    }
                }
            } while (Process32NextW(hSnapshot, &pe));
        }

        CloseHandle(hSnapshot);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}