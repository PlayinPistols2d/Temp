#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QThreadPool>
#include <QMetaObject>

/**
 * @brief The SmartCardReader class handles NFC card reader operations and runs them in a separate thread.
 */
class SmartCardReader : public QObject
{
    Q_OBJECT
public:
    explicit SmartCardReader(QObject *parent = nullptr);
    ~SmartCardReader();

    void startMonitoring();
    void stopMonitoring();

    enum ReaderState {
        Connected,
        Disconnected,
        Connecting
    };

signals:
    void cardInserted();
    void cardRemoved();
    void cardDataRead(const QByteArray &data);
    void errorOccurred(const QString &error);
    void readerStateChanged(ReaderState state);

private:
    SCARDCONTEXT hContext;
    QString readerName;
    QString errorMessage;
    bool cardPresent;
    ReaderState currentState;
    bool isMonitoring;

    void monitorCardStatus();
    bool readCardData(QByteArray &cardData);
    void checkForReaderConnection();
    void setReaderState(ReaderState state);
};

#endif // SMARTCARDREADER_H









#include "SmartCardReader.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE SCARD_CTL_CODE(2048)

SmartCardReader::SmartCardReader(QObject *parent)
    : QObject(parent),
      hContext(0),
      cardPresent(false),
      currentState(Disconnected),
      isMonitoring(false)
{
}

SmartCardReader::~SmartCardReader()
{
    stopMonitoring();
    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

void SmartCardReader::startMonitoring()
{
    isMonitoring = true;
    QtConcurrent::run([this]() {
        while (isMonitoring) {
            if (currentState == Disconnected) {
                setReaderState(Connecting);
                if (hContext == 0) {
                    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
                    if (lReturn != SCARD_S_SUCCESS) {
                        emit errorOccurred(QString("Failed to establish context: 0x%1").arg(QString::number(lReturn, 16).toUpper()));
                        setReaderState(Disconnected);
                        QThread::sleep(1);
                        continue;
                    }
                }
                checkForReaderConnection();
                QThread::sleep(1);  // Sleep before checking again to avoid high CPU usage
            } else {
                monitorCardStatus();
            }
        }
    });
}

void SmartCardReader::stopMonitoring()
{
    isMonitoring = false;
}

void SmartCardReader::checkForReaderConnection()
{
    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR mszReaders = NULL;
    LONG lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);

    if (lReturn != SCARD_S_SUCCESS || dwReaders == 0) {
        QMetaObject::invokeMethod(this, [this]() {
            emit errorOccurred("No reader found or failed to list readers.");
            setReaderState(Disconnected);
        }, Qt::QueuedConnection);
        if (mszReaders) {
            SCardFreeMemory(hContext, mszReaders);
        }
        return;
    }

    QString newReaderName = QString::fromWCharArray(mszReaders);
    SCardFreeMemory(hContext, mszReaders);

    if (readerName != newReaderName) {
        readerName = newReaderName;
        QMetaObject::invokeMethod(this, [this]() {
            setReaderState(Connected);
            qDebug() << "Reader connected:" << readerName;
        }, Qt::QueuedConnection);
    }
}

void SmartCardReader::monitorCardStatus()
{
    SCARD_READERSTATE readerState;
    readerState.szReader = (LPCWSTR)readerName.utf16();
    readerState.pvUserData = NULL;
    readerState.dwCurrentState = SCARD_STATE_UNAWARE;
    readerState.dwEventState = 0;
    readerState.cbAtr = 0;

    while (isMonitoring && currentState == Connected) {
        readerState.dwCurrentState = readerState.dwEventState;
        LONG lReturn = SCardGetStatusChange(hContext, 500, &readerState, 1);

        if (lReturn == SCARD_S_SUCCESS) {
            if ((readerState.dwEventState & SCARD_STATE_CHANGED)) {
                if ((readerState.dwEventState & SCARD_STATE_PRESENT) && !cardPresent) {
                    cardPresent = true;
                    QMetaObject::invokeMethod(this, &SmartCardReader::cardInserted, Qt::QueuedConnection);

                    QByteArray cardData;
                    if (readCardData(cardData)) {
                        QMetaObject::invokeMethod(this, [cardData]() {
                            emit cardDataRead(cardData);
                        }, Qt::QueuedConnection);
                    } else {
                        QMetaObject::invokeMethod(this, [this]() {
                            emit errorOccurred(errorMessage);
                        }, Qt::QueuedConnection);
                    }
                } else if (!(readerState.dwEventState & SCARD_STATE_PRESENT) && cardPresent) {
                    cardPresent = false;
                    QMetaObject::invokeMethod(this, &SmartCardReader::cardRemoved, Qt::QueuedConnection);
                }
            }
        } else if (lReturn == SCARD_E_READER_UNAVAILABLE || lReturn == SCARD_E_NO_READERS_AVAILABLE) {
            QMetaObject::invokeMethod(this, [this]() {
                qDebug() << "Reader disconnected, attempting to reconnect...";
                setReaderState(Disconnected);
                readerName.clear();
            }, Qt::QueuedConnection);
            break;
        } else if (lReturn != SCARD_E_TIMEOUT) {
            QMetaObject::invokeMethod(this, [this, lReturn]() {
                emit errorOccurred(QString("SCardGetStatusChange failed: 0x%1").arg(QString::number(lReturn, 16).toUpper()));
                setReaderState(Disconnected);
                readerName.clear();
            }, Qt::QueuedConnection);
            break;
        }
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

    QByteArray apduCommand = QByteArray::fromHex("FFCA000000");
    SCARD_IO_REQUEST pioSendPci = (dwActiveProtocol == SCARD_PROTOCOL_T0) ? *SCARD_PCI_T0 : *SCARD_PCI_T1;

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

    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
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

void SmartCardReader::setReaderState(ReaderState state)
{
    if (currentState != state) {
        currentState = state;
        QMetaObject::invokeMethod(this, [this, state]() {
            emit readerStateChanged(state);
            qDebug() << "Reader state changed to:" << (state == Connected ? "Connected" :
                                                        state == Connecting ? "Connecting" : "Disconnected");
        }, Qt::QueuedConnection);
    }
}