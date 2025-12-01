#include "OverlayWindow.h"
#include "Logger.h"

// Static members
HHOOK OverlayWindow::s_keyboardHook = nullptr;
OverlayWindow* OverlayWindow::s_instance = nullptr;

// Colors
static const COLORREF COLOR_BG = RGB(45, 45, 48);
static const COLORREF COLOR_TEXT = RGB(255, 255, 255);
static const COLORREF COLOR_ERROR = RGB(255, 100, 100);

OverlayWindow::OverlayWindow() {
    s_instance = this;
}

OverlayWindow::~OverlayWindow() {
    destroy();
    s_instance = nullptr;
}

bool OverlayWindow::create(HINSTANCE hInstance) {
    LOG_INFO("Creating overlay window");
    m_hInstance = hInstance;
    
    // Unregister class if exists (cleanup from previous run)
    UnregisterClassW(L"ParentalControlOverlay", hInstance);
    
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.lpszClassName = L"ParentalControlOverlay";
    
    if (!RegisterClassExW(&wc)) {
        LOG_ERROR("Failed to register window class: " + std::to_string(GetLastError()));
        return false;
    }
    
    // Get screen size
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    
    // Create fullscreen topmost window
    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"ParentalControlOverlay",
        L"Parental Control",
        WS_POPUP,
        0, 0, screenW, screenH,
        nullptr, nullptr, hInstance, this
    );
    
    if (!m_hwnd) {
        LOG_ERROR("Failed to create window: " + std::to_string(GetLastError()));
        return false;
    }
    
    // Create fonts
    m_titleFont = CreateFontW(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    m_normalFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    m_bgBrush = CreateSolidBrush(COLOR_BG);
    
    createControls();
    installKeyboardHook();
    
    LOG_INFO("Overlay window created successfully");
    return true;
}

void OverlayWindow::createControls() {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int centerX = screenW / 2;
    int centerY = screenH / 2;
    
    // Title label
    m_titleLabel = CreateWindowExW(0, L"STATIC", L"Computer Locked",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        centerX - 200, centerY - 120, 400, 50,
        m_hwnd, (HMENU)ID_TITLE_LABEL, m_hInstance, nullptr);
    SendMessageW(m_titleLabel, WM_SETFONT, (WPARAM)m_titleFont, TRUE);
    
    // Message label
    m_messageLabel = CreateWindowExW(0, L"STATIC", L"Enter password to unlock",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        centerX - 200, centerY - 60, 400, 30,
        m_hwnd, (HMENU)ID_MESSAGE_LABEL, m_hInstance, nullptr);
    SendMessageW(m_messageLabel, WM_SETFONT, (WPARAM)m_normalFont, TRUE);
    
    // Password input
    m_passwordEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_CENTER | ES_AUTOHSCROLL,
        centerX - 150, centerY - 10, 300, 35,
        m_hwnd, (HMENU)ID_PASSWORD_EDIT, m_hInstance, nullptr);
    SendMessageW(m_passwordEdit, WM_SETFONT, (WPARAM)m_normalFont, TRUE);
    
    // Submit button
    m_submitButton = CreateWindowExW(0, L"BUTTON", L"Unlock",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        centerX - 75, centerY + 40, 150, 40,
        m_hwnd, (HMENU)ID_SUBMIT_BUTTON, m_hInstance, nullptr);
    SendMessageW(m_submitButton, WM_SETFONT, (WPARAM)m_normalFont, TRUE);
    
    // Shutdown button
    m_shutdownButton = CreateWindowExW(0, L"BUTTON", L"Shutdown",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        centerX - 160, centerY + 100, 150, 35,
        m_hwnd, (HMENU)ID_SHUTDOWN_BUTTON, m_hInstance, nullptr);
    SendMessageW(m_shutdownButton, WM_SETFONT, (WPARAM)m_normalFont, TRUE);
    
    // Restart button
    m_restartButton = CreateWindowExW(0, L"BUTTON", L"Restart",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        centerX + 10, centerY + 100, 150, 35,
        m_hwnd, (HMENU)ID_RESTART_BUTTON, m_hInstance, nullptr);
    SendMessageW(m_restartButton, WM_SETFONT, (WPARAM)m_normalFont, TRUE);
}

void OverlayWindow::updateControls() {
    if (!m_hwnd) return;
    
    // Update title based on state
    const wchar_t* title = L"Computer Locked";
    switch (m_state) {
        case State::LOCKED: title = L"Computer Locked"; break;
        case State::NOTIFICATION: title = L"Warning"; break;
        case State::SHUTDOWN_WARNING: title = L"Shutdown Warning"; break;
        default: break;
    }
    SetWindowTextW(m_titleLabel, title);
    
    // Update message
    std::wstring msg = m_notificationText;
    if (!m_countdownText.empty()) {
        msg += L"\n" + m_countdownText;
    }
    SetWindowTextW(m_messageLabel, msg.c_str());
    
    // Show/hide password controls based on state
    bool showPassword = (m_state == State::LOCKED);
    ShowWindow(m_passwordEdit, showPassword ? SW_SHOW : SW_HIDE);
    ShowWindow(m_submitButton, showPassword ? SW_SHOW : SW_HIDE);
    
    // Always show shutdown/restart buttons when visible
    bool showButtons = (m_state != State::HIDDEN);
    ShowWindow(m_shutdownButton, showButtons ? SW_SHOW : SW_HIDE);
    ShowWindow(m_restartButton, showButtons ? SW_SHOW : SW_HIDE);
}

void OverlayWindow::show() {
    if (m_hwnd) {
        LOG_INFO("Showing overlay");
        updateControls();
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
        if (m_state == State::LOCKED) {
            SetFocus(m_passwordEdit);
            SetWindowTextW(m_passwordEdit, L"");
        }
        installKeyboardHook();
    }
}

