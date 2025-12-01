#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <atomic>

class OverlayWindow {
public:
    enum class State {
        HIDDEN,
        LOCKED,
        NOTIFICATION,
        SHUTDOWN_WARNING
    };

    OverlayWindow();
    ~OverlayWindow();

    bool create(HINSTANCE hInstance);
    void show();
    void hide();
    void destroy();
    
    void setState(State state);
    State getState() const { return m_state; }
    
    void setNotificationText(const std::string& text);
    void setCountdownText(const std::string& text);
    void setPasswordError(bool error);
    
    void setPasswordCallback(std::function<void(const std::string&)> callback) { m_passwordCallback = callback; }
    void setShutdownCallback(std::function<void()> callback) { m_shutdownCallback = callback; }
    void setRestartCallback(std::function<void()> callback) { m_restartCallback = callback; }
    
    int run();  // Main message loop
    bool isRunning() const { return m_running; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void createControls();
    void updateControls();
    void installKeyboardHook();
    void uninstallKeyboardHook();
    
    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    
    // Controls
    HWND m_titleLabel = nullptr;
    HWND m_messageLabel = nullptr;
    HWND m_passwordEdit = nullptr;
    HWND m_submitButton = nullptr;
    HWND m_shutdownButton = nullptr;
    HWND m_restartButton = nullptr;
    
    // Resources
    HFONT m_titleFont = nullptr;
    HFONT m_normalFont = nullptr;
    HBRUSH m_bgBrush = nullptr;
    
    // State
    State m_state = State::LOCKED;
    std::wstring m_notificationText = L"Enter password to unlock";
    std::wstring m_countdownText;
    bool m_passwordError = false;
    std::atomic<bool> m_running{true};
    
    // Keyboard hook
    static HHOOK s_keyboardHook;
    static OverlayWindow* s_instance;
    
    // Callbacks
    std::function<void(const std::string&)> m_passwordCallback;
    std::function<void()> m_shutdownCallback;
    std::function<void()> m_restartCallback;
    
    // Control IDs
    static constexpr int ID_PASSWORD_EDIT = 1001;
    static constexpr int ID_SUBMIT_BUTTON = 1002;
    static constexpr int ID_SHUTDOWN_BUTTON = 1003;
    static constexpr int ID_RESTART_BUTTON = 1004;
    static constexpr int ID_TITLE_LABEL = 1005;
    static constexpr int ID_MESSAGE_LABEL = 1006;
};
