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

    void run() override;
    void stop();

    enum class ReaderState {
        Disconnected,
        Connecting,
        Connected
    };
    Q_ENUM(ReaderState)

    ReaderState getState() const;

signals:
    void stateChanged(SmartCardReader::ReaderState newState);
    void cardDataRead(const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    void monitorReaders();
    bool connectReader();
    void disconnectReader();
    void monitorCardStatus();
    bool readCardData(QByteArray &cardData);

    SCARDCONTEXT hContext;
    QString readerName;
    QString errorMessage;
    std::atomic<bool> isRunning;
    std::atomic<bool> cardPresent;
    std::atomic<ReaderState> currentState;
};

#endif // SMARTCARDREADER_H






#include "SmartCardReader.h"
#include <QDebug>
#include <QTimer>
#include <tchar.h>  // Include for TCHAR types

SmartCardReader::SmartCardReader(QObject *parent)
    : QThread(parent),
      hContext(0),
      isRunning(false),
      cardPresent(false),
      currentState(ReaderState::Disconnected)
{
}

SmartCardReader::~SmartCardReader()
{
    stop();
    wait();  // Ensure the thread finishes before destruction
    if (hContext != 0) {
        SCardReleaseContext(hContext);
    }
}

void SmartCardReader::stop()
{
    isRunning = false;
}

SmartCardReader::ReaderState SmartCardReader::getState() const
{
    return currentState.load();
}

void SmartCardReader::run()
{
    isRunning = true;
    monitorReaders();
}

void SmartCardReader::monitorReaders()
{
    LONG lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to establish context: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        emit errorOccurred(errorMessage);
        return;
    }

    while (isRunning) {
        // Check for available readers
        DWORD dwReaders = SCARD_AUTOALLOCATE;
        LPTSTR mszReaders = NULL;
        lReturn = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);

        if (lReturn == SCARD_S_SUCCESS && dwReaders > 1) {
            QStringList readerList;

#ifdef UNICODE
            // mszReaders is wchar_t*
            readerList = QString::fromWCharArray(mszReaders).split(QChar('\0'), QString::SkipEmptyParts);
#else
            // mszReaders is char*
            readerList = QString::fromLocal8Bit(mszReaders).split('\0', QString::SkipEmptyParts);
#endif

            if (!readerList.isEmpty()) {
                if (currentState != ReaderState::Connected) {
                    // Attempt to connect to the reader
                    readerName = readerList.first();
                    currentState = ReaderState::Connecting;
                    emit stateChanged(currentState);

                    if (connectReader()) {
                        currentState = ReaderState::Connected;
                        emit stateChanged(currentState);
                        monitorCardStatus();
                    } else {
                        currentState = ReaderState::Disconnected;
                        emit stateChanged(currentState);
                    }
                }
            } else {
                // No readers found
                if (currentState != ReaderState::Disconnected) {
                    currentState = ReaderState::Disconnected;
                    emit stateChanged(currentState);
                }
            }
        } else {
            // No readers found or an error occurred
            if (currentState != ReaderState::Disconnected) {
                currentState = ReaderState::Disconnected;
                emit stateChanged(currentState);
            }
        }

        if (mszReaders != NULL) {
            SCardFreeMemory(hContext, mszReaders);
            mszReaders = NULL;
        }

        // Sleep for a while before checking again
        QThread::sleep(1);
    }

    // Clean up context
    SCardReleaseContext(hContext);
    hContext = 0;
}

bool SmartCardReader::connectReader()
{
    // In this simplified example, we assume that the reader is always available when detected
    return true;
}

void SmartCardReader::disconnectReader()
{
    // Any cleanup if needed
    currentState = ReaderState::Disconnected;
    emit stateChanged(currentState);
}

void SmartCardReader::monitorCardStatus()
{
    SCARD_READERSTATE readerState;

#ifdef UNICODE
    readerState.szReader = (LPWSTR)readerName.utf16();
#else
    readerState.szReader = (LPSTR)readerName.toLocal8Bit().constData();
#endif

    readerState.pvUserData = NULL;
    readerState.dwCurrentState = SCARD_STATE_UNAWARE;
    readerState.dwEventState = 0;
    readerState.cbAtr = 0;

    while (isRunning && currentState == ReaderState::Connected) {
        // Copy the current event state to current state
        readerState.dwCurrentState = readerState.dwEventState;

        // Set a timeout for SCardGetStatusChange
        LONG lReturn = SCardGetStatusChange(hContext, 500, &readerState, 1);
        if (lReturn == SCARD_S_SUCCESS) {
            // Card inserted
            if ((readerState.dwEventState & SCARD_STATE_PRESENT) && !cardPresent) {
                cardPresent = true;

                // Read data from the card
                QByteArray cardData;
                if (readCardData(cardData)) {
                    emit cardDataRead(cardData);
                } else {
                    emit errorOccurred(errorMessage);
                }
            }
            // Card removed
            else if (!(readerState.dwEventState & SCARD_STATE_PRESENT) && cardPresent) {
                cardPresent = false;
            }
            // Reader removed
            if (readerState.dwEventState & SCARD_STATE_UNKNOWN) {
                // The reader has been disconnected
                currentState = ReaderState::Disconnected;
                emit stateChanged(currentState);
                break;
            }
        } else if (lReturn == SCARD_E_UNKNOWN_READER || lReturn == SCARD_E_READER_UNAVAILABLE) {
            // The reader has been disconnected
            currentState = ReaderState::Disconnected;
            emit stateChanged(currentState);
            break;
        } else if (lReturn != SCARD_E_TIMEOUT) {
            errorMessage = QString("SCardGetStatusChange failed: 0x%1")
                               .arg(QString::number(lReturn, 16).toUpper());
            emit errorOccurred(errorMessage);
            break;
        }
        // Optional: Sleep for a short period to prevent high CPU usage
        // QThread::msleep(10);
    }
}

bool SmartCardReader::readCardData(QByteArray &cardData)
{
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;

#ifdef UNICODE
    LPCWSTR readerNamePtr = (LPCWSTR)readerName.utf16();
#else
    LPCSTR readerNamePtr = (LPCSTR)readerName.toLocal8Bit().constData();
#endif

    LONG lReturn = SCardConnect(hContext,
                                readerNamePtr,
                                SCARD_SHARE_SHARED,
                                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                &hCard,
                                &dwActiveProtocol);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to connect to card: 0x%1")
                           .arg(QString::number(lReturn, 16).toUpper());
        return false;
    }

    // APDU command to get the UID
    QByteArray apduCommand = QByteArray::fromHex("FFCA000000");

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
            int uidLength = cbRecvLength - 2;  // Exclude SW1 and SW2
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