void OverlayWindow::hide() {
    if (m_hwnd) {
        LOG_INFO("Hiding overlay");
        ShowWindow(m_hwnd, SW_HIDE);
        uninstallKeyboardHook();
    }
}

void OverlayWindow::destroy() {
    uninstallKeyboardHook();
    
    if (m_titleFont) { DeleteObject(m_titleFont); m_titleFont = nullptr; }
    if (m_normalFont) { DeleteObject(m_normalFont); m_normalFont = nullptr; }
    if (m_bgBrush) { DeleteObject(m_bgBrush); m_bgBrush = nullptr; }
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    
    if (m_hInstance) {
        UnregisterClassW(L"ParentalControlOverlay", m_hInstance);
    }
}

void OverlayWindow::setState(State state) {
    LOG_INFO("Setting overlay state: " + std::to_string(static_cast<int>(state)));
    m_state = state;
    
    if (state == State::HIDDEN) {
        hide();
    } else {
        updateControls();
        show();
    }
}

void OverlayWindow::setNotificationText(const std::string& text) {
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    m_notificationText.resize(len - 1);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &m_notificationText[0], len);
    
    if (m_messageLabel) {
        std::wstring msg = m_notificationText;
        if (!m_countdownText.empty()) {
            msg += L"\n" + m_countdownText;
        }
        SetWindowTextW(m_messageLabel, msg.c_str());
    }
}

void OverlayWindow::setCountdownText(const std::string& text) {
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    m_countdownText.resize(len - 1);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &m_countdownText[0], len);
    
    if (m_messageLabel) {
        std::wstring msg = m_notificationText;
        if (!m_countdownText.empty()) {
            msg += L"\n" + m_countdownText;
        }
        SetWindowTextW(m_messageLabel, msg.c_str());
    }
}

void OverlayWindow::setPasswordError(bool error) {
    m_passwordError = error;
}

void OverlayWindow::installKeyboardHook() {
    if (!s_keyboardHook) {
        s_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, m_hInstance, 0);
        if (s_keyboardHook) {
            LOG_INFO("Keyboard hook installed");
        }
    }
}

void OverlayWindow::uninstallKeyboardHook() {
    if (s_keyboardHook) {
        UnhookWindowsHookEx(s_keyboardHook);
        s_keyboardHook = nullptr;
        LOG_INFO("Keyboard hook uninstalled");
    }
}

LRESULT CALLBACK OverlayWindow::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && s_instance && s_instance->m_state != State::HIDDEN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        // Block system keys when locked
        bool block = false;
        
        // Block Alt+Tab, Alt+Esc, Alt+F4
        if ((GetAsyncKeyState(VK_MENU) & 0x8000)) {
            if (kbStruct->vkCode == VK_TAB || kbStruct->vkCode == VK_ESCAPE || kbStruct->vkCode == VK_F4) {
                block = true;
            }
        }

        // Block shift key alone (to prevent sticky keys)
        if (kbStruct->vkCode == VK_SHIFT) {
            block = true;
        }
        
        // Block Ctrl+Esc (Start menu)
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && kbStruct->vkCode == VK_ESCAPE) {
            block = true;
        }
        
        // Block Windows key
        if (kbStruct->vkCode == VK_LWIN || kbStruct->vkCode == VK_RWIN) {
            block = true;
        }
        
        // Block Ctrl+Alt+Del sequence (can't fully block, but block Ctrl+Shift+Esc for Task Manager)
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000) && kbStruct->vkCode == VK_ESCAPE) {
            block = true;
        }
        
        if (block) {
            return 1;  // Block the key
        }
    }
    
    return CallNextHookEx(s_keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK OverlayWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    OverlayWindow* pThis = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (OverlayWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hwnd = hwnd;
    } else {
        pThis = (OverlayWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (pThis) {
        return pThis->handleMessage(hwnd, uMsg, wParam, lParam);
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT OverlayWindow::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, m_passwordError ? COLOR_ERROR : COLOR_TEXT);
            SetBkColor(hdc, COLOR_BG);
            return (LRESULT)m_bgBrush;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            if (wmId == ID_SUBMIT_BUTTON) {
                wchar_t password[256] = {0};
                GetWindowTextW(m_passwordEdit, password, 256);
                
                // Convert to UTF-8
                int size = WideCharToMultiByte(CP_UTF8, 0, password, -1, nullptr, 0, nullptr, nullptr);
                std::string passwordStr(size - 1, 0);
                WideCharToMultiByte(CP_UTF8, 0, password, -1, &passwordStr[0], size, nullptr, nullptr);
                
                LOG_INFO("Password submitted");
                if (m_passwordCallback && !passwordStr.empty()) {
                    m_passwordCallback(passwordStr);
                }
                SetWindowTextW(m_passwordEdit, L"");
            }
            else if (wmId == ID_SHUTDOWN_BUTTON) {
                LOG_INFO("Shutdown clicked");
                if (m_shutdownCallback) m_shutdownCallback();
            }
            else if (wmId == ID_RESTART_BUTTON) {
                LOG_INFO("Restart clicked");
                if (m_restartCallback) m_restartCallback();
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_RETURN && m_state == State::LOCKED) {
                SendMessageW(hwnd, WM_COMMAND, ID_SUBMIT_BUTTON, 0);
                return 0;
            }
            break;
        }
        
        case WM_CLOSE:
            // Prevent closing when locked
            if (m_state != State::HIDDEN) {
                return 0;
            }
            break;
        
        case WM_DESTROY:
            m_running = false;
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int OverlayWindow::run() {
    MSG msg;
    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        // Handle dialog messages for tab navigation
        if (!IsDialogMessage(m_hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}
