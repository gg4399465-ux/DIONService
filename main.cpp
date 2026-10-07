// ============================================================================
//  DION SERVICE v1.0 - APEX NATIVE
//  Ultimate Low-Latency / Desync / Input Lag Killer
//  
//  ✅ Timer Engine (0.5ms)
//  ✅ ETW Process Monitor
//  ✅ Registry Tweaks
//  ✅ Network Optimization
//  ✅ Named Pipe IPC
// ============================================================================

#include <windows.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

#include "Logger.h"
#include "TimerEngine.h"
#include "RegistryTweaks.h"
#include "ProcessWatcher.h"
#include "NetworkTweaks.h"
#include "IpcServer.h"

// ---- Globals ----
SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

std::atomic<bool>     g_Running{ true };
std::atomic<bool>     g_HardcoreMode{ false };
std::atomic<bool>     g_ApexMode{ false };

// ---- Service Name ----
#define SERVICE_NAME  L"DIONService"
#define DISPLAY_NAME  L"DION ULTIMATE Service"
#define DESCRIPTION   L"DION ULTIMATE - Low-Latency / Desync / Input Lag Killer"

// ---- Forward Declarations ----
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
bool   InstallService();
bool   UninstallService();

// ============================================================================
//  SERVICE WORKER THREAD
// ============================================================================
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    Logger::Init(L"C:\\ProgramData\\DION_ULTIMATE\\service.log");
    Logger::Log(LOG_LEVEL::INFO, L"=== DION SERVICE v1.0 APEX NATIVE STARTED ===");

    // ---- 1. Timer Engine ----
    TimerEngine::Init();
    TimerEngine::SetResolution(5000);  // 0.5ms
    Logger::Log(LOG_LEVEL::OK, L"[TIMER] 0.5ms active");

    // ---- 2. Network Tweaks (ครั้งแรกเท่านั้น) ----
    NetworkTweaks::ApplyStandard();
    Logger::Log(LOG_LEVEL::OK, L"[NETWORK] Standard tweaks applied");

    // ---- 3. Process Watcher (Real-time) ----
    ProcessWatcher::Start([](const std::wstring& procName, DWORD pid) {
        Logger::Log(LOG_LEVEL::OK, L"[PROCESS] Detected: " + procName + L" (PID: " + std::to_wstring(pid) + L")");

        // Apply process priority
        HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
        if (hProc) {
            SetPriorityClass(hProc, HIGH_PRIORITY_CLASS);
            CloseHandle(hProc);
            Logger::Log(LOG_LEVEL::OK, L"[PROCESS] Priority set to HIGH");
        }
        });
    Logger::Log(LOG_LEVEL::OK, L"[PROCESS] Watcher started");

    // ---- 4. IPC Server ----
    IpcServer::Start([](const std::wstring& cmd) {
        Logger::Log(LOG_LEVEL::INFO, L"[IPC] Command: " + cmd);

        if (cmd == L"RUN_STANDARD") {
            RegistryTweaks::ApplyStandard();
            Logger::Log(LOG_LEVEL::OK, L"[IPC] Standard mode applied");
        }
        else if (cmd == L"RUN_HARDCORE") {
            RegistryTweaks::ApplyHardcore();
            NetworkTweaks::ApplyHardcore();
            g_HardcoreMode = true;
            Logger::Log(LOG_LEVEL::OK, L"[IPC] Hardcore mode applied");
        }
        else if (cmd == L"RUN_APEX") {
            RegistryTweaks::ApplyApex();
            NetworkTweaks::ApplyApex();
            g_HardcoreMode = true;
            g_ApexMode = true;
            Logger::Log(LOG_LEVEL::OK, L"[IPC] APEX mode applied");
        }
        else if (cmd == L"STOP") {
            g_Running = false;
            Logger::Log(LOG_LEVEL::INFO, L"[IPC] Stop requested");
        }
        else if (cmd == L"STATUS") {
            Logger::Log(LOG_LEVEL::INFO, L"[IPC] Status: " +
                std::wstring(g_ApexMode ? L"APEX" : g_HardcoreMode ? L"HARDCORE" : L"STANDARD"));
        }
        else {
            Logger::Log(LOG_LEVEL::WARN, L"[IPC] Unknown command");
        }
        });
    Logger::Log(LOG_LEVEL::OK, L"[IPC] Server started");

    // ---- Main Loop ----
    while (g_Running) {
        // Refresh timer ทุก 30 วินาที (กัน Windows reset)
        TimerEngine::Refresh();

        // Sleep 30 วินาที
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    // ---- Cleanup ----
    Logger::Log(LOG_LEVEL::INFO, L"=== SHUTTING DOWN ===");
    IpcServer::Stop();
    ProcessWatcher::Stop();
    TimerEngine::Release();

    Logger::Log(LOG_LEVEL::OK, L"=== DION SERVICE STOPPED ===");
    Logger::Close();

    return 0;
}

