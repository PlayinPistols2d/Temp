#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QByteArray>
#include <QThread>
#include <QObject>
#include <atomic>

/**
 * @brief The SmartCardReader class handles the interaction with the ACR122U NFC reader.
 * It monitors card insertion, removal, and reader connections, and manages status states.
 */
class SmartCardReader : public QThread
{
    Q_OBJECT
public:
    explicit SmartCardReader(QObject *parent = nullptr);
    ~SmartCardReader();

    /**
     * @brief Initializes the smart card context and sets up the reader.
     * @return True if initialization is successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief The main thread execution function that starts monitoring card status.
     */
    void run() override;

    /**
     * @brief Stops the monitoring thread gracefully.
     */
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

    void monitorCardStatus();
    bool readCardData(QByteArray &cardData);
    void checkForReaderConnection();
    void setReaderState(ReaderState state);
};

#endif // SMARTCARDREADER_H





#include "SmartCardReader.h"
#include <QDebug>
#include <QThread>

#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE SCARD_CTL_CODE(2048)

SmartCardReader::SmartCardReader(QObject *parent)
    : QThread(parent),
      hContext(0),
      isMonitoring(false),
      cardPresent(false),  // Initialize the card presence flag
      currentState(Disconnected)  // Initial state
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
    setReaderState(Connecting);
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        emit errorOccurred(errorMessage);
        setReaderState(Disconnected);
        return false;
    }

    checkForReaderConnection();
    return currentState == Connected;
}

void SmartCardReader::run()
{
    isMonitoring = true;
    while (isMonitoring) {
        if (currentState == Disconnected) {
            setReaderState(Connecting);
            checkForReaderConnection();
            if (currentState == Connected) {
                monitorCardStatus();
            } else {
                QThread::sleep(1);  // Sleep for a second before checking again
            }
        } else {
            monitorCardStatus();
        }
    }
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
        emit errorOccurred("No reader found or failed to list readers.");
        setReaderState(Disconnected);
        if (mszReaders) {
            SCardFreeMemory(hContext, mszReaders);
        }
        return;
    }

    // Select the first available reader
    readerName = QString::fromWCharArray(mszReaders);
    SCardFreeMemory(hContext, mszReaders);
    setReaderState(Connected);
    qDebug() << "Reader connected:" << readerName;
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
                    emit cardInserted();

                    QByteArray cardData;
                    if (readCardData(cardData)) {
                        emit cardDataRead(cardData);
                    } else {
                        emit errorOccurred(errorMessage);
                    }
                } else if (!(readerState.dwEventState & SCARD_STATE_PRESENT) && cardPresent) {
                    cardPresent = false;
                    emit cardRemoved();
                }
            }
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = QString("SCardGetStatusChange failed: 0x%1")
                               .arg(QString::number(lReturn, 16).toUpper());
            emit errorOccurred(errorMessage);
            setReaderState(Disconnected);
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
        emit readerStateChanged(currentState);
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

    // Connect signals to slots for handling card and reader events
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

    // Connect the readerStateChanged signal to monitor the reader's state
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

    // Initialize the reader
    if (!reader.initialize()) {
        qDebug() << "Initialization failed. The application will exit.";
        return -1;
    }

    // Start monitoring in a separate thread
    reader.start();

    return a.exec();
}