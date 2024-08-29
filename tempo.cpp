#include <QObject>
#include <QMetaMethod>
#include <QDebug>

void listConnections(QObject* sender, const char* signal)
{
    const QMetaObject* metaObject = sender->metaObject();
    int signalIndex = metaObject->indexOfSignal(QMetaObject::normalizedSignature(signal).constData());
    
    if (signalIndex < 0) {
        qWarning() << "Signal not found!";
        return;
    }

    QMetaMethod signalMethod = metaObject->method(signalIndex);
    
    // Introspect the sender object for connections
    const auto* connections = QObjectPrivate::get(sender)->connections();
    for (auto it = connections->begin(); it != connections->end(); ++it) {
        if (it->signalIndex == signalIndex) {
            QObject* receiver = it->receiver;
            int slotIndex = receiver->metaObject()->indexOfMethod(it->method);
            QMetaMethod slotMethod = receiver->metaObject()->method(slotIndex);

            qDebug() << "Signal" << signalMethod.methodSignature() << "is connected to"
                     << "Slot" << slotMethod.methodSignature() << "of object" << receiver;
        }
    }
}