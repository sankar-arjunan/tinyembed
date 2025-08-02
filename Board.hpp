#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>

class Board {
public:
    Board(const std::string& name, int gpioPinsCount);
    ~Board();

    void initUART(const std::string& rxPipePath, const int& bufferSize);
    void initTimer(int frequencyHz);
    void connectUART(const std::string& txPipePath);
    void disconnectUART();

    bool readGPIO(int pinNumber) const;
    int getGPIOCount();
    bool writeGPIO(int pinNumber, bool value);
    bool toggleGPIO(int pinNumber);

    bool sendByteUART(char byte);
    char readByteUART();
    bool isUARTBufferFull() const;
    void clearUARTBuffer();

    uint64_t getClockCycle() const;
    void delayClockCycles(int milliseconds);

    void printLog() const;
    void clearLog();

    void resetBoard(const std::string& name, int gpioPinsCount);

private:
    std::string boardName;
    std::string txPipeName;
    std::string rxPipeName;
    int rxPipeFd = -1;
    int txPipeFd = -1;

    // UART internal state
    std::queue<char> uartBuffer;
    size_t uartBufferSize;
    std::mutex uartMutex;
    std::thread uartRxThread;
    std::atomic<bool> uartRunning;

    // Timer internal state
    std::thread timerThread;
    std::atomic<bool> timerRunning;
    std::atomic<uint64_t> ticks;
    int timerFrequencyHz;

    // GPIO
    int gpioCount;
    std::vector<bool> gpioPins;

    // Logging
    std::vector<std::string> logEntries;

    // Internal helpers
    void uartReceiveLoop();
    void timerTickLoop();
};
