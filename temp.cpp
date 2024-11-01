#ifndef NFCREADER_H
#define NFCREADER_H

#include <QObject>
#include <winscard.h>

class NFCReader : public QObject {
    Q_OBJECT

public:
    explicit NFCReader(QObject *parent = nullptr);
    ~NFCReader();

    bool initialize();
    QString readCard();

signals:
    void cardRead(QString cardData);

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    bool isConnected;

    bool connectToCard();
    void disconnectFromCard();
};

#endif // NFCREADER_H





#include "NFCReader.h"
#include <QDebug>

NFCReader::NFCReader(QObject *parent)
    : QObject(parent), isConnected(false), hContext(0), hCard(0) {
}

NFCReader::~NFCReader() {
    disconnectFromCard();
    if (hContext) {
        SCardReleaseContext(hContext);
    }
}

bool NFCReader::initialize() {
    LONG result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to establish context:" << result;
        return false;
    }
    return true;
}

bool NFCReader::connectToCard() {
    if (!hContext) return false;

    DWORD dwActiveProtocol;
    LONG result = SCardConnect(hContext, "ACS ACR122U PICC Interface", SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to connect to the card:" << result;
        return false;
    }
    isConnected = true;
    return true;
}

void NFCReader::disconnectFromCard() {
    if (isConnected && hCard) {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        isConnected = false;
    }
}

QString NFCReader::readCard() {
    if (!connectToCard()) return QString();

    BYTE cmd[] = {0xFF, 0xCA, 0x00, 0x00, 0x00};  // Example: Get UID for ISO14443-4 cards
    BYTE response[256];
    DWORD responseLen = sizeof(response);

    LONG result = SCardTransmit(hCard, SCARD_PCI_T1, cmd, sizeof(cmd), NULL, response, &responseLen);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to read card:" << result;
        disconnectFromCard();
        return QString();
    }

    disconnectFromCard();
    QByteArray cardData(reinterpret_cast<char*>(response), responseLen);
    QString cardDataHex = cardData.toHex();
    emit cardRead(cardDataHex);
    return cardDataHex;
}





#include "NFCReader.h"

// Create and initialize NFC reader
NFCReader *reader = new NFCReader(this);
if (reader->initialize()) {
    QString cardData = reader->readCard();
    qDebug() << "Read Card Data:" << cardData;
}
