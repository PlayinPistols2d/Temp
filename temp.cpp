#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>
#include <QThread>
#include <QObject>
#include <atomic>

class SmartCardReader : public QThread
{
    Q_OBJECT
public:
    explicit SmartCardReader(QObject *parent = nullptr);
    ~SmartCardReader();

    bool initialize();
    void run() override;
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

// Define the control code if needed for SCardControl
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE SCARD_CTL_CODE(2048)

SmartCardReader::SmartCardReader(QObject *parent)
    : QThread(parent),
      hContext(0),
      isMonitoring(false)
{
}

SmartCardReader::~SmartCardReader()
{
    stopMonitoring();
    wait();  // Ensure the thread finishes before destruction
    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        emit errorOccurred(errorMessage);
        return false;
    }

    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR mszReaders = NULL;
    lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to list readers: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
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
        readerState.dwCurrentState = readerState.dwEventState;
        LONG lReturn = SCardGetStatusChange(hContext, 10, &readerState, 1);
        if (lReturn == SCARD_S_SUCCESS) {
            if ((readerState.dwEventState & SCARD_STATE_CHANGED)) {
                if ((readerState.dwEventState & SCARD_STATE_PRESENT) &&
                    !(readerState.dwCurrentState & SCARD_STATE_PRESENT)) {
                    // Card inserted
                    emit cardInserted();

                    // Read data from the card
                    QByteArray cardData;
                    if (readCardData(cardData)) {
                        emit cardDataRead(cardData);
                    } else {
                        emit errorOccurred(errorMessage);
                    }
                } else if ((readerState.dwEventState & SCARD_STATE_EMPTY) &&
                           !(readerState.dwCurrentState & SCARD_STATE_EMPTY)) {
                    // Card removed
                    emit cardRemoved();
                }
            }
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = QString("SCardGetStatusChange failed: 0x%1")
                               .arg(QString::number(lReturn, 16).toUpper());
            emit errorOccurred(errorMessage);
            break;
        }
        // Remove sleep to increase responsiveness
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
        errorMessage = QString("Failed to connect to card: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        return false;
    }

    // APDU command to get the UID with Le set to 6
    QByteArray apduCommand = QByteArray::fromHex("FFCA000006");

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

    BYTE pbRecvBuffer[258];
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
        errorMessage = QString("Failed to transmit APDU: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        return false;
    }

    // Check for success status word (SW1 SW2)
    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
            // Success, extract UID
            int uidLength = cbRecvLength - 2;
            cardData = QByteArray(reinterpret_cast<char*>(pbRecvBuffer), uidLength);
            return true;
        } else {
            errorMessage = QString("APDU command failed with status: %1 %2")
                               .arg(QString("%1").arg(sw1, 2, 16, QChar('0')).toUpper())
                               .arg(QString("%1").arg(sw2, 2, 16, QChar('0')).toUpper());
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
        qDebug() << "Card UID:" << data.toHex().toUpper();
        qDebug() << "UID Length:" << data.size();
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