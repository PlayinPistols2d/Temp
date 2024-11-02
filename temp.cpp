#ifndef ACR122UREADER_H
#define ACR122UREADER_H

#include <winscard.h>
#include <QObject>
#include <QString>
#include <QByteArray>

class ACR122UReader : public QObject {
    Q_OBJECT

public:
    ACR122UReader();
    ~ACR122UReader();

    bool initializeReader();
    bool connectCard();
    QByteArray readCardData();
    void cleanup();
    QString getCardUID();

signals:
    void cardScanned(const QString &uid);

public slots:
    void checkForCard();

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
};

#endif // ACR122UREADER_H



#include "ACR122UReader.h"
#include <QDebug>

ACR122UReader::ACR122UReader() : hContext(0), hCard(0), dwActiveProtocol(0) {}

ACR122UReader::~ACR122UReader() {
    cleanup();
}

bool ACR122UReader::initializeReader() {
    LONG result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to establish context:" << result;
        return false;
    }
    return true;
}

bool ACR122UReader::connectCard() {
    DWORD dwReaderLen = SCARD_AUTOALLOCATE;
    LPSTR mszReaders = NULL;

    LONG result = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &dwReaderLen);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to list readers:" << result;
        return false;
    }

    result = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    SCardFreeMemory(hContext, mszReaders);

    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to connect to card:" << result;
        return false;
    }

    return true;
}

QByteArray ACR122UReader::readCardData() {
    BYTE pbRecvBuffer[258];
    DWORD dwRecvLength = sizeof(pbRecvBuffer);
    BYTE pbSendBuffer[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };
    DWORD dwSendLength = sizeof(pbSendBuffer);

    LONG result = SCardTransmit(hCard, SCARD_PCI_T0, pbSendBuffer, dwSendLength, NULL, pbRecvBuffer, &dwRecvLength);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to read card data:" << result;
        return QByteArray();
    }

    return QByteArray((char*)pbRecvBuffer, dwRecvLength);
}

void ACR122UReader::cleanup() {
    if (hCard) {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        hCard = 0;
    }
    if (hContext) {
        SCardReleaseContext(hContext);
        hContext = 0;
    }
}

QString ACR122UReader::getCardUID() {
    QByteArray cardData = readCardData();
    if (!cardData.isEmpty()) {
        return cardData.toHex().toUpper();
    }
    return QString();
}

void ACR122UReader::checkForCard() {
    if (!connectCard()) {
        qDebug() << "Waiting for a card...";
        return;
    }

    QString cardUID = getCardUID();
    if (!cardUID.isEmpty()) {
        emit cardScanned(cardUID);
    }

    cleanup();
}




#include <QCoreApplication>
#include <QTimer>
#include "ACR122UReader.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    ACR122UReader reader;
    if (!reader.initializeReader()) {
        return -1;
    }

    QObject::connect(&reader, &ACR122UReader::cardScanned, [](const QString &uid) {
        qDebug() << "Card UID:" << uid;
    });

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &reader, &ACR122UReader::checkForCard);
    timer.start(1000);  // Check every second

    return a.exec();
}
