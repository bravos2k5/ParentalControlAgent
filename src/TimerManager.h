#pragma once

#include <functional>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>

class TimerManager {
public:
    TimerManager();
    ~TimerManager();

    void startGrantedTimer(int seconds, std::function<void()> onExpired);
    void startBlockTimer(int seconds, std::function<void()> onExpired);
    void startShutdownTimer(int seconds, std::function<void()> onExpired);
    
    void cancelGrantedTimer();
    void cancelBlockTimer();
    void cancelShutdownTimer();
    void cancelAllTimers();
    
    int getRemainingGrantedSeconds() const;
    int getRemainingBlockSeconds() const;
    int getRemainingShutdownSeconds() const;
    
    bool isGrantedActive() const;
    bool isBlockActive() const;
    bool isShutdownActive() const;

private:
    struct Timer {
        std::atomic<bool> active{false};
        std::atomic<int> remainingSeconds{0};
        std::thread timerThread;
        std::function<void()> callback;
    };

    void runTimer(Timer& timer, int seconds);
    void stopTimer(Timer& timer);

    Timer m_grantedTimer;
    Timer m_blockTimer;
    Timer m_shutdownTimer;
    
    std::mutex m_mutex;
};
