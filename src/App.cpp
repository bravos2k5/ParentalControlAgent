#include "App.h"
#include "Logger.h"
#include "DeviceInfo.h"
#include <sstream>

const std::string App::EMERGENCY_PASSWORD = "emergency123";
const std::string App::WS_ENDPOINT = "wss://example.yourdomain.com/ws/";

App::App() {
    m_overlay = std::make_unique<OverlayWindow>();
    m_wsClient = std::make_unique<WebSocketClient>();
    m_timerManager = std::make_unique<TimerManager>();
}

App::~App() {
    shutdown();
}

bool App::initialize(HINSTANCE hInstance) {
    // Check for single instance using mutex
    m_mutex = CreateMutexW(nullptr, FALSE, L"ParentalControlAgentMutex");
    if (m_mutex == nullptr) {
        LOG_ERROR("Failed to create mutex, error: " + std::to_string(GetLastError()));
        return false;
    }
    
    DWORD waitResult = WaitForSingleObject(m_mutex, 0);
    if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED) {
        CloseHandle(m_mutex);
        m_mutex = nullptr;
        return false;
    }
    
    if (waitResult == WAIT_ABANDONED) {
        LOG_WARNING("Previous instance did not release mutex properly, taking ownership");
    }

    LOG_INFO("Initializing application");
    
    // Get device info
    m_deviceId = DeviceInfo::getDeviceId();
    m_deviceName = DeviceInfo::getDeviceName();
    
    LOG_INFO("Device ID: " + m_deviceId);
    LOG_INFO("Device Name: " + m_deviceName);
    
    // Create overlay window (but don't show yet)
    if (!m_overlay->create(hInstance)) {
        LOG_ERROR("Failed to create overlay window");
        return false;
    }
    
    // Setup callbacks
    setupCallbacks();
    
    // Try to connect to server (wait up to 60 seconds, no UI during this time)
    LOG_INFO("Attempting to connect to server...");
    m_wsClient->connect(WS_ENDPOINT, m_deviceId, m_deviceName);
    
    if (m_wsClient->waitForConnection(60)) {
        // Connected successfully - show locked screen
        LOG_INFO("Connected to server, showing lock screen");
        m_emergencyMode = false;
        m_overlay->setNotificationText("Connected. Enter password to unlock.");
        m_overlay->setState(OverlayWindow::State::LOCKED);
    } else {
        // Failed to connect - enter emergency mode
        LOG_WARNING("Failed to connect to server, entering emergency mode");
        m_emergencyMode = true;
        m_overlay->setNotificationText("Offline mode. Use emergency password.");
        m_overlay->setState(OverlayWindow::State::LOCKED);
    }
    
    m_blocked = true;
    
    LOG_INFO("Application initialized successfully");
    return true;
}

void App::setupCallbacks() {
    // WebSocket message callback
    m_wsClient->setMessageCallback([this](const std::string& message) {
        handleWebSocketMessage(message);
    });
    
    // Connection callback - only handle disconnection after initial connect
    m_wsClient->setConnectionCallback([this](bool connected) {
        if (!connected && !m_emergencyMode) {
            // Lost connection after being connected - enter emergency mode
            LOG_WARNING("Lost connection to server, entering emergency mode");
            m_emergencyMode = true;
            m_overlay->setNotificationText("Connection lost. Use emergency password.");
            if (m_blocked) {
                m_overlay->setState(OverlayWindow::State::LOCKED);
            }
        }
    });
    
    // Overlay callbacks
    m_overlay->setPasswordCallback([this](const std::string& password) {
        if (m_emergencyMode) {
            checkEmergencyPassword(password);
        } else {
            sendPasswordToServer(password);
        }
    });
    
    m_overlay->setShutdownCallback([this]() {
        performShutdown();
    });
    
    m_overlay->setRestartCallback([this]() {
        performRestart();
    });
}

void App::checkEmergencyPassword(const std::string& password) {
    LOG_INFO("Checking emergency password");
    
    if (password == EMERGENCY_PASSWORD) {
        LOG_INFO("Emergency password accepted");
        handleGranted(3600);
    } else {
        LOG_WARNING("Wrong emergency password");
        handleDenied();
    }
}

