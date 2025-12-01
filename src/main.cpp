#include <windows.h>
#include "App.h"
#include "Logger.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    
    char logPath[MAX_PATH];
    GetModuleFileNameA(nullptr, logPath, MAX_PATH);
    std::string logPathStr(logPath);
    size_t lastSlash = logPathStr.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        logPathStr = logPathStr.substr(0, lastSlash + 1);
    }
    logPathStr += "parental_control.log";
    
    Logger::getInstance().init(logPathStr);
    LOG_INFO("Application starting");
    LOG_INFO("Log file: " + logPathStr);
    
    App app;
    
    if (!app.initialize(hInstance)) {
        LOG_ERROR("Failed to initialize application");
        MessageBoxW(nullptr, L"Failed to initialize application. Check if another instance is running.", 
                   L"Parental Control", MB_ICONERROR);
        return 1;
    }
    
    int result = app.run();
    
    LOG_INFO("Application exiting with code: " + std::to_string(result));
    return result;
}
