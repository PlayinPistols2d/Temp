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
}

void SmartCardReader::run()
{
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
}

void SmartCardReader::stopMonitoring()
{
    isMonitoring = false;
}

bool SmartCardReader::checkForReaderConnection()
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

    // Start the monitoring thread
    reader.start();

    return a.exec();  // Keep the application running to continue monitoring for a reader
}