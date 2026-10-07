#pragma once
#include <windows.h>
#include <cstdint>

class TimerEngine {
public:
    static bool Init();
    static bool SetResolution(uint32_t resolution100ns = 5000);  // 0.5ms
    static bool Refresh();  // เรียกซ้ำเพื่อคงค่า
    static bool Release();
    static uint32_t GetCurrentResolutionMs();

private:
    static bool isActive;
    static uint32_t requestedResolution;
};