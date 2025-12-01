#include "WebSocketClient.h"
#include "Logger.h"
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

WebSocketClient::WebSocketClient() {}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

void WebSocketClient::connect(const std::string& url, const std::string& deviceId, const std::string& deviceName) {
    LOG_INFO("Connecting to WebSocket: " + url);
    
    // Initialize Winsock if needed
    if (!m_wsaInitialized) {
        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (res != 0) {
            LOG_ERROR("WSAStartup failed with error: " + std::to_string(res));
            return;
        }
        m_wsaInitialized = true;
        LOG_INFO("WSAStartup succeeded");
    }

    m_webSocket.setUrl(url);
    
    // Set headers for device identification
    ix::WebSocketHttpHeaders headers;
    headers["X-Device-Id"] = deviceId;
    headers["X-Device-Name"] = deviceName;
    headers["Origin"] = "https://control.bravos.io.vn";
    headers["User-Agent"] = "ParentalControlAgent/1.0";
    m_webSocket.setExtraHeaders(headers);
    
    // Disable automatic reconnection - we handle it manually
    m_webSocket.disableAutomaticReconnection();
    
    // Set message callback
    m_webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        switch (msg->type) {
            case ix::WebSocketMessageType::Open:
                LOG_INFO("WebSocket connected");
                m_connected = true;
                if (m_connectionCallback) {
                    m_connectionCallback(true);
                }
                break;
                
            case ix::WebSocketMessageType::Close:
                LOG_INFO("WebSocket disconnected: " + msg->closeInfo.reason);
                m_connected = false;
                if (m_connectionCallback) {
                    m_connectionCallback(false);
                }
                break;
                
            case ix::WebSocketMessageType::Error:
                LOG_ERROR("WebSocket error: " + msg->errorInfo.reason);
                m_connected = false;
                break;
                
            case ix::WebSocketMessageType::Message:
                LOG_INFO("Received message: " + msg->str);
                if (m_messageCallback) {
                    m_messageCallback(msg->str);
                }
                break;
                
            default:
                break;
        }
    });
    
    m_webSocket.start();
}

bool WebSocketClient::waitForConnection(int timeoutSeconds) {
    LOG_INFO("Waiting for connection (timeout: " + std::to_string(timeoutSeconds) + "s)");
    
    auto start = std::chrono::steady_clock::now();
    while (!m_connected) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        
        if (elapsed >= timeoutSeconds) {
            LOG_WARNING("Connection timeout after " + std::to_string(timeoutSeconds) + " seconds");
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    LOG_INFO("Connected successfully");
    return true;
}

void WebSocketClient::disconnect() {
    LOG_INFO("Disconnecting WebSocket");
    m_webSocket.stop();
    m_connected = false;
    if (m_wsaInitialized) {
        WSACleanup();
        m_wsaInitialized = false;
        LOG_INFO("WSACleanup called");
    }
}

void WebSocketClient::sendMessage(const std::string& message) {
    if (m_connected) {
        LOG_INFO("Sending message: " + message);
        m_webSocket.send(message);
    } else {
        LOG_WARNING("Cannot send message, not connected: " + message);
    }
}

void WebSocketClient::setMessageCallback(MessageCallback callback) {
    m_messageCallback = callback;
}

void WebSocketClient::setConnectionCallback(ConnectionCallback callback) {
    m_connectionCallback = callback;
}

bool WebSocketClient::isConnected() const {
    return m_connected;
}
