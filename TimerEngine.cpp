#include "TimerEngine.h"
#include "Logger.h"

// ntdll imports
typedef NTSTATUS(WINAPI* NtSetTimerResolution_t)(ULONG, BOOLEAN, PULONG);
typedef NTSTATUS(WINAPI* NtQueryTimerResolution_t)(PULONG, PULONG, PULONG);

static NtSetTimerResolution_t pNtSetTimerResolution = nullptr;
static NtQueryTimerResolution_t pNtQueryTimerResolution = nullptr;

bool TimerEngine::isActive = false;
uint32_t TimerEngine::requestedResolution = 5000;

bool TimerEngine::Init() {
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (!hNtdll) hNtdll = LoadLibrary(L"ntdll.dll");
    if (!hNtdll) {
        Logger::Log(LOG_LEVEL::ERROR, L"[TIMER] ntdll not found");
        return false;
    }

    pNtSetTimerResolution = (NtSetTimerResolution_t)GetProcAddress(hNtdll, "NtSetTimerResolution");
    pNtQueryTimerResolution = (NtQueryTimerResolution_t)GetProcAddress(hNtdll, "NtQueryTimerResolution");

    if (!pNtSetTimerResolution || !pNtQueryTimerResolution) {
        Logger::Log(LOG_LEVEL::ERROR, L"[TIMER] Functions not found");
        return false;
    }

    return true;
}

bool TimerEngine::SetResolution(uint32_t resolution100ns) {
    if (!pNtSetTimerResolution) return false;

    ULONG current = 0;
    NTSTATUS status = pNtSetTimerResolution(resolution100ns, TRUE, &current);

    if (status == 0) {
        isActive = true;
        requestedResolution = resolution100ns;
        return true;
    }
    return false;
}

bool TimerEngine::Refresh() {
    if (!isActive || !pNtSetTimerResolution) return false;
    ULONG current = 0;
    pNtSetTimerResolution(requestedResolution, TRUE, &current);
    return true;
}

bool TimerEngine::Release() {
    if (!pNtSetTimerResolution) return false;
    ULONG current = 0;
    pNtSetTimerResolution(requestedResolution, FALSE, &current);
    isActive = false;
    return true;
}

uint32_t TimerEngine::GetCurrentResolutionMs() {
    if (!pNtQueryTimerResolution) return 0;
    ULONG min = 0, max = 0, current = 0;
    pNtQueryTimerResolution(&min, &max, &current);
    return (uint32_t)(current / 10000);  // 100ns → ms
}