#pragma once

#include <windows.h>
#include <string>
#include <atomic>
#include <memory>

#include "OverlayWindow.h"
#include "WebSocketClient.h"
#include "TimerManager.h"

class App {
public:
    App();
    ~App();

    bool initialize(HINSTANCE hInstance);
    int run();
    void shutdown();

private:
    void setupCallbacks();
    void handleWebSocketMessage(const std::string& message);
    
    void handleGranted(int seconds);
    void handleBlock(int seconds);
    void handleShutdown(int seconds);
    void handleDenied();
    
    void onGrantedExpired();
    void onBlockExpired();
    void onShutdownExpired();
    
    void sendPasswordToServer(const std::string& password);
    void sendBlockedMessage();
    
    void performShutdown();
    void performRestart();
    
    void checkEmergencyPassword(const std::string& password);
    
    int parseSeconds(const std::string& value);

    std::unique_ptr<OverlayWindow> m_overlay;
    std::unique_ptr<WebSocketClient> m_wsClient;
    std::unique_ptr<TimerManager> m_timerManager;
    
    std::atomic<bool> m_emergencyMode{false};
    std::atomic<bool> m_blocked{false};
    std::atomic<bool> m_running{true};
    
    std::string m_deviceId;
    std::string m_deviceName;
    
    HANDLE m_mutex = nullptr;
    
    static const std::string EMERGENCY_PASSWORD;
    static const std::string WS_ENDPOINT;
};
