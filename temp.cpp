#ifndef BARCODESCANNER_H
#define BARCODESCANNER_H

#include <QObject>
#include <QUdpSocket>
#include <QString>

class BarcodeScanner : public QObject {
    Q_OBJECT

public:
    BarcodeScanner(const QString &serverAddress, quint16 serverPort);
    void startReading();  // Starts reading from the input

private:
    QUdpSocket *m_udpSocket;
    QString m_serverAddress;
    quint16 m_serverPort;

    void sendBarcode(const QString &barcode);  // Sends the barcode to the server
};

#endif // BARCODESCANNER_H




#include "barcode_scanner.h"
#include <QTextStream>
#include <QHostAddress>
#include <QDebug>

BarcodeScanner::BarcodeScanner(const QString &serverAddress, quint16 serverPort)
    : m_serverAddress(serverAddress), m_serverPort(serverPort) {
    m_udpSocket = new QUdpSocket(this);
}

void BarcodeScanner::startReading() {
    QTextStream input(stdin);
    QString barcode;
    qDebug() << "Waiting for barcode input...";

    while (true) {
        barcode = input.readLine();
        if (!barcode.isEmpty()) {
            sendBarcode(barcode);
        }
    }
}

void BarcodeScanner::sendBarcode(const QString &barcode) {
    QByteArray data = barcode.toUtf8();
    m_udpSocket->writeDatagram(data, QHostAddress(m_serverAddress), m_serverPort);
    qDebug() << "Barcode sent: " << barcode;
}




#include <QCoreApplication>
#include "barcode_scanner.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString serverAddress = "127.0.0.1";  // Set your server IP
    quint16 serverPort = 12345;           // Set your server port

    BarcodeScanner scanner(serverAddress, serverPort);
    
    // Start reading input from the barcode scanner
    scanner.startReading();

    return a.exec();
}