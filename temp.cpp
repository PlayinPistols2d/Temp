#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <string>
#include <vector>

class SmartCardReader
{
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();
    std::vector<std::string> listReaders();
    bool connect(const std::string& readerName);
    bool isValid();

private:
    void* hContext;
    void* hCard;
    unsigned long dwActiveProtocol;
};

#endif // SMARTCARDREADER_H






#include "SmartCardReader.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <winscard.h>
#else
#include <pcsclite.h>
#include <winscard.h>
#endif

SmartCardReader::SmartCardReader()
    : hContext(nullptr), hCard(nullptr), dwActiveProtocol(0)
{
}

SmartCardReader::~SmartCardReader()
{
    if (hCard)
    {
        SCardDisconnect((SCARDHANDLE)hCard, SCARD_LEAVE_CARD);
    }
    if (hContext)
    {
        SCardReleaseContext((SCARDCONTEXT)hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL,
                                    (SCARDCONTEXT*)&hContext);
    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to establish context: " << rv << std::endl;
        hContext = nullptr;
        return false;
    }
    return true;
}

std::vector<std::string> SmartCardReader::listReaders()
{
    std::vector<std::string> readers;

    if (!hContext)
    {
        std::cerr << "Context not established." << std::endl;
        return readers;
    }

    DWORD dwReaders = 0;
    LONG rv = SCardListReaders((SCARDCONTEXT)hContext, NULL, NULL, &dwReaders);
    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to get readers list size: " << rv << std::endl;
        return readers;
    }

    if (dwReaders == 0)
    {
        std::cerr << "No readers available." << std::endl;
        return readers;
    }

    char* mszReaders = new char[dwReaders];
    rv = SCardListReaders((SCARDCONTEXT)hContext, NULL, mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to list readers: " << rv << std::endl;
        delete[] mszReaders;
        return readers;
    }

    char* pReader = mszReaders;
    while (*pReader)
    {
        readers.push_back(std::string(pReader));
        pReader += strlen(pReader) + 1;
    }

    delete[] mszReaders;
    return readers;
}

bool SmartCardReader::connect(const std::string& readerName)
{
    if (!hContext)
    {
        std::cerr << "Context not established." << std::endl;
        return false;
    }

    LONG rv = SCardConnect((SCARDCONTEXT)hContext, readerName.c_str(),
                           SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                           (SCARDHANDLE*)&hCard, &dwActiveProtocol);
    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to connect to reader: " << rv << std::endl;
        hCard = nullptr;
        return false;
    }

    return true;
}

bool SmartCardReader::isValid()
{
    if (!hCard)
    {
        std::cerr << "Not connected to a reader." << std::endl;
        return false;
    }

    DWORD dwState = 0;
    DWORD dwProtocol = 0;
    BYTE pbAtr[33];
    DWORD dwAtrLen = sizeof(pbAtr);

    LONG rv = SCardStatus((SCARDHANDLE)hCard, NULL, NULL, &dwState,
                          &dwProtocol, pbAtr, &dwAtrLen);
    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to get card status: " << rv << std::endl;
        return false;
    }

    return (dwState & SCARD_PRESENT) != 0;
}





#include "SmartCardReader.h"
#include <iostream>

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
        std::cerr << "No readers found." << std::endl;
        return -1;
    }

    std::cout << "Available readers:" << std::endl;
    for (const auto& name : readers)
    {
        std::cout << " - " << name << std::endl;
    }

    // Connect to the first reader
    if (!reader.connect(readers[0]))
    {
        return -1;
    }

    if (reader.isValid())
    {
        std::cout << "Card is present and valid." << std::endl;
    }
    else
    {
        std::cout << "Card is not valid or not present." << std::endl;
    }

    return 0;
}