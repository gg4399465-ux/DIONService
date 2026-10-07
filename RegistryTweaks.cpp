#include "RegistryTweaks.h"
#include "Logger.h"

bool RegistryTweaks::SetDword(HKEY root, const std::wstring& subkey, const std::wstring& name, DWORD value) {
    HKEY hKey;
    LONG result = RegCreateKeyEx(root, subkey.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) return false;

    result = RegSetValueEx(hKey, name.c_str(), 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegistryTweaks::SetString(HKEY root, const std::wstring& subkey, const std::wstring& name, const std::wstring& value) {
    HKEY hKey;
    LONG result = RegCreateKeyEx(root, subkey.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) return false;

    result = RegSetValueEx(hKey, name.c_str(), 0, REG_SZ, (const BYTE*)value.c_str(), (DWORD)((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

void RegistryTweaks::ApplyStandard() {
    Logger::Log(LOG_LEVEL::INFO, L"[REG] Applying STANDARD tweaks");

    // TCP/IP
    std::wstring tcpPath = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"TcpTimedWaitDelay", 30);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"MaxUserPort", 65534);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"TcpFinWait2Delay", 30);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"TcpWindowSize", 65535);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"GlobalMaxTcpWindowSize", 65535);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"EnableWsd", 0);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"DisableTaskOffload", 1);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"EnableTCPChimney", 0);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"EnableRSS", 1);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"TcpNoDelay", 1);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"TcpAckFrequency", 1);

    // MMCSS
    std::wstring mmcssPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile";
    SetDword(HKEY_LOCAL_MACHINE, mmcssPath, L"NetworkThrottlingIndex", 0xFFFFFFFF);
    SetDword(HKEY_LOCAL_MACHINE, mmcssPath, L"SystemResponsiveness", 0);

    // Games Task
    std::wstring gamesTask = mmcssPath + L"\\Tasks\\Games";
    SetDword(HKEY_LOCAL_MACHINE, gamesTask, L"Priority", 6);
    SetDword(HKEY_LOCAL_MACHINE, gamesTask, L"GPU Priority", 8);
    SetString(HKEY_LOCAL_MACHINE, gamesTask, L"Scheduling Category", L"High");
    SetString(HKEY_LOCAL_MACHINE, gamesTask, L"SFIO Priority", L"High");

    // Game Bar / DVR
    SetDword(HKEY_CURRENT_USER, L"System\\GameConfigStore", L"GameDVR_Enabled", 0);
    SetDword(HKEY_CURRENT_USER, L"System\\GameConfigStore", L"GameDVR_FSEBehaviorMode", 2);
    SetDword(HKEY_CURRENT_USER, L"System\\GameConfigStore", L"GameDVR_HonorUserFSEBehaviorMode", 1);
    SetDword(HKEY_CURRENT_USER, L"System\\GameConfigStore", L"GameDVR_DXGIHonorFSEWindowsCompatible", 1);
    SetDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR", L"AppCaptureEnabled", 0);
    SetDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\GameBar", L"AllowAutoGameMode", 0);
    SetDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\GameBar", L"ShowStartupPanel", 0);

    // Input Lag (NEW)
    SetDword(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\PriorityControl", L"Win32PrioritySeparation", 40);

    // Filter Keys OFF
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Accessibility\\Keyboard Response", L"Flags", L"122");
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Accessibility\\StickyKeys", L"Flags", L"506");
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Accessibility\\ToggleKeys", L"Flags", L"58");

    // Mouse Accel OFF
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Mouse", L"MouseSpeed", L"0");
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Mouse", L"MouseThreshold1", L"0");
    SetString(HKEY_CURRENT_USER, L"Control Panel\\Mouse", L"MouseThreshold2", L"0");

    Logger::Log(LOG_LEVEL::OK, L"[REG] STANDARD applied");
}

void RegistryTweaks::ApplyHardcore() {
    Logger::Log(LOG_LEVEL::INFO, L"[REG] Applying HARDCORE tweaks");

    // Network Buffer Tuning
    std::wstring tcpPath = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"MaxFreeTcbs", 65534);
    SetDword(HKEY_LOCAL_MACHINE, tcpPath, L"MaxHashTableSize", 65536);
    SetString(HKEY_LOCAL_MACHINE, tcpPath, L"TcpAutotuninglevel", L"normal");

    // Game Mode ON
    SetDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\GameBar", L"AllowAutoGameMode", 1);

    // HAGS ON
    SetDword(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers", L"HwSchMode", 2);

    // Memory Management
    SetDword(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management", L"LargePageMinimum", 0);

    Logger::Log(LOG_LEVEL::OK, L"[REG] HARDCORE applied");
}

void RegistryTweaks::ApplyApex() {
    Logger::Log(LOG_LEVEL::INFO, L"[REG] Applying APEX tweaks");

    // Deep Network Throttling
    SetDword(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        L"NetworkThrottlingIndex", 0xFFFFFFFF);

    // USB Selective Suspend OFF
    SetDword(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\USB", L"DisableSelectiveSuspend", 1);

    Logger::Log(LOG_LEVEL::OK, L"[REG] APEX applied");
}