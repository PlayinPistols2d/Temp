#ifndef ACR122U_CONNECTOR_H
#define ACR122U_CONNECTOR_H

#include <string>

class ACR122UConnector {
public:
    ACR122UConnector();
    ~ACR122UConnector();

    bool initialize();
    bool isConnected() const;
    std::string getLastError() const;

private:
    void* cardContext; // Handle for PC/SC context (void* for cross-platform use)
    std::string lastError;
};

#endif // ACR122U_CONNECTOR_H




#include "acr122u_connector.h"
#include <iostream>

#ifdef _WIN32
#include <winscard.h>
#else
#include <pcsclite.h>
#include <winscard.h>
#endif

ACR122UConnector::ACR122UConnector() : cardContext(nullptr), lastError("") {}

ACR122UConnector::~ACR122UConnector() {
    if (cardContext) {
        SCardReleaseContext(reinterpret_cast<SCARDCONTEXT>(cardContext));
    }
}

bool ACR122UConnector::initialize() {
    SCARDCONTEXT context;
    LONG result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &context);
    if (result != SCARD_S_SUCCESS) {
        lastError = "Failed to establish context: " + std::to_string(result);
        return false;
    }
    cardContext = reinterpret_cast<void*>(context);
    return true;
}

bool ACR122UConnector::isConnected() const {
    if (!cardContext) {
        return false;
    }

    SCARDHANDLE cardHandle;
    DWORD activeProtocol;
    LONG result = SCardConnect(
        reinterpret_cast<SCARDCONTEXT>(cardContext),
        "ACS ACR122U PICC Interface",
        SCARD_SHARE_SHARED,
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
        &cardHandle,
        &activeProtocol
    );

    if (result == SCARD_S_SUCCESS) {
        SCardDisconnect(cardHandle, SCARD_UNPOWER_CARD);
        return true;
    }

    return false;
}

std::string ACR122UConnector::getLastError() const {
    return lastError;
}






#include "acr122u_connector.h"
#include <iostream>

int main() {
    ACR122UConnector connector;
    if (connector.initialize()) {
        if (connector.isConnected()) {
            std::cout << "Smart card scanner is connected and valid.\n";
        } else {
            std::cout << "Smart card scanner is not connected.\n";
        }
    } else {
        std::cerr << "Initialization failed: " << connector.getLastError() << "\n";
    }

    return 0;
}