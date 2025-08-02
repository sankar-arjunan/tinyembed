#include "Board.hpp"
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Board::Board(const std::string& name, int gpioPinsCount)
    : boardName(name)
    , gpioCount(gpioPinsCount)
    , gpioPins(gpioPinsCount, false)
    , ticks(0)
    , uartRunning(false)
    , timerRunning(false) 
    {
        logEntries.push_back("Board created: " + name);
}
Board::~Board() {
    disconnectUART();

    uartRunning = false;

    if (uartRxThread.joinable()) uartRxThread.join();
    if (rxPipeFd != -1) {
        close(rxPipeFd);
        rxPipeFd = -1;
    }

    timerRunning = false;
    if (timerThread.joinable()) timerThread.join();
}


void Board::initUART(const std::string& rxPipePath, const int& bufferSize) {
    if (rxPipeFd != -1) {
        logEntries.push_back("UART RX already initialized.");
        return;
    }

    rxPipeName = rxPipePath;
    uartBufferSize = bufferSize;

    mkfifo(rxPipePath.c_str(), 0666);
    rxPipeFd = open(rxPipePath.c_str(), O_RDONLY | O_NONBLOCK);

    if (rxPipeFd == -1) {
        logEntries.push_back("Failed to open RX pipe: " + rxPipePath);
        return;
    }

    uartRunning = true;
    uartRxThread = std::thread(&Board::uartReceiveLoop, this);
    logEntries.push_back("UART Initialized with RX FD = " + std::to_string(rxPipeFd));
}



void Board::initTimer(int frequencyHz) {
    timerFrequencyHz = frequencyHz;
    timerRunning = true;
    timerThread = std::thread(&Board::timerTickLoop, this);
    logEntries.push_back("Timer started with frequency " + std::to_string(timerFrequencyHz) + " MHz");
}

void Board::connectUART(const std::string& txPipePath) {
    uartRunning = true;
    txPipeName = txPipePath;
    txPipeFd = open(txPipePath.c_str(), O_WRONLY | O_NONBLOCK);
    logEntries.push_back("UART CONNECTED TO = " + std::to_string(txPipeFd));
}

void Board::disconnectUART() {
    if (txPipeFd != -1) {
        close(txPipeFd);
        txPipeFd = -1;
    }
    txPipeName.clear();
    logEntries.push_back("UART disconnected.");
}


bool Board::readGPIO(int pinNumber) const {
    return gpioPins[pinNumber];
}

int Board::getGPIOCount(){
    return gpioCount;
}

bool Board::writeGPIO(int pinNumber, bool value) {
    gpioPins[pinNumber] = value;
    return gpioPins[pinNumber];
}

bool Board::toggleGPIO(int pinNumber) {
    gpioPins[pinNumber] = !gpioPins[pinNumber];
    return gpioPins[pinNumber];
}

bool Board::sendByteUART(char byte) {
    if (txPipeFd == -1) {
        logEntries.push_back("UART send failed: TX pipe not open.");
        return false;
    }

    ssize_t result = write(txPipeFd, &byte, 1);
    if (result == 1) {
        logEntries.push_back("UART sent byte: " + std::to_string(static_cast<int>(byte)));
        return true;
    } else {
        logEntries.push_back("UART send failed: write() error.");
        return false;
    }
}

char Board::readByteUART() {
    std::lock_guard<std::mutex> lock(uartMutex);

    if (!uartBuffer.empty()) {
        char byte = uartBuffer.front();
        uartBuffer.pop();
        return byte;
    } else {
        return '\0';
    }
}


bool Board::isUARTBufferFull() const {
    return uartBuffer.size() >= uartBufferSize;
}


void Board::clearUARTBuffer() {
    std::lock_guard<std::mutex> lock(uartMutex);
    std::queue<char> empty;
    std::swap(uartBuffer, empty);
    logEntries.push_back("UART buffer cleared.");
}

uint64_t Board::getClockCycle() const {
    return ticks.load();
}

void Board::delayClockCycles(int milliseconds) {
    uint64_t target = ticks.load() + milliseconds;
    while (ticks.load() < target) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Board::printLog() const {
    for (const std::string& log : logEntries) {
        std::cout << log << std::endl;
    }
}


void Board::clearLog() {
    logEntries.clear();
}

void Board::resetBoard(const std::string& name, int gpioPinsCount) {
    boardName = name;
    gpioCount = gpioPinsCount;
    gpioPins.assign(gpioPinsCount, false);
    logEntries.clear();
    uartBuffer = std::queue<char>();
    ticks = 0;
    logEntries.push_back("Board reset: " + name);
}

void Board::uartReceiveLoop() {
    char buffer;
    while (uartRunning) {
        if (rxPipeFd != -1) {
            ssize_t bytesRead = read(rxPipeFd, &buffer, 1);
            if (bytesRead > 0) {
                std::lock_guard<std::mutex> lock(uartMutex);
                if (uartBuffer.size() < uartBufferSize) {
                    uartBuffer.push(buffer);
                } else {
                    logEntries.push_back("UART RX buffer overflow");
                }
            } else if (bytesRead == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                logEntries.push_back("UART RX read error.");
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


void Board::timerTickLoop() {
    uint64_t interval = 1000 / timerFrequencyHz;
    while (timerRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        ++ticks;
    }
}
