#include <QVector>
#include <QStringList>
#include <QString>

QStringList convertWordsToHexStrings(const QVector<quint16> &words)
{
    QStringList list;
    for (quint16 w : words) {
        // Формируем строку в hex, длиной 4 символа (16-бит => 4 hex-цифры)
        QString hexStr = QString("%1").arg(w, 4, 16, QChar('0')).toUpper();
        // Например, если w = 0x01AB, то hexStr станет "01AB"
        list << hexStr;
    }
    return list;
}