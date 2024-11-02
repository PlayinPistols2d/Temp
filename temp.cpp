#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

class SmartCardReader : public QThread
{
    Q_OBJECT
public:
    explicit SmartCardReader(QObject *parent = nullptr);
    ~SmartCardReader();

    bool initialize();
    void run() override;  // Overridden from QThread

    void stopMonitoring();

signals:
    void cardInserted();
    void cardRemoved();
    void cardDataRead(const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    SCARDCONTEXT hContext;
    QString readerName;
    QString errorMessage;
    std::atomic<bool> isMonitoring;

    void monitorCardStatus();
    bool readCardData(QByteArray &cardData);
};

#endif // SMARTCARDREADER_H



#include "SmartCardReader.h"
#include <QDebug>

SmartCardReader::SmartCardReader(QObject *parent)
    : QThread(parent),
      hContext(0),
      isMonitoring(false)
{
}

SmartCardReader::~SmartCardReader()
{
    stopMonitoring();
    wait();  // Wait for the thread to finish
    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: %1").arg(lReturn);
        emit errorOccurred(errorMessage);
        return false;
    }

    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR mszReaders = NULL;
    lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to list readers: %1").arg(lReturn);
        emit errorOccurred(errorMessage);
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

void SmartCardReader::run()
{
    isMonitoring = true;
    monitorCardStatus();
}

void SmartCardReader::stopMonitoring()
{
    isMonitoring = false;
}

void SmartCardReader::monitorCardStatus()
{
    SCARD_READERSTATE readerState;
    readerState.szReader = (LPCWSTR)readerName.utf16();
    readerState.pvUserData = NULL;
    readerState.dwCurrentState = SCARD_STATE_UNAWARE;
    readerState.dwEventState = 0;
    readerState.cbAtr = 0;

    while (isMonitoring) {
        LONG lReturn = SCardGetStatusChange(hContext, 500, &readerState, 1);
        if (lReturn == SCARD_S_SUCCESS) {
            if ((readerState.dwEventState & SCARD_STATE_CHANGED)) {
                if (readerState.dwEventState & SCARD_STATE_PRESENT) {
                    // Card inserted
                    emit cardInserted();

                    // Read data from the card
                    QByteArray cardData;
                    if (readCardData(cardData)) {
                        emit cardDataRead(cardData);
                    } else {
                        emit errorOccurred(errorMessage);
                    }
                } else if (readerState.dwEventState & SCARD_STATE_EMPTY) {
                    // Card removed
                    emit cardRemoved();
                }
                readerState.dwCurrentState = readerState.dwEventState;
            }
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = QString("SCardGetStatusChange failed: %1").arg(lReturn);
            emit errorOccurred(errorMessage);
            break;
        }
        msleep(100);  // Small sleep to prevent tight loop
    }
}

bool SmartCardReader::readCardData(QByteArray &cardData)
{
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;

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

    // Send APDU command to read data
    // Example APDU command to read binary data (modify as per your card's specification)
    QByteArray apduCommand = QByteArray::fromHex("FFB0000008");  // Read 8 bytes starting at offset 0
    QByteArray apduResponse;

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

    BYTE pbRecvBuffer[258];  // Max response size (256 bytes data + 2 bytes SW1 SW2)
    DWORD cbRecvLength = sizeof(pbRecvBuffer);

    lReturn = SCardTransmit(hCard,
                            &pioSendPci,
                            (LPCBYTE)apduCommand.constData(),
                            apduCommand.size(),
                            NULL,
                            pbRecvBuffer,
                            &cbRecvLength);
    SCardDisconnect(hCard, SCARD_LEAVE_CARD);

    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to transmit APDU: %1").arg(lReturn);
        return false;
    }

    // Check for success status word (SW1 SW2)
    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
            // Success, extract data
            cardData = QByteArray(reinterpret_cast<char*>(pbRecvBuffer), cbRecvLength - 2);
            return true;
        } else {
            errorMessage = QString("APDU command failed with status: %1 %2")
                               .arg(QString::number(sw1, 16).toUpper())
                               .arg(QString::number(sw2, 16).toUpper());
            return false;
        }
    } else {
        errorMessage = "Invalid response from card.";
        return false;
    }
}




#include <QCoreApplication>
#include <QDebug>
#include "SmartCardReader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SmartCardReader reader;

    QObject::connect(&reader, &SmartCardReader::cardInserted, []() {
        qDebug() << "Card inserted.";
    });

    QObject::connect(&reader, &SmartCardReader::cardRemoved, []() {
        qDebug() << "Card removed.";
    });

    QObject::connect(&reader, &SmartCardReader::cardDataRead, [](const QByteArray &data) {
        qDebug() << "Card Data Read:" << data.toHex().toUpper();
    });

    QObject::connect(&reader, &SmartCardReader::errorOccurred, [](const QString &error) {
        qDebug() << "Error:" << error;
    });

    if (!reader.initialize()) {
        qDebug() << "Initialization failed.";
        return -1;
    }

    reader.start();  // Start monitoring in a separate thread

    return a.exec();
}


