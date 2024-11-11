#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QVector>
#include <QTimer>

struct LogEntry {
    QString timestamp;
    QString eventType;
    QString who;
    QString comment;
};

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& getInstance();
    void logEvent(const QString &eventType, const QString &who, const QString &comment);

private:
    QVector<LogEntry> logBuffer;
    QMutex mutex;
    QTimer *flushTimer;
    QThread *workerThread;

    Logger();  // Private constructor for singleton pattern
    ~Logger(); // Destructor

    void flushLogs(); // Method to flush logs to the database

    // Disable copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H





#include "Logger.h"
#include <QDateTime>
#include <QDebug>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    // Move the logger to a separate thread
    workerThread = new QThread();
    this->moveToThread(workerThread);
    connect(workerThread, &QThread::started, this, [&]() {
        // Setup periodic flushing of logs
        flushTimer = new QTimer();
        connect(flushTimer, &QTimer::timeout, this, &Logger::flushLogs);
        flushTimer->start(5000); // Flush every 5 seconds
    });

    workerThread->start();
}

Logger::~Logger() {
    flushTimer->stop();
    workerThread->quit();
    workerThread->wait();
    delete flushTimer;
    delete workerThread;
}

void Logger::logEvent(const QString &eventType, const QString &who, const QString &comment) {
    QMutexLocker locker(&mutex);

    // Create a log entry with the current timestamp
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    entry.eventType = eventType;
    entry.who = who;
    entry.comment = comment;

    // Add the log entry to the buffer
    logBuffer.append(entry);
}

void Logger::flushLogs() {
    QMutexLocker locker(&mutex);

    if (logBuffer.isEmpty()) {
        return; // No logs to flush
    }

    // Example output (replace this with actual database insertion logic)
    qDebug() << "Flushing logs to database. Number of logs:" << logBuffer.size();
    for (const LogEntry &entry : logBuffer) {
        qDebug() << "[" << entry.timestamp << "]"
                 << "Type:" << entry.eventType
                 << "Who:" << entry.who
                 << "Comment:" << entry.comment;
    }

    // Clear the buffer after flushing
    logBuffer.clear();
}




