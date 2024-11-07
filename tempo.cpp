#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>
#include <QObject>
#include <QMutex>
#include <QFuture>

/**
 * @brief The SmartCardReader class handles interaction with the ACR122U NFC reader.
 * It monitors card insertion, removal, and reader connections, and manages status states.
 */
class SmartCardReader : public QObject
{
    Q_OBJECT
public:
    explicit SmartCardReader(QObject *parent = nullptr);
    ~SmartCardReader();

    bool initialize();
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
    std::atomic<bool> isMonitoring;
    bool cardPresent;
    ReaderState currentState;
    QMutex mutex;  // Mutex for thread safety

    void monitorCardStatus();
    bool readCardData(QByteArray &cardData);
    bool checkForReaderConnection();
    void setReaderState(ReaderState state);
};

#endif // SMARTCARDREADER_H









#include "SmartCardReader.h"
#include <QDebug>
#include <QtConcurrent>
#include <QMetaObject>

#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE SCARD_CTL_CODE(2048)

SmartCardReader::SmartCardReader(QObject *parent)
    : QObject(parent),
      hContext(0),
      isMonitoring(false),
      cardPresent(false),
      currentState(Disconnected)
{
    // Ensure that signals are emitted in a thread-safe manner
    moveToThread(parent->thread());
}

SmartCardReader::~SmartCardReader()
{
    stopMonitoring();
    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize()
{
    QMutexLocker locker(&mutex);
    if (hContext != 0) {
        SCardReleaseContext(hContext);
        hContext = 0;
    }

    setReaderState(Connecting);
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        emit errorOccurred(errorMessage);
        setReaderState(Disconnected);
        return false;
    }

    return checkForReaderConnection();
}

void SmartCardReader::startMonitoring()
{
    isMonitoring = true;
    QtConcurrent::run([this]() { monitorCardStatus(); });  // Run the monitoring method concurrently
}

void SmartCardReader::stopMonitoring()
{
    QMutexLocker locker(&mutex);
    isMonitoring = false;
}

bool SmartCardReader::checkForReaderConnection()
{
    QMutexLocker locker(&mutex);
    DWORD dwReaders = SCARD_AUTOALLOCATE;
    LPTSTR mszReaders = NULL;
    LONG lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);

    if (lReturn != SCARD_S_SUCCESS || dwReaders == 0) {
        emit errorOccurred("No reader found or failed to list readers.");
        setReaderState(Disconnected);
        if (mszReaders) {
            SCardFreeMemory(hContext, mszReaders);
        }
        return false;
    }

    QString newReaderName = QString::fromWCharArray(mszReaders);
    SCardFreeMemory(hContext, mszReaders);

    if (readerName != newReaderName) {
        readerName = newReaderName;
        setReaderState(Connected);
        qDebug() << "Reader connected:" << readerName;
    }

    return true;
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
                    QMetaObject::invokeMethod(this, "cardInserted", Qt::QueuedConnection);

                    QByteArray cardData;
                    if (readCardData(cardData)) {
                        QMetaObject::invokeMethod(this, "cardDataRead", Qt::QueuedConnection, Q_ARG(QByteArray, cardData));
                    } else {
                        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection, Q_ARG(QString, errorMessage));
                    }
                } else if (!(readerState.dwEventState & SCARD_STATE_PRESENT) && cardPresent) {
                    cardPresent = false;
                    QMetaObject::invokeMethod(this, "cardRemoved", Qt::QueuedConnection);
                }
            }
        } else if (lReturn == SCARD_E_READER_UNAVAILABLE || lReturn == SCARD_E_NO_READERS_AVAILABLE) {
            qDebug() << "Reader disconnected, attempting to reconnect...";
            setReaderState(Disconnected);
            readerName.clear();
            break;
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = QString("SCardGetStatusChange failed: 0x%1")
                               .arg(QString::number(lReturn, 16).toUpper());
            QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection, Q_ARG(QString, errorMessage));
            setReaderState(Disconnected);
            readerName.clear();
            break;
        }
    }
}

bool SmartCardReader::readCardData(QByteArray &cardData)
{
    QMutexLocker locker(&mutex);
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
        QMetaObject::invokeMethod(this, "readerStateChanged", Qt::QueuedConnection, Q_ARG(ReaderState, state));
        qDebug() << "Reader state changed to:" << (state == Connected ? "Connected" :
                                                    state == Connecting ? "Connecting" : "Disconnected");
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

    QObject::connect(&reader, &SmartCardReader::readerStateChanged, [](SmartCardReader::ReaderState state) {
        switch (state) {
            case SmartCardReader::Connected:
                qDebug() << "Reader state: Connected";
                break;
            case SmartCardReader::Disconnected:
                qDebug() << "Reader state: Disconnected";
                break;
            case SmartCardReader::Connecting:
                qDebug() << "Reader state: Connecting";
                break;
        }
    });

    // Start monitoring in a concurrent manner
    reader.startMonitoring();

    return a.exec();
}