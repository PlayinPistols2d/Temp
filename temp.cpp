#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>

class SmartCardReader
{
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();              // Establishes context and lists readers
    bool connect();                 // Connects to the smart card
    void disconnect();              // Disconnects from the smart card
    void release();                 // Releases the context

    bool sendAPDU(const QByteArray &apduCommand, QByteArray &apduResponse);

    QString getReaderName() const;
    QString getErrorMessage() const;

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;
    QString readerName;
    QString errorMessage;
    bool isConnected;
};

#endif // SMARTCARDREADER_H


#include "SmartCardReader.h"
#include <QDebug>

SmartCardReader::SmartCardReader()
    : hContext(0),
      hCard(0),
      dwActiveProtocol(0),
      isConnected(false)
{
}

SmartCardReader::~SmartCardReader()
{
    disconnect();
    release();
}

bool SmartCardReader::initialize()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: %1").arg(lReturn);
        return false;
    }

    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR mszReaders = NULL;
    lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to list readers: %1").arg(lReturn);
        SCardReleaseContext(hContext);
        hContext = 0;
        return false;
    }

    // Assuming the first reader is the target reader
    readerName = QString::fromWCharArray(mszReaders);

    // Free the memory allocated by SCardListReaders
    SCardFreeMemory(hContext, mszReaders);

    return true;
}

bool SmartCardReader::connect()
{
    if (hContext == 0) {
        errorMessage = "Context not established.";
        return false;
    }

    LONG lReturn = SCardConnect(hContext,
                                (LPCWSTR)readerName.utf16(),
                                SCARD_SHARE_SHARED,
                                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                &hCard,
                                &dwActiveProtocol);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to connect to card: %1").arg(lReturn);
        return false;
    }

    isConnected = true;
    return true;
}

void SmartCardReader::disconnect()
{
    if (isConnected) {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        hCard = 0;
        isConnected = false;
    }
}

void SmartCardReader::release()
{
    if (hContext != 0) {
        SCardReleaseContext(hContext);
        hContext = 0;
    }
}

bool SmartCardReader::sendAPDU(const QByteArray &apduCommand, QByteArray &apduResponse)
{
    if (!isConnected) {
        errorMessage = "Not connected to the card.";
        return false;
    }

    SCARD_IO_REQUEST pioSendPci;
    if (dwActiveProtocol == SCARD_PROTOCOL_T0) {
        pioSendPci = *SCARD_PCI_T0;
    } else if (dwActiveProtocol == SCARD_PROTOCOL_T1) {
        pioSendPci = *SCARD_PCI_T1;
    } else {
        errorMessage = "Unknown active protocol.";
        return false;
    }

    BYTE pbRecvBuffer[256];
    DWORD cbRecvLength = sizeof(pbRecvBuffer);

    LONG lReturn = SCardTransmit(hCard,
                                 &pioSendPci,
                                 (LPCBYTE)apduCommand.constData(),
                                 apduCommand.size(),
                                 NULL,
                                 pbRecvBuffer,
                                 &cbRecvLength);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to transmit APDU: %1").arg(lReturn);
        return false;
    }

    apduResponse = QByteArray(reinterpret_cast<char*>(pbRecvBuffer), cbRecvLength);
    return true;
}

QString SmartCardReader::getReaderName() const
{
    return readerName;
}

QString SmartCardReader::getErrorMessage() const
{
    return errorMessage;
}


#include <QCoreApplication>
#include <QDebug>
#include "SmartCardReader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SmartCardReader reader;

    // Initialize the reader
    if (!reader.initialize()) {
        qDebug() << "Initialization Error:" << reader.getErrorMessage();
        return -1;
    }
    qDebug() << "Reader Name:" << reader.getReaderName();

    // Connect to the card
    if (!reader.connect()) {
        qDebug() << "Connection Error:" << reader.getErrorMessage();
        return -1;
    }
    qDebug() << "Connected to the card successfully.";

    // Prepare an APDU command (example: SELECT FILE command)
    QByteArray apduCommand = QByteArray::fromHex("00A4040000"); // Replace with your APDU
    QByteArray apduResponse;

    // Send the APDU command
    if (!reader.sendAPDU(apduCommand, apduResponse)) {
        qDebug() << "APDU Transmission Error:" << reader.getErrorMessage();
        reader.disconnect();
        reader.release();
        return -1;
    }

    // Display the APDU response
    qDebug() << "APDU Response:" << apduResponse.toHex().toUpper();

    // Disconnect and release resources
    reader.disconnect();
    reader.release();

    return a.exec();
}
