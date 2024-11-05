#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>

#ifdef _WIN32
#include <winscard.h>
#else
#include <winscard.h> // Provided by PC/SC Lite on Linux
#endif

class SmartCardReader
{
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();
    void startMonitoring();
    void stopMonitoring();

    // Event callbacks
    std::function<void()> onCardInserted;
    std::function<void()> onCardRemoved;
    std::function<void(const std::string &uid)> onCardDataRead;
    std::function<void(const std::string &error)> onErrorOccurred;

private:
    SCARDCONTEXT hContext;
    std::string readerName;
    std::string errorMessage;
    std::atomic<bool> isMonitoring;
    std::atomic<bool> cardPresent;
    std::thread monitoringThread;

    void monitorCardStatus();
    bool readCardData(std::string &cardUID);
    void listAvailableReaders();
};

#endif // SMARTCARDREADER_H










#include "SmartCardReader.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <tchar.h>
#endif

SmartCardReader::SmartCardReader()
    : hContext(0),
      isMonitoring(false),
      cardPresent(false)
{
}

SmartCardReader::~SmartCardReader()
{
    stopMonitoring();
    if (monitoringThread.joinable())
        monitoringThread.join();

    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = "Failed to establish context: 0x" + std::to_string(lReturn);
        if (onErrorOccurred) onErrorOccurred(errorMessage);
        return false;
    }

    // List available readers
    DWORD dwReaders = SCARD_AUTOALLOCATE;
    char *mszReaders = NULL;

#ifdef _WIN32
    // Use ANSI functions explicitly to avoid issues with UNICODE settings
    lReturn = SCardListReadersA(hContext, NULL, (LPSTR)&mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = "Failed to list readers: 0x" + std::to_string(lReturn);
        if (onErrorOccurred) onErrorOccurred(errorMessage);
        SCardReleaseContext(hContext);
        hContext = 0;
        return false;
    }

    // Parse the multi-string to get all reader names
    std::vector<std::string> readerList;
    char *pReader = mszReaders;
    while (*pReader != '\0') {
        readerList.push_back(pReader);
        pReader += strlen(pReader) + 1;
    }

    // Free the memory allocated by SCardListReaders
    SCardFreeMemory(hContext, mszReaders);

#else
    // On Linux, SCARD_AUTOALLOCATE is not supported
    // First, get the required buffer size
    lReturn = SCardListReaders(hContext, NULL, NULL, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS || dwReaders == 0) {
        errorMessage = "Failed to list readers or no readers available: 0x" +
                       std::to_string(lReturn);
        if (onErrorOccurred) onErrorOccurred(errorMessage);
        SCardReleaseContext(hContext);
        hContext = 0;
        return false;
    }

    // Allocate buffer for reader names
    mszReaders = new char[dwReaders];
    lReturn = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = "Failed to list readers: 0x" + std::to_string(lReturn);
        if (onErrorOccurred) onErrorOccurred(errorMessage);
        delete[] mszReaders;
        SCardReleaseContext(hContext);
        hContext = 0;
        return false;
    }

    // Parse the multi-string to get all reader names
    std::vector<std::string> readerList;
    char *pReader = mszReaders;
    while (*pReader != '\0') {
        readerList.push_back(pReader);
        pReader += strlen(pReader) + 1;
    }

    // Free the memory allocated for reader names
    delete[] mszReaders;
#endif

    if (readerList.empty()) {
        errorMessage = "No readers found.";
        if (onErrorOccurred) onErrorOccurred(errorMessage);
        SCardReleaseContext(hContext);
        hContext = 0;
        return false;
    }

    // Use the first reader (you can modify this to select a specific reader)
    readerName = readerList[0];

    // Optionally, print the list of readers
    std::cout << "Available readers:" << std::endl;
    for (const auto &name : readerList) {
        std::cout << " - " << name << std::endl;
    }

    return true;
}

void SmartCardReader::startMonitoring()
{
    isMonitoring = true;
    monitoringThread = std::thread(&SmartCardReader::monitorCardStatus, this);
}

void SmartCardReader::stopMonitoring()
{
    isMonitoring = false;
}

