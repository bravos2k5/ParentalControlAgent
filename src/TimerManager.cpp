#include "TimerManager.h"
#include "Logger.h"

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {
    cancelAllTimers();
}

void TimerManager::runTimer(Timer& timer, int seconds) {
    timer.remainingSeconds = seconds;
    timer.active = true;
    
    while (timer.active && timer.remainingSeconds > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (timer.active) {
            timer.remainingSeconds--;
        }
    }
    
    if (timer.active && timer.remainingSeconds <= 0) {
        timer.active = false;
        if (timer.callback) {
            timer.callback();
        }
    }
}

void TimerManager::stopTimer(Timer& timer) {
    timer.active = false;
    timer.remainingSeconds = 0;
    if (timer.timerThread.joinable()) {
        timer.timerThread.join();
    }
}

void TimerManager::startGrantedTimer(int seconds, std::function<void()> onExpired) {
    std::lock_guard<std::mutex> lock(m_mutex);
    stopTimer(m_grantedTimer);
    
    LOG_INFO("Starting granted timer for " + std::to_string(seconds) + " seconds");
    m_grantedTimer.callback = onExpired;
    m_grantedTimer.timerThread = std::thread(&TimerManager::runTimer, this, 
                                              std::ref(m_grantedTimer), seconds);
}

void TimerManager::startBlockTimer(int seconds, std::function<void()> onExpired) {
    std::lock_guard<std::mutex> lock(m_mutex);
    stopTimer(m_blockTimer);
    
    // Minimum 5 seconds
    if (seconds < 5) {
        seconds = 5;
    }
    
    LOG_INFO("Starting block timer for " + std::to_string(seconds) + " seconds");
    m_blockTimer.callback = onExpired;
    m_blockTimer.timerThread = std::thread(&TimerManager::runTimer, this, 
                                            std::ref(m_blockTimer), seconds);
}

void TimerManager::startShutdownTimer(int seconds, std::function<void()> onExpired) {
    std::lock_guard<std::mutex> lock(m_mutex);
    stopTimer(m_shutdownTimer);
    
    LOG_INFO("Starting shutdown timer for " + std::to_string(seconds) + " seconds");
    m_shutdownTimer.callback = onExpired;
    m_shutdownTimer.timerThread = std::thread(&TimerManager::runTimer, this, 
                                               std::ref(m_shutdownTimer), seconds);
}

void TimerManager::cancelGrantedTimer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    LOG_INFO("Cancelling granted timer");
    stopTimer(m_grantedTimer);
}

void TimerManager::cancelBlockTimer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    LOG_INFO("Cancelling block timer");
    stopTimer(m_blockTimer);
}

void TimerManager::cancelShutdownTimer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    LOG_INFO("Cancelling shutdown timer");
    stopTimer(m_shutdownTimer);
}

void TimerManager::cancelAllTimers() {
    cancelGrantedTimer();
    cancelBlockTimer();
    cancelShutdownTimer();
}

int TimerManager::getRemainingGrantedSeconds() const {
    return m_grantedTimer.remainingSeconds;
}

int TimerManager::getRemainingBlockSeconds() const {
    return m_blockTimer.remainingSeconds;
}

int TimerManager::getRemainingShutdownSeconds() const {
    return m_shutdownTimer.remainingSeconds;
}

bool TimerManager::isGrantedActive() const {
    return m_grantedTimer.active;
}

bool TimerManager::isBlockActive() const {
    return m_blockTimer.active;
}

bool TimerManager::isShutdownActive() const {
    return m_shutdownTimer.active;
}
