#include "SmartCardReader.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
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

    DWORD dwReaders = SCARD_AUTOALLOCATE;
    char* mszReaders = nullptr;

    // Use the appropriate function based on the platform
#ifdef _WIN32
    LONG rv = SCardListReadersA((SCARDCONTEXT)hContext, NULL, (LPSTR)&mszReaders, &dwReaders);
#else
    LONG rv = SCardListReaders((SCARDCONTEXT)hContext, NULL, (LPSTR)&mszReaders, &dwReaders);
#endif

    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to list readers: " << rv << std::endl;
        return readers;
    }

    // Parse the multi-string of reader names
    char* pReader = mszReaders;
    while (*pReader)
    {
        readers.emplace_back(pReader);
        pReader += strlen(pReader) + 1;
    }

    // Free the allocated memory
    SCardFreeMemory((SCARDCONTEXT)hContext, mszReaders);

    return readers;
}

bool SmartCardReader::connect(const std::string& readerName)
{
    if (!hContext)
    {
        std::cerr << "Context not established." << std::endl;
        return false;
    }

    // Use the appropriate function based on the platform
#ifdef _WIN32
    LONG rv = SCardConnectA((SCARDCONTEXT)hContext, readerName.c_str(),
                            SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                            (SCARDHANDLE*)&hCard, &dwActiveProtocol);
#else
    LONG rv = SCardConnect((SCARDCONTEXT)hContext, readerName.c_str(),
                           SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                           (SCARDHANDLE*)&hCard, &dwActiveProtocol);
#endif

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

    // Use the appropriate function based on the platform
#ifdef _WIN32
    LONG rv = SCardStatusA((SCARDHANDLE)hCard, NULL, NULL, &dwState,
                           &dwProtocol, pbAtr, &dwAtrLen);
#else
    LONG rv = SCardStatus((SCARDHANDLE)hCard, NULL, NULL, &dwState,
                          &dwProtocol, pbAtr, &dwAtrLen);
#endif

    if (rv != SCARD_S_SUCCESS)
    {
        std::cerr << "Failed to get card status: " << rv << std::endl;
        return false;
    }

    return (dwState & SCARD_PRESENT) != 0;
}