void App::handleWebSocketMessage(const std::string& message) {
    LOG_INFO("Processing message: " + message);
    
    if (message.find("GRANTED:") == 0) {
        int seconds = parseSeconds(message.substr(8));
        handleGranted(seconds);
    }
    else if (message.find("BLOCK:") == 0) {
        int seconds = parseSeconds(message.substr(6));
        handleBlock(seconds);
    }
    else if (message.find("SHUTDOWN:") == 0) {
        int seconds = parseSeconds(message.substr(9));
        handleShutdown(seconds);
    }
    else if (message == "DENIED") {
        handleDenied();
    }
    else {
        LOG_WARNING("Unknown message: " + message);
    }
}

void App::handleGranted(int seconds) {
    LOG_INFO("Access granted for " + std::to_string(seconds) + " seconds");
    
    m_blocked = false;
    m_timerManager->cancelBlockTimer();
    m_overlay->setState(OverlayWindow::State::HIDDEN);
    
    m_timerManager->startGrantedTimer(seconds, [this]() {
        onGrantedExpired();
    });
}

void App::handleBlock(int seconds) {
    LOG_INFO("Will block after " + std::to_string(seconds) + " seconds");
    
    m_timerManager->cancelGrantedTimer();
    
    m_timerManager->startBlockTimer(seconds, [this]() {
        onBlockExpired();
    });
}

void App::handleShutdown(int seconds) {
    LOG_INFO("Shutdown in " + std::to_string(seconds) + " seconds");
    
    m_overlay->setState(OverlayWindow::State::SHUTDOWN_WARNING);
    m_overlay->setNotificationText("Computer will shutdown in " + std::to_string(seconds) + " seconds!");
    
    m_timerManager->startShutdownTimer(seconds, [this]() {
        onShutdownExpired();
    });
}

void App::handleDenied() {
    LOG_INFO("Access denied - wrong password");
    m_overlay->setPasswordError(true);
    m_overlay->setNotificationText("Wrong password. Please try again.");
}

void App::onGrantedExpired() {
    LOG_INFO("Granted time expired, locking");
    m_blocked = true;
    m_overlay->setState(OverlayWindow::State::LOCKED);
    m_overlay->setNotificationText("Time expired. Enter password to unlock.");
    sendBlockedMessage();
}

void App::onBlockExpired() {
    LOG_INFO("Block timer expired, blocking user");
    m_blocked = true;
    m_timerManager->cancelGrantedTimer();
    m_overlay->setState(OverlayWindow::State::LOCKED);
    m_overlay->setNotificationText("Access blocked. Enter password to unlock.");
    sendBlockedMessage();
}

void App::onShutdownExpired() {
    LOG_INFO("Shutdown timer expired, performing shutdown");
    performShutdown();
}

void App::sendPasswordToServer(const std::string& password) {
    if (!password.empty()) {
        LOG_INFO("Sending password to server");
        m_wsClient->sendMessage("PASSWORD:" + password);
        m_overlay->setPasswordError(false);
    }
}

void App::sendBlockedMessage() {
    LOG_INFO("Sending BLOCKED message to server");
    m_wsClient->sendMessage("BLOCKED");
}

void App::performShutdown() {
    LOG_INFO("Performing system shutdown");
    
    // Execute shutdown command
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
        CloseHandle(hToken);
    }
    
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
}

void App::performRestart() {
    LOG_INFO("Performing system restart");
    
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
        CloseHandle(hToken);
    }
    
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
}

int App::parseSeconds(const std::string& value) {
    try {
        return std::stoi(value);
    } catch (...) {
        LOG_WARNING("Failed to parse seconds: " + value);
        return 0;
    }
}

int App::run() {
    LOG_INFO("Starting message loop");
    return m_overlay->run();
}

void App::shutdown() {
    LOG_INFO("Shutting down application");
    m_running = false;
    
    if (m_wsClient) {
        m_wsClient->disconnect();
    }
    
    if (m_timerManager) {
        m_timerManager->cancelAllTimers();
    }
    
    if (m_mutex) {
        ReleaseMutex(m_mutex);
        CloseHandle(m_mutex);
        m_mutex = nullptr;
    }
}
