#include "SmartCardReader.h"
#include <QDebug>
#include <QThread>
#include <exception>

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
    try {
        stopMonitoring();
        wait();  // Wait for the thread to finish
        if (hContext != 0) {
            SCardReleaseContext(hContext);
        }
    } catch (const std::exception &ex) {
        qDebug() << "Exception in destructor:" << ex.what();
    } catch (...) {
        qDebug() << "Unknown exception in destructor.";
    }
}

bool SmartCardReader::initialize()
{
    try {
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
            return false;  // Do not exit, continue waiting in the run loop
        }

        // Try to find a reader, but don't return false if none is found initially
        checkForReaderConnection();
        return true;  // Always return true to keep the app running
    } catch (const std::exception &ex) {
        qDebug() << "Exception in initialize:" << ex.what();
        emit errorOccurred("Exception in initialize: " + QString::fromStdString(ex.what()));
        setReaderState(Disconnected);
        return false;
    } catch (...) {
        qDebug() << "Unknown exception in initialize.";
        emit errorOccurred("Unknown exception in initialize.");
        setReaderState(Disconnected);
        return false;
    }
}

void SmartCardReader::run()
{
    try {
        isMonitoring = true;
        while (isMonitoring) {
            if (currentState == Disconnected) {
                setReaderState(Connecting);
                initialize();  // Attempt to initialize and find a reader
                QThread::sleep(1);  // Sleep for a second before retrying to prevent high CPU usage
            } else {
                monitorCardStatus();
            }
        }
    } catch (const std::exception &ex) {
        qDebug() << "Exception in run loop:" << ex.what();
        emit errorOccurred("Exception in run loop: " + QString::fromStdString(ex.what()));
    } catch (...) {
        qDebug() << "Unknown exception in run loop.";
        emit errorOccurred("Unknown exception in run loop.");
    }
}

void SmartCardReader::stopMonitoring()
{
    try {
        isMonitoring = false;
    } catch (const std::exception &ex) {
        qDebug() << "Exception in stopMonitoring:" << ex.what();
    } catch (...) {
        qDebug() << "Unknown exception in stopMonitoring.";
    }
}

bool SmartCardReader::checkForReaderConnection()
{
    try {
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

        // Select the first available reader
        QString newReaderName = QString::fromWCharArray(mszReaders);
        SCardFreeMemory(hContext, mszReaders);

        if (readerName != newReaderName) {
            readerName = newReaderName;
            setReaderState(Connected);
            qDebug() << "Reader connected:" << readerName;
        }

        return true;
    } catch (const std::exception &ex) {
        qDebug() << "Exception in checkForReaderConnection:" << ex.what();
        emit errorOccurred("Exception in checkForReaderConnection: " + QString::fromStdString(ex.what()));
        setReaderState(Disconnected);
        return false;
    } catch (...) {
        qDebug() << "Unknown exception in checkForReaderConnection.";
        emit errorOccurred("Unknown exception in checkForReaderConnection.");
        setReaderState(Disconnected);
        return false;
    }
}

void SmartCardReader::monitorCardStatus()
{
    try {
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
            } else if (lReturn == SCARD_E_READER_UNAVAILABLE || lReturn == SCARD_E_NO_READERS_AVAILABLE) {
                qDebug() << "Reader disconnected, attempting to reconnect...";
                setReaderState(Disconnected);
                readerName.clear();  // Clear the reader name to force detection
                break;
            } else if (lReturn != SCARD_E_TIMEOUT) {
                errorMessage = QString("SCardGetStatusChange failed: 0x%1")
                                   .arg(QString::number(lReturn, 16).toUpper());
                emit errorOccurred(errorMessage);
                setReaderState(Disconnected);
                readerName.clear();  // Clear the reader name to force detection
                break;
            }
        }
    } catch (const std::exception &ex) {
        qDebug() << "Exception in monitorCardStatus:" << ex.what();
        emit errorOccurred("Exception in monitorCardStatus: " + QString::fromStdString(ex.what()));
        setReaderState(Disconnected);
    } catch (...) {
        qDebug() << "Unknown exception in monitorCardStatus.";
        emit errorOccurred("Unknown exception in monitorCardStatus.");
        setReaderState(Disconnected);
    }
}

bool SmartCardReader::readCardData(QByteArray &cardData)
{
    try {
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
    } catch (const std::exception &ex) {
        qDebug() << "Exception in readCardData:" << ex.what();
        emit errorOccurred("Exception in readCardData: " + QString::fromStdString(ex.what()));
        return false;
    } catch (...) {
        qDebug() << "Unknown exception in readCardData.";
        emit errorOccurred("Unknown exception in readCardData.");
        return false;
    }
}