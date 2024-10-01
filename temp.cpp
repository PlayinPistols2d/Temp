// BarcodeListener.h

#ifndef BARCODEREADER_H
#define BARCODEREADER_H

#include <QObject>
#include <QSerialPort>
#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>

class BarcodeListener : public QObject
{
    Q_OBJECT

public:
    explicit BarcodeListener(QObject *parent = nullptr);
    ~BarcodeListener();

private slots:
    void readBarcodeData();

private:
    QSerialPort *serialPort;
    QUdpSocket *udpSocket;
    QByteArray barcodeBuffer;
    QHostAddress serverAddress;
    quint16 serverPort;
};

#endif // BARCODEREADER_H




// BarcodeListener.cpp

#include "BarcodeListener.h"
#include <QCoreApplication>
#include <QDebug>

BarcodeListener::BarcodeListener(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this))
{
    // Initialize serial port
    serialPort = new QSerialPort(this);
    serialPort->setPortName("COM4"); // Adjust port name if necessary
    serialPort->setBaudRate(QSerialPort::Baud9600); // Adjust baud rate if necessary
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // Attempt to open the serial port
    if (serialPort->open(QIODevice::ReadOnly))
    {
        qDebug() << "Serial port opened successfully.";
        connect(serialPort, &QSerialPort::readyRead, this, &BarcodeListener::readBarcodeData);
    }
    else
    {
        qCritical() << "Failed to open serial port:" << serialPort->errorString();
        QCoreApplication::quit();
    }

    // Set server address and port
    serverAddress = QHostAddress("192.168.1.100"); // Replace with your server's IP
    serverPort = 12345; // Replace with your server's port
}

BarcodeListener::~BarcodeListener()
{
    if (serialPort->isOpen())
        serialPort->close();
}

void BarcodeListener::readBarcodeData()
{
    // Read all available data from the serial port
    QByteArray data = serialPort->readAll();
    barcodeBuffer.append(data);

    // Check for end-of-barcode character (assuming newline '\n' or carriage return '\r')
    if (barcodeBuffer.contains('\n') || barcodeBuffer.contains('\r'))
    {
        // Extract the complete barcode string
        QString barcode = QString::fromUtf8(barcodeBuffer).trimmed();
        barcodeBuffer.clear();

        if (!barcode.isEmpty())
        {
            qDebug() << "Barcode read:" << barcode;

            // Send barcode to server via UDP
            QByteArray datagram = barcode.toUtf8();
            udpSocket->writeDatagram(datagram, serverAddress, serverPort);
            qDebug() << "Barcode sent to server at" << serverAddress.toString() << "on port" << serverPort;
        }
    }
}


// main.cpp

#include <QCoreApplication>
#include "BarcodeListener.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    BarcodeListener listener;

    return app.exec();
}