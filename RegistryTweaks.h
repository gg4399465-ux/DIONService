#pragma once
#include <windows.h>
#include <string>

class RegistryTweaks {
public:
    static void ApplyStandard();
    static void ApplyHardcore();
    static void ApplyApex();

private:
    static bool SetDword(HKEY root, const std::wstring& subkey, const std::wstring& name, DWORD value);
    static bool SetString(HKEY root, const std::wstring& subkey, const std::wstring& name, const std::wstring& value);
};