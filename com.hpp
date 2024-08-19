#include <QCoreApplication>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDebug>

class ComPortMonitor : public QObject
{
    Q_OBJECT

public:
    ComPortMonitor(QObject *parent = nullptr)
        : QObject(parent)
    {
        // Start a timer to check for new COM ports every 2 seconds
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ComPortMonitor::checkForNewPorts);
        timer->start(2000);

        // Store the initial list of available ports
        availablePorts = QSerialPortInfo::availablePorts();
    }

private slots:
    void checkForNewPorts()
    {
        // Get the current list of available ports
        QList<QSerialPortInfo> currentPorts = QSerialPortInfo::availablePorts();

        // Check if there is a new port that was not in the previous list
        for (const QSerialPortInfo &portInfo : currentPorts)
        {
            if (!availablePorts.contains(portInfo))
            {
                qDebug() << "New COM Port Detected:";
                qDebug() << "Port Name:" << portInfo.portName();
                qDebug() << "Description:" << portInfo.description();
                qDebug() << "Manufacturer:" << portInfo.manufacturer();
                qDebug() << "Serial Number:" << portInfo.serialNumber();
                qDebug() << "System Location:" << portInfo.systemLocation();
            }
        }

        // Update the stored list of available ports
        availablePorts = currentPorts;
    }

private:
    QTimer *timer;
    QList<QSerialPortInfo> availablePorts;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ComPortMonitor monitor;

    return a.exec();
}

#include "main.moc"