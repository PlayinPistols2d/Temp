// BarcodeListener.h

#ifndef BARCODELISTENER_H
#define BARCODELISTENER_H

#include <QObject>
#include <QSerialPort>
#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>

class BarcodeListener : public QObject
{
    Q_OBJECT

public:
    explicit BarcodeListener(QObject *parent = nullptr);
    ~BarcodeListener();

private slots:
    void readBarcodeData();
    void handleSerialError(QSerialPort::SerialPortError error);
    void attemptReconnect();

private:
    void openSerialPort();

    QSerialPort *serialPort;
    QUdpSocket *udpSocket;
    QByteArray barcodeBuffer;
    QHostAddress serverAddress;
    quint16 serverPort;
    QTimer *reconnectTimer;
    QString portName;
    int baudRate;
};

#endif // BARCODELISTENER_H


// BarcodeListener.cpp

#include "BarcodeListener.h"
#include <QCoreApplication>
#include <QDebug>

BarcodeListener::BarcodeListener(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this)), reconnectTimer(new QTimer(this))
{
    // Serial port settings
    portName = "COM4"; // Adjust port name if necessary
    baudRate = QSerialPort::Baud9600; // Adjust baud rate if necessary

    // Initialize serial port
    serialPort = new QSerialPort(this);

    // Attempt to open the serial port
    openSerialPort();

    // Set server address and port
    serverAddress = QHostAddress("192.168.1.100"); // Replace with your server's IP
    serverPort = 12345; // Replace with your server's port

    // Setup reconnect timer but do not start it yet
    reconnectTimer->setInterval(5000); // Attempt reconnection every 5 seconds
    connect(reconnectTimer, &QTimer::timeout, this, &BarcodeListener::attemptReconnect);
}

BarcodeListener::~BarcodeListener()
{
    if (serialPort->isOpen())
        serialPort->close();
}

void BarcodeListener::openSerialPort()
{
    if (serialPort->isOpen())
        serialPort->close();

    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadOnly))
    {
        qDebug() << "Serial port opened successfully.";

        // Connect signals and slots
        connect(serialPort, &QSerialPort::readyRead, this, &BarcodeListener::readBarcodeData);
        connect(serialPort, &QSerialPort::errorOccurred, this, &BarcodeListener::handleSerialError);

        // Stop the reconnect timer if running
        if (reconnectTimer->isActive())
            reconnectTimer->stop();
    }
    else
    {
        qWarning() << "Failed to open serial port:" << serialPort->errorString();

        // Start reconnect timer if not already running
        if (!reconnectTimer->isActive())
            reconnectTimer->start();
    }
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

void BarcodeListener::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        qWarning() << "Serial port error occurred:" << serialPort->errorString();

        // Disconnect signals to prevent redundant calls
        disconnect(serialPort, &QSerialPort::readyRead, this, &BarcodeListener::readBarcodeData);
        disconnect(serialPort, &QSerialPort::errorOccurred, this, &BarcodeListener::handleSerialError);

        serialPort->close();

        // Start reconnect timer
        if (!reconnectTimer->isActive())
            reconnectTimer->start();
    }
}

void BarcodeListener::attemptReconnect()
{
    qDebug() << "Attempting to reconnect to serial port...";

    openSerialPort();
}