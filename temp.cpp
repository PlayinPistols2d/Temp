#include "HexConverter.h"
#include <QtEndian>
#include <QDataStream>
#include <QDebug>

QVector<quint16> HexConverter::convertParametersToHex(const QVector<Parameter>& parameters) {
    QVector<quint16> result;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian); // Выберите порядок байтов, если необходимо

    int currentWord = parameters.isEmpty() ? -1 : parameters.first().startWord;
    QByteArray wordBuffer(2, 0); // 2 байта на слово (16 бит)

    for (int i = 0; i < parameters.size(); ++i) {
        const Parameter& param = parameters[i];

        int wordCount = param.endWord - param.startWord + 1;
        QByteArray hexData = convertToHexBytes(param.value, param.type, wordCount);

        if (param.startWord == param.endWord) {
            if (param.startBit == 0 && param.endBit == 15) {
                // Полное слово
                stream.writeRawData(hexData.constData(), 2);
                result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())));
            } else {
                // Частичное заполнение слова
                int bitSize = param.endBit - param.startBit + 1;
                quint16 mask = (1 << bitSize) - 1;
                quint16 value = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())) & mask;
                
                if (param.startWord != currentWord) {
                    if (!wordBuffer.isEmpty()) {
                        result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData())));
                    }
                    wordBuffer.fill(0);
                    currentWord = param.startWord;
                }

                quint16 oldValue = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData()));
                oldValue |= (value << param.startBit);
                qToLittleEndian(oldValue, reinterpret_cast<uchar*>(wordBuffer.data()));
            }
        } else {
            // Параметр занимает несколько слов
            for (int j = 0; j < hexData.size(); j += 2) {
                quint16 word = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData() + j));
                result.append(word);
            }
        }
    }

    // Добавление последнего буферного слова, если есть незаполненные параметры
    if (!wordBuffer.isEmpty()) {
        result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData())));
    }

    return applyEndianSwap(result);
}

QByteArray HexConverter::convertToHexBytes(double value, const QString& type, int wordCount) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    if (type == "int") {
        qint32 intValue = static_cast<qint32>(value);
        stream << intValue;
    } else if (type == "uint") {
        quint32 uintValue = static_cast<quint32>(value);
        stream << uintValue;
    } else if (type == "float") {
        float floatValue = static_cast<float>(value);
        stream.writeRawData(reinterpret_cast<const char*>(&floatValue), 4);
    } else if (type == "double") {
        stream.writeRawData(reinterpret_cast<const char*>(&value), 8);
    }

    if (data.size() < wordCount * 2) {
        data.append(QByteArray(wordCount * 2 - data.size(), 0)); // Дополняем нулями
    }

    return data;
}

QVector<quint16> HexConverter::applyEndianSwap(const QVector<quint16>& words) {
    QVector<quint16> swappedWords;

    if (words.size() > 1) {
        for (int i = words.size() - 1; i >= 0; --i) {
            swappedWords.append(words[i]);
        }
    } else {
        swappedWords = words;
    }

    return swappedWords;
}