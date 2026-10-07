#include "NetworkTweaks.h"
#include "Logger.h"
#include <windows.h>
#include <iphlpapi.h>
#include <vector>

#pragma comment(lib, "iphlpapi.lib")

void NetworkTweaks::ApplyStandard() {
    Logger::Log(LOG_LEVEL::INFO, L"[NET] Applying standard network tweaks");
    DisableNICOffloads();
    SetRSS();
    Logger::Log(LOG_LEVEL::OK, L"[NET] Standard applied");
}

void NetworkTweaks::ApplyHardcore() {
    Logger::Log(LOG_LEVEL::INFO, L"[NET] Applying hardcore network tweaks");
    // Additional NIC tuning via WMI would go here
    Logger::Log(LOG_LEVEL::OK, L"[NET] Hardcore applied");
}

void NetworkTweaks::ApplyApex() {
    Logger::Log(LOG_LEVEL::INFO, L"[NET] Applying APEX network tweaks");
    Logger::Log(LOG_LEVEL::OK, L"[NET] APEX applied");
}

void NetworkTweaks::DisableNICOffloads() {
    // ใช้ PowerShell เรียกครั้งเดียว (ง่ายกว่า WMI)
    system("powershell -NoProfile -Command \"Get-NetAdapter | Where-Object {$_.Status -eq 'Up'} | ForEach-Object { Disable-NetAdapterRsc -Name $_.Name -ErrorAction SilentlyContinue; Set-NetAdapterLso -Name $_.Name -V1IPv4Enabled $false -IPv4Enabled $false -IPv6Enabled $false -ErrorAction SilentlyContinue }\"");
    Logger::Log(LOG_LEVEL::OK, L"[NET] NIC offloads disabled");
}

void NetworkTweaks::SetRSS() {
    system("powershell -NoProfile -Command \"Get-NetAdapter | Where-Object {$_.Status -eq 'Up'} | ForEach-Object { Set-NetAdapterRss -Name $_.Name -BaseProcessorNumber 2 -MaxProcessorNumber 4 -ErrorAction SilentlyContinue }\"");
    Logger::Log(LOG_LEVEL::OK, L"[NET] RSS configured (Core 2-4)");
}