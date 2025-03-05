#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QDebug>

bool validateValue(const QString &typeStr, const QString &valueStr) {
    int typeId = QMetaType::type(typeStr.toUtf8().constData()); // Получаем ID типа

    if (typeId == QMetaType::UnknownType) {
        qWarning() << "Неизвестный тип:" << typeStr;
        return false;
    }

    QVariant value = QVariant::fromValue(valueStr);
    bool ok = false;

    switch (typeId) {
        case QMetaType::Bool:
            value = valueStr.toInt(&ok);
            break;
        case QMetaType::Int:
            value = valueStr.toInt(&ok);
            if (ok && (value.toInt() < std::numeric_limits<int>::min() || value.toInt() > std::numeric_limits<int>::max())) return false;
            break;
        case QMetaType::UInt:
            value = valueStr.toUInt(&ok);
            break;
        case QMetaType::LongLong:
            value = valueStr.toLongLong(&ok);
            if (ok && (value.toLongLong() < std::numeric_limits<qint64>::min() || value.toLongLong() > std::numeric_limits<qint64>::max())) return false;
            break;
        case QMetaType::ULongLong:
            value = valueStr.toULongLong(&ok);
            break;
        case QMetaType::Double:
            value = valueStr.toDouble(&ok);
            if (ok && (value.toDouble() < -std::numeric_limits<double>::max() || value.toDouble() > std::numeric_limits<double>::max())) return false;
            break;
        case QMetaType::Float:
            value = valueStr.toFloat(&ok);
            if (ok && (value.toFloat() < -std::numeric_limits<float>::max() || value.toFloat() > std::numeric_limits<float>::max())) return false;
            break;
        default:
            qWarning() << "Тип" << typeStr << "не поддерживается.";
            return false;
    }

    if (!ok) {
        qWarning() << "Ошибка преобразования значения:" << valueStr << "в тип" << typeStr;
        return false;
    }

    return true;
}