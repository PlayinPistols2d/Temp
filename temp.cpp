#include "barcodelistener.h"
#include <QTextStream>
#include <QDebug>

BarcodeListener::BarcodeListener(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this)), inputNotifier(new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this))
{
    connect(inputNotifier, &QSocketNotifier::activated, this, &BarcodeListener::onBarcodeReceived);
}

void BarcodeListener::onBarcodeReceived()
{
    QTextStream input(stdin);
    QString barcode = input.readLine().trimmed();
    
    if (!barcode.isEmpty()) {
        qDebug() << "Barcode received:" << barcode;
        sendBarcodeToServer(barcode);
    }
}

void BarcodeListener::sendBarcodeToServer(const QString &barcode)
{
    QByteArray datagram = barcode.toUtf8();
    QHostAddress serverAddress("127.0.0.1");  // Replace with your server's IP address
    quint16 serverPort = 12345;               // Replace with your server's UDP port
    
    udpSocket->writeDatagram(datagram, serverAddress, serverPort);
    qDebug() << "Barcode sent to server.";
}


#ifndef BARCODERLISTENER_H
#define BARCODERLISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QSocketNotifier>

class BarcodeListener : public QObject {
    Q_OBJECT

public:
    explicit BarcodeListener(QObject *parent = nullptr);

private slots:
    void onBarcodeReceived();
    void sendBarcodeToServer(const QString &barcode);

private:
    QUdpSocket *udpSocket;
    QSocketNotifier *inputNotifier;
};

#endif // BARCODERLISTENER_H


#include <QCoreApplication>
#include "barcodelistener.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    BarcodeListener listener;

    return a.exec();
}

