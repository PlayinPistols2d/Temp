#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QDateTime>

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& getInstance();

    // Method to log events with specified parameters
    void logEvent(const QString &eventType, const QString &who, const QString &comment);

private:
    QMutex mutex;

    Logger();  // Private constructor for singleton pattern
    ~Logger(); // Destructor

    // Disable copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H








#include "Logger.h"
#include <QDebug> // Optional for console output during development

// Singleton instance method
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Constructor
Logger::Logger() {
    // Initialization code if needed
}

// Destructor
Logger::~Logger() {
    // Cleanup code if needed
}

// Method to log an event
void Logger::logEvent(const QString &eventType, const QString &who, const QString &comment) {
    QMutexLocker locker(&mutex); // Ensure thread safety

    // Creating a timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // Example output (this can be replaced with your custom logging implementation)
    qDebug() << "[" << timestamp << "]"
             << "Type:" << eventType
             << "Who:" << who
             << "Comment:" << comment;

    // Add code to forward this data to your custom database class or file logging here
}


