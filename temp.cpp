#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <string>
#include <vector>
#include <functional>

#ifdef _WIN32
#include <winscard.h>
#else
#include <pcsclite.h>
#include <winscard.h> // Provided by PC/SC Lite on Linux
#endif

class SmartCardReader
{
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();
    void listAvailableReaders();
    bool connectReader(const std::string &readerName);
    bool readCardUID(std::string &cardUID);

    // Event callbacks (optional, can be used if integrating into an event-driven application)
    std::function<void()> onCardInserted;
    std::function<void()> onCardRemoved;
    std::function<void(const std::string &uid)> onCardDataRead;
    std::function<void(const std::string &error)> onErrorOccurred;

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    std::string connectedReader;
    DWORD activeProtocol;

    void disconnectReader();
};

#endif // SMARTCARDREADER_H







#include "SmartCardReader.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <tchar.h>
#endif

SmartCardReader::SmartCardReader()
    : hContext(0),
      hCard(0),
      activeProtocol(0)
{
}

SmartCardReader::~SmartCardReader()
{
    disconnectReader();

    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        std::cerr << "Failed to establish context: " << pcsc_stringify_error(lReturn) << std::endl;
        return false;
    }

    return true;
}

void SmartCardReader::listAvailableReaders()
{
    LONG lReturn;
    char *mszReaders = NULL;
    DWORD dwReaders = SCARD_AUTOALLOCATE;

    lReturn = SCardListReaders(hContext, NULL, (char *)&mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        std::cerr << "Failed to list readers: " << pcsc_stringify_error(lReturn) << std::endl;
        return;
    }

    if (dwReaders == 0 || mszReaders == NULL || *mszReaders == '\0') {
        std::cerr << "No smart card readers found." << std::endl;
        SCardFreeMemory(hContext, mszReaders);
        return;
    }

    // Parse the multi-string to get all reader names
    std::vector<std::string> readerList;
    char *pReader = mszReaders;

    while (*pReader != '\0') {
        std::string readerName = pReader;
        readerList.push_back(readerName);
        pReader += readerName.length() + 1;
    }

    // Free the memory allocated by SCardListReaders
    SCardFreeMemory(hContext, mszReaders);

    // Print the list of readers
    std::cout << "Available smart card readers:" << std::endl;
    for (size_t i = 0; i < readerList.size(); ++i) {
        std::cout << " [" << i << "] " << readerList[i] << std::endl;
    }
}

bool SmartCardReader::connectReader(const std::string &readerName)
{
    disconnectReader();

    LONG lReturn = SCardConnect(hContext,
                                readerName.c_str(),
                                SCARD_SHARE_SHARED,
                                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                &hCard,
                                &activeProtocol);
    if (lReturn != SCARD_S_SUCCESS) {
        std::cerr << "Failed to connect to reader: " << pcsc_stringify_error(lReturn) << std::endl;
        return false;
    }

    connectedReader = readerName;
    return true;
}

bool SmartCardReader::readCardUID(std::string &cardUID)
{
    if (hCard == 0) {
        std::cerr << "No card connected." << std::endl;
        return false;
    }

    // APDU command to get the UID
    BYTE apduCommand[] = {0xFF, 0xCA, 0x00, 0x00, 0x00};

    SCARD_IO_REQUEST pioSendPci;
    if (activeProtocol == SCARD_PROTOCOL_T0) {
        pioSendPci = *SCARD_PCI_T0;
    } else if (activeProtocol == SCARD_PROTOCOL_T1) {
        pioSendPci = *SCARD_PCI_T1;
    } else {
        std::cerr << "Unknown active protocol." << std::endl;
        return false;
    }

    BYTE pbRecvBuffer[258];
    DWORD cbRecvLength = sizeof(pbRecvBuffer);

    LONG lReturn = SCardTransmit(hCard,
                                 &pioSendPci,
                                 apduCommand,
                                 sizeof(apduCommand),
                                 NULL,
                                 pbRecvBuffer,
                                 &cbRecvLength);

    if (lReturn != SCARD_S_SUCCESS) {
        std::cerr << "Failed to transmit APDU: " << pcsc_stringify_error(lReturn) << std::endl;
        return false;
    }

    // Check for success status word (SW1 SW2)
    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
            // Success, extract UID
            int uidLength = cbRecvLength - 2; // Exclude SW1 and SW2
            cardUID.assign((char *)pbRecvBuffer, uidLength);

            return true;
        } else {
            char sw1_str[3], sw2_str[3];
            sprintf(sw1_str, "%02X", sw1);
            sprintf(sw2_str, "%02X", sw2);
            std::cerr << "APDU command failed with status: " << sw1_str << " " << sw2_str << std::endl;
            return false;
        }
    } else {
        std::cerr << "Invalid response from card." << std::endl;
        return false;
    }
}

void SmartCardReader::disconnectReader()
{
    if (hCard != 0) {
        SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
        hCard = 0;
    }
}