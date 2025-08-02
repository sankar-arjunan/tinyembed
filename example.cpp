#include "Board.hpp"
#include <iostream>
#include <thread>
#include <filesystem>

void printGPIO(Board& b) {
    for (int i = 0; i < b.getGPIOCount(); ++i)
        std::cout << b.readGPIO(7 - i);
    std::cout << "\n";
}

int main() {
    std::filesystem::remove("A_RX");
    std::filesystem::remove("B_RX");

    Board A("BoardA", 8);
    Board B("BoardB", 8);

    A.initTimer(2);
    A.initUART("A_RX", 16);
    B.initUART("B_RX", 16);
    A.connectUART("B_RX");
    B.connectUART("A_RX");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    B.sendByteUART('Z');
    A.delayClockCycles(1);

    char ch = A.readByteUART();
    for (int i = 0; i < 8; ++i)
        A.writeGPIO(i, (ch >> i) & 1);

    A.delayClockCycles(1);

    for (int i = 0; i < 8; ++i)
        A.toggleGPIO(i);

    A.sendByteUART('K');
    A.delayClockCycles(1);
    char resp = B.readByteUART();

    for (int i = 0; i < 8; ++i)
        B.writeGPIO(i, (resp >> i) & 1);

    A.clearUARTBuffer();
    B.clearUARTBuffer();

    std::cout << "A GPIO: "; printGPIO(A);
    std::cout << "B GPIO: "; printGPIO(B);

    A.printLog();
    B.printLog();

    A.disconnectUART();
    B.disconnectUART();
    std::filesystem::remove("A_RX");
    std::filesystem::remove("B_RX");
    return 0;
}
