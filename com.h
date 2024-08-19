#ifndef COMPORTMONITOR_H
#define COMPORTMONITOR_H

#include <QObject>
#include <QSerialPortInfo>
#include <QTimer>
#include <QList>

class ComPortMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ComPortMonitor(QObject *parent = nullptr);

private slots:
    void checkForNewPorts();

private:
    QTimer *timer;
    QList<QSerialPortInfo> availablePorts;
};

#endif // COMPORTMONITOR_H