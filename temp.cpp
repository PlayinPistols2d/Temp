#include <winscard.h>
#include <iostream>
#include <QDebug>

void readCardData() {
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    LONG result;

    // Establish a context
    result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to establish context";
        return;
    }

    // Connect to the card
    result = SCardConnect(hContext, "Your Reader Name", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to connect to card";
        SCardReleaseContext(hContext);
        return;
    }

    // Buffer for card data (example APDU command, modify as needed)
    BYTE sendBuffer[] = { 0x00, 0xA4, 0x04, 0x00, 0x0A };
    BYTE recvBuffer[256];
    DWORD recvLength = sizeof(recvBuffer);

    // Send APDU command
    result = SCardTransmit(hCard, SCARD_PCI_T0, sendBuffer, sizeof(sendBuffer), NULL, recvBuffer, &recvLength);
    if (result == SCARD_S_SUCCESS) {
        QByteArray cardData(reinterpret_cast<char*>(recvBuffer), recvLength);
        qDebug() << "Card Data:" << cardData.toHex();  // Output card data as hex
    } else {
        qDebug() << "Failed to transmit APDU";
    }

    // Clean up
    SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
    SCardReleaseContext(hContext);
}