void SmartCardReader::monitorCardStatus()
{
#ifdef _WIN32
    SCARD_READERSTATEA readerState = { 0 };
#else
    SCARD_READERSTATE readerState = { 0 };
#endif

    readerState.szReader = readerName.c_str();
    readerState.dwCurrentState = SCARD_STATE_UNAWARE;

    while (isMonitoring) {
        // Copy the current event state to current state
        readerState.dwCurrentState = readerState.dwEventState;

#ifdef _WIN32
        LONG lReturn = SCardGetStatusChangeA(hContext, 500, &readerState, 1);
#else
        LONG lReturn = SCardGetStatusChange(hContext, 500, &readerState, 1);
#endif

        if (lReturn == SCARD_S_SUCCESS) {
            // Card inserted
            if ((readerState.dwEventState & SCARD_STATE_PRESENT) && !cardPresent.load()) {
                cardPresent = true;
                if (onCardInserted) onCardInserted();

                // Read data from the card
                std::string cardData;
                if (readCardData(cardData)) {
                    if (onCardDataRead) onCardDataRead(cardData);
                } else {
                    if (onErrorOccurred) onErrorOccurred(errorMessage);
                }
            }
            // Card removed
            else if (!(readerState.dwEventState & SCARD_STATE_PRESENT) && cardPresent.load()) {
                cardPresent = false;
                if (onCardRemoved) onCardRemoved();
            }
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = "SCardGetStatusChange failed: 0x" + std::to_string(lReturn);
            if (onErrorOccurred) onErrorOccurred(errorMessage);
            break;
        }
        // Sleep for a short period to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool SmartCardReader::readCardData(std::string &cardUID)
{
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;

#ifdef _WIN32
    LONG lReturn = SCardConnectA(hContext,
                                 readerName.c_str(),
                                 SCARD_SHARE_SHARED,
                                 SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                 &hCard,
                                 &dwActiveProtocol);
#else
    LONG lReturn = SCardConnect(hContext,
                                readerName.c_str(),
                                SCARD_SHARE_SHARED,
                                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                &hCard,
                                &dwActiveProtocol);
#endif

    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = "Failed to connect to card: 0x" + std::to_string(lReturn);
        return false;
    }

    // APDU command to get the UID
    BYTE apduCommand[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };

    SCARD_IO_REQUEST pioSendPci;
    if (dwActiveProtocol == SCARD_PROTOCOL_T0) {
        pioSendPci = *SCARD_PCI_T0;
    } else if (dwActiveProtocol == SCARD_PROTOCOL_T1) {
        pioSendPci = *SCARD_PCI_T1;
    } else {
        errorMessage = "Unknown active protocol.";
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        return false;
    }

    BYTE pbRecvBuffer[258];
    DWORD cbRecvLength = sizeof(pbRecvBuffer);

    lReturn = SCardTransmit(hCard,
                            &pioSendPci,
                            apduCommand,
                            sizeof(apduCommand),
                            NULL,
                            pbRecvBuffer,
                            &cbRecvLength);

    SCardDisconnect(hCard, SCARD_LEAVE_CARD);

    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = "Failed to transmit APDU: 0x" + std::to_string(lReturn);
        return false;
    }

    // Check for success status word (SW1 SW2)
    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
            // Success, extract UID
            int uidLength = cbRecvLength - 2;  // Exclude SW1 and SW2
            cardUID.assign((char*)pbRecvBuffer, uidLength);

            // Debug: Print UID
            std::cout << "Card UID: ";
            for (int i = 0; i < uidLength; ++i) {
                printf("%02X", (unsigned char)cardUID[i]);
            }
            std::cout << std::endl;

            return true;
        } else {
            char sw1_str[3], sw2_str[3];
            sprintf(sw1_str, "%02X", sw1);
            sprintf(sw2_str, "%02X", sw2);
            errorMessage = "APDU command failed with status: " + std::string(sw1_str) + " " + std::string(sw2_str);
            return false;
        }
    } else {
        errorMessage = "Invalid response from card.";
        return false;
    }
}











#include <iostream>
#include "SmartCardReader.h"

void hexPrint(const std::string &data)
{
    for (unsigned char c : data) {
        printf("%02X", c);
    }
}

int main()
{
    SmartCardReader reader;

    reader.onCardInserted = []() {
        std::cout << "Card inserted." << std::endl;
    };

    reader.onCardRemoved = []() {
        std::cout << "Card removed." << std::endl;
    };

    reader.onCardDataRead = [](const std::string &data) {
        std::cout << "Card UID: ";
        hexPrint(data);
        std::cout << std::endl;
    };

    reader.onErrorOccurred = [](const std::string &error) {
        std::cout << "Error: " << error << std::endl;
    };

    if (!reader.initialize()) {
        std::cout << "Initialization failed." << std::endl;
        return -1;
    }

    reader.startMonitoring();

    // Keep the main thread alive
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    reader.stopMonitoring();

    return 0;
}