// ============================================================================
//  SERVICE CONTROL HANDLER
// ============================================================================
void WINAPI ServiceCtrlHandler(DWORD ctrlCode)
{
    switch (ctrlCode)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWaitHint = 10000;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        g_Running = false;

        if (g_ServiceStopEvent)
            SetEvent(g_ServiceStopEvent);

        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

// ============================================================================
//  SERVICE MAIN
// ============================================================================
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    // Register handler
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (!g_StatusHandle) return;

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 5000;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Create stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!g_ServiceStopEvent)
    {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Set service as running
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Start worker thread
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    // Wait for stop event
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);

    // Wait for worker
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);

    // Cleanup
    CloseHandle(g_ServiceStopEvent);
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

// ============================================================================
//  INSTALL / UNINSTALL
// ============================================================================
bool InstallService()
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!schSCManager) {
        wprintf(L"OpenSCManager failed (%d)\n", GetLastError());
        return false;
    }

    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    SC_HANDLE schService = CreateService(
        schSCManager,
        SERVICE_NAME,
        DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        path,
        NULL, NULL, NULL, NULL, NULL);

    if (!schService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            wprintf(L"Service already exists\n");
        }
        else {
            wprintf(L"CreateService failed (%d)\n", err);
        }
        CloseServiceHandle(schSCManager);
        return false;
    }

    // Set description
    SERVICE_DESCRIPTION desc = { (LPWSTR)DESCRIPTION };
    ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &desc);

    wprintf(L"Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool UninstallService()
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!schSCManager) return false;

    SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
    if (!schService) {
        CloseServiceHandle(schSCManager);
        return false;
    }

    SERVICE_STATUS status;
    ControlService(schService, SERVICE_CONTROL_STOP, &status);
    DeleteService(schService);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    wprintf(L"Service uninstalled\n");
    return true;
}

// ============================================================================
//  ENTRY POINT
// ============================================================================
int wmain(int argc, wchar_t* argv[])
{
    // Check arguments
    if (argc > 1)
    {
        std::wstring arg = argv[1];

        if (arg == L"install") {
            InstallService();
            return 0;
        }
        else if (arg == L"uninstall") {
            UninstallService();
            return 0;
        }
        else if (arg == L"console") {
            // Run in console mode (for testing)
            wprintf(L"Running in console mode...\n");
            ServiceWorkerThread(NULL);
            return 0;
        }
        else if (arg == L"start") {
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
            SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_START);
            StartService(schService, 0, NULL);
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            wprintf(L"Service started\n");
            return 0;
        }
        else if (arg == L"stop") {
            SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
            SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP);
            SERVICE_STATUS status;
            ControlService(schService, SERVICE_CONTROL_STOP, &status);
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            wprintf(L"Service stopped\n");
            return 0;
        }
        else {
            wprintf(L"Usage:\n");
            wprintf(L"  DIONService.exe install    - Install service\n");
            wprintf(L"  DIONService.exe uninstall  - Uninstall service\n");
            wprintf(L"  DIONService.exe start      - Start service\n");
            wprintf(L"  DIONService.exe stop       - Stop service\n");
            wprintf(L"  DIONService.exe console    - Run in console mode\n");
            return 0;
        }
    }

    // No arguments → run as service
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}