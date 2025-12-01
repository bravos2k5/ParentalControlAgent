#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <ixwebsocket/IXWebSocket.h>

class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using ConnectionCallback = std::function<void(bool connected)>;

    WebSocketClient();
    ~WebSocketClient();

    void connect(const std::string& url, const std::string& deviceId, const std::string& deviceName);
    bool waitForConnection(int timeoutSeconds);  // Returns true if connected within timeout
    void disconnect();
    void sendMessage(const std::string& message);
    
    void setMessageCallback(MessageCallback callback);
    void setConnectionCallback(ConnectionCallback callback);
    
    bool isConnected() const;

private:
    ix::WebSocket m_webSocket;
    MessageCallback m_messageCallback;
    ConnectionCallback m_connectionCallback;
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_wsaInitialized{false};
};
