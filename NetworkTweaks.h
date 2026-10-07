#pragma once
#include <string>

class NetworkTweaks {
public:
    static void ApplyStandard();
    static void ApplyHardcore();
    static void ApplyApex();

private:
    static void DisableNICOffloads();
    static void SetRSS();
};