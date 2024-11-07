#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <winscard.h>

#define MAX_ATR_SIZE 33
#define MAX_READERNAME 256

enum class ReaderState {
    CONNECTING,
    CONNECTED,
    DISCONNECTED
};

class SmartCardReader {
public:
    SmartCardReader();
    ~SmartCardReader();
    void start();
    void stop();

private:
    void establishContext();
    void releaseContext();
    void listReaders();
    void connectToCard();
    void disconnectFromCard();
    void readCardUID();
    void monitorReader();

    SCARDCONTEXT applicationContext;
    LPTSTR reader;
    SCARDHANDLE connectionHandler;
    DWORD activeProtocol;
    std::atomic<ReaderState> state;
    std::thread monitorThread;
    std::atomic<bool> running;
};

#endif // SMARTCARDREADER_H







#include "SmartCardReader.h"

SmartCardReader::SmartCardReader()
    : reader(nullptr), activeProtocol(0), running(false), state(ReaderState::DISCONNECTED) {
    establishContext();
}

SmartCardReader::~SmartCardReader() {
    stop();
    releaseContext();
}

void SmartCardReader::start() {
    running = true;
    monitorThread = std::thread(&SmartCardReader::monitorReader, this);
}

void SmartCardReader::stop() {
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    disconnectFromCard();
}

void SmartCardReader::establishContext() {
    LONG status = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &applicationContext);
    if (status == SCARD_S_SUCCESS) {
        std::cout << "Context established" << std::endl;
    } else {
        std::cerr << "Establish context error: " << pcsc_stringify_error(status) << std::endl;
        exit(1);
    }
}

void SmartCardReader::releaseContext() {
    if (applicationContext) {
        LONG status = SCardReleaseContext(applicationContext);
        if (status == SCARD_S_SUCCESS) {
            std::cout << "Context released" << std::endl;
        } else {
            std::cerr << "Release context error: " << pcsc_stringify_error(status) << std::endl;
        }
    }
}

void SmartCardReader::listReaders() {
    DWORD readersSize = SCARD_AUTOALLOCATE;
    LONG status = SCardListReaders(applicationContext, nullptr, (LPTSTR)&reader, &readersSize);
    if (status == SCARD_S_SUCCESS) {
        std::wcout << L"Reader found: " << reader << std::endl;
    } else {
        std::cerr << "List reader error: " << pcsc_stringify_error(status) << std::endl;
        reader = nullptr;
    }
}

void SmartCardReader::connectToCard() {
    if (!reader) {
        listReaders();
    }

    if (reader) {
        activeProtocol = -1;
        state = ReaderState::CONNECTING;
        LONG status = SCardConnect(applicationContext, reader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &connectionHandler, &activeProtocol);
        if (status == SCARD_S_SUCCESS) {
            state = ReaderState::CONNECTED;
            std::cout << "Connected to card" << std::endl;
            readCardUID();
        } else {
            state = ReaderState::DISCONNECTED;
            std::cerr << "Card connection error: " << pcsc_stringify_error(status) << std::endl;
        }
    }
}

void SmartCardReader::disconnectFromCard() {
    if (state == ReaderState::CONNECTED) {
        LONG status = SCardDisconnect(connectionHandler, SCARD_LEAVE_CARD);
        if (status == SCARD_S_SUCCESS) {
            std::cout << "Disconnected from card" << std::endl;
        } else {
            std::cerr << "Card disconnection error: " << pcsc_stringify_error(status) << std::endl;
        }
    }
    state = ReaderState::DISCONNECTED;
}

void SmartCardReader::readCardUID() {
    BYTE atr[MAX_ATR_SIZE] = { 0 };
    DWORD atrLength = sizeof(atr);
    char readerName[MAX_READERNAME] = { 0 };
    DWORD readerLength = sizeof(readerName);
    DWORD readerState, readerProtocol;

    LONG status = SCardStatus(connectionHandler, readerName, &readerLength, &readerState, &readerProtocol, atr, &atrLength);
    if (status == SCARD_S_SUCCESS) {
        std::cout << "Reading card UID...\nATR: ";
        for (DWORD i = 0; i < atrLength; i++) {
            std::printf("%02X ", atr[i]);
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Get card information error: " << pcsc_stringify_error(status) << std::endl;
        disconnectFromCard();
        state = ReaderState::DISCONNECTED;
    }
}

void SmartCardReader::monitorReader() {
    while (running) {
        if (state == ReaderState::DISCONNECTED || state == ReaderState::CONNECTING) {
            connectToCard();
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}







#include "SmartCardReader.h"

int main() {
    SmartCardReader reader;
    reader.start();

    std::cout << "Press Enter to stop the application..." << std::endl;
    std::cin.get();

    reader.stop();
    return 0;
}