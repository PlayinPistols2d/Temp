#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <winscard.h>
#else
#include <pcsclite.h>
#include <winscard.h>
#endif

class SmartCardReader
{
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();
    std::vector<std::string> listReaders();
    bool connect(const std::string& readerName);
    void disconnect();
    bool readCardUID(std::string& uid);

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
};

#endif // SMARTCARDREADER_H







#include "SmartCardReader.h"
#include <iostream>
#include <cstring>

SmartCardReader::SmartCardReader()
    : hContext(0), hCard(0), dwActiveProtocol(0)
{
}

SmartCardReader::~SmartCardReader()
{
    disconnect();
    if (hContext != 0)
    {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardEstablishContext failed: " << result << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> SmartCardReader::listReaders()
{
    std::vector<std::string> readers;
    LONG result;
    char* mszReaders = NULL;
    DWORD dwReaders = SCARD_AUTOALLOCATE;

    result = SCardListReaders(hContext, NULL, (char*)&mszReaders, &dwReaders);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardListReaders failed: " << result << std::endl;
        return readers;
    }

    char* pReader = mszReaders;
    while (*pReader)
    {
        readers.push_back(std::string(pReader));
        pReader += strlen(pReader) + 1;
    }

    SCardFreeMemory(hContext, mszReaders);
    return readers;
}

bool SmartCardReader::connect(const std::string& readerName)
{
    disconnect();

#ifdef _WIN32
    LPCSTR szReader = readerName.c_str();
#else
    const char* szReader = readerName.c_str();
#endif

    LONG result = SCardConnect(hContext, szReader, SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardConnect failed: " << result << std::endl;
        return false;
    }
    return true;
}

void SmartCardReader::disconnect()
{
    if (hCard != 0)
    {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        hCard = 0;
    }
}

bool SmartCardReader::readCardUID(std::string& uid)
{
    if (hCard == 0)
    {
        std::cerr << "No card connected." << std::endl;
        return false;
    }

    BYTE pbSendBuffer[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };
    BYTE pbRecvBuffer[256];
    DWORD dwRecvLength = sizeof(pbRecvBuffer);

    SCARD_IO_REQUEST ioRequest;
    ioRequest.dwProtocol = dwActiveProtocol;
    ioRequest.cbPciLength = sizeof(SCARD_IO_REQUEST);

    LONG result = SCardTransmit(hCard, &ioRequest, pbSendBuffer, sizeof(pbSendBuffer),
                                NULL, pbRecvBuffer, &dwRecvLength);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardTransmit failed: " << result << std::endl;
        return false;
    }

    if (dwRecvLength >= 2)
    {
        BYTE sw1 = pbRecvBuffer[dwRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[dwRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00)
        {
            uid.assign((char*)pbRecvBuffer, dwRecvLength - 2);
            return true;
        }
        else
        {
            std::cerr << "Unexpected response status: " << std::hex << (int)sw1 << " " << (int)sw2 << std::dec << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Invalid response length." << std::endl;
        return false;
    }
}







#include <iostream>
#include "SmartCardReader.h"

void printHex(const std::string& data)
{
    for (unsigned char c : data)
    {
        printf("%02X", c);
    }
    printf("\n");
}

int main()
{
    SmartCardReader reader;
    if (!reader.initialize())
    {
        return -1;
    }

    auto readers = reader.listReaders();
    if (readers.empty())
    {
        std::cerr << "No smart card readers found." << std::endl;
        return -1;
    }

    std::cout << "Available readers:" << std::endl;
    for (size_t i = 0; i < readers.size(); ++i)
    {
        std::cout << "[" << i << "] " << readers[i] << std::endl;
    }

    int choice = 0;
    if (readers.size() > 1)
    {
        std::cout << "Select a reader: ";
        std::cin >> choice;
        if (choice < 0 || choice >= (int)readers.size())
        {
            std::cerr << "Invalid choice." << std::endl;
            return -1;
        }
    }

    if (!reader.connect(readers[choice]))
    {
        return -1;
    }

    std::string uid;
    if (reader.readCardUID(uid))
    {
        std::cout << "Card UID: ";
        printHex(uid);
    }
    else
    {
        std::cerr << "Failed to read card UID." << std::endl;
    }

    reader.disconnect();
    return 0;
}






std::vector<std::string> SmartCardReader::listReaders()
{
    std::vector<std::string> readers;
    LONG result;
    LPTSTR mszReaders = NULL;
    DWORD dwReaders = SCARD_AUTOALLOCATE;

    result = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardListReaders failed: " << result << std::endl;
        return readers;
    }

#ifdef _WIN32
    LPTSTR pReader = mszReaders;
    while (*pReader)
    {
        std::wstring wReaderName(pReader);
        std::string readerName(wReaderName.begin(), wReaderName.end());
        readers.push_back(readerName);
        pReader += wcslen(pReader) + 1;
    }
#else
    char* pReader = mszReaders;
    while (*pReader)
    {
        readers.push_back(std::string(pReader));
        pReader += strlen(pReader) + 1;
    }
#endif

    SCardFreeMemory(hContext, mszReaders);
    return readers;
}






bool SmartCardReader::connect(const std::string& readerName)
{
    disconnect();

#ifdef _WIN32
#ifdef UNICODE
    // Convert std::string to std::wstring
    std::wstring wReaderName(readerName.begin(), readerName.end());
    LPCWSTR szReader = wReaderName.c_str();
#else
    LPCSTR szReader = readerName.c_str();
#endif
#else
    const char* szReader = readerName.c_str();
#endif

    LONG result = SCardConnect(hContext, szReader, SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardConnect failed: " << result << std::endl;
        return false;
    }
    return true;
}