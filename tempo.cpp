#ifndef SMARTCARDREADER_H
#define SMARTCARDREADER_H

#include <winscard.h>
#include <QString>
#include <QDebug>

class SmartCardReader {
public:
    SmartCardReader();
    ~SmartCardReader();

    bool initialize();
    bool connectToReader();
    void disconnectReader();
    QString getReaderName() const;

private:
    SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
    DWORD activeProtocol;
    QString readerName;
};

#endif // SMARTCARDREADER_H





#include "SmartCardReader.h"

SmartCardReader::SmartCardReader()
    : hContext(0), hCard(0), activeProtocol(0) {}

SmartCardReader::~SmartCardReader() {
    disconnectReader();  // Ensure disconnection on destruction
    if (hContext) {
        SCardReleaseContext(hContext);
    }
}

bool SmartCardReader::initialize() {
    LONG result = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to establish context:" << result;
        return false;
    }

    DWORD readersLen;
    result = SCardListReaders(hContext, NULL, NULL, &readersLen);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to list readers:" << result;
        return false;
    }

    char *readers = new char[readersLen];
    result = SCardListReaders(hContext, NULL, readers, &readersLen);
    if (result == SCARD_S_SUCCESS) {
        readerName = QString::fromLocal8Bit(readers);
        qDebug() << "Reader found:" << readerName;
        delete[] readers;
        return true;
    } else {
        qDebug() << "Error listing readers.";
        delete[] readers;
        return false;
    }
}

bool SmartCardReader::connectToReader() {
    if (readerName.isEmpty()) {
        qDebug() << "No reader available.";
        return false;
    }

    LONG result = SCardConnect(hContext, readerName.toLocal8Bit().constData(), SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &activeProtocol);
    
    if (result == SCARD_S_SUCCESS) {
        qDebug() << "Successfully connected to the reader!";
        return true;
    } else {
        qDebug() << "Failed to connect to the card:" << result;
        return false;
    }
}

void SmartCardReader::disconnectReader() {
    if (hCard) {
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        hCard = 0;
    }
}

QString SmartCardReader::getReaderName() const {
    return readerName;
}





#include <QCoreApplication>
#include "SmartCardReader.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    SmartCardReader reader;

    if (reader.initialize()) {
        qDebug() << "Reader initialized:" << reader.getReaderName();
        
        if (reader.connectToReader()) {
            qDebug() << "Connection to reader successful!";
            reader.disconnectReader();
        } else {
            qDebug() << "Failed to connect to the reader.";
        }
    } else {
        qDebug() << "Failed to initialize reader.";
    }

    return app.exec();
}
