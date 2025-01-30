#include "HexConverter.h"
#include <QtEndian>
#include <QDataStream>
#include <QDebug>

QVector<quint16> HexConverter::convertParametersToHex(const QVector<Parameter>& parameters) {
    QVector<quint16> result;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    int currentWord = parameters.isEmpty() ? -1 : parameters.first().startWord;
    quint16 tempWord = 0;  // Буферное слово

    for (int i = 0; i < parameters.size(); ++i) {
        const Parameter& param = parameters[i];

        // Обработка разрывов между словами
        if (param.startWord > currentWord + 1) {
            int gap = param.startWord - (currentWord + 1);
            for (int j = 0; j < gap; ++j) {
                result.append(0x0000);
            }
        }

        int wordCount = param.endWord - param.startWord + 1;
        QByteArray hexData = convertToHexBytes(param.value, param.type, wordCount);

        if (param.startWord == param.endWord) {
            if (param.startBit == 0 && param.endBit == 15) {
                // Полное слово
                result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())));
                tempWord = 0;  // Очистка буфера
            } else {
                // Частичное слово
                int bitSize = param.endBit - param.startBit + 1;
                quint16 mask = (1 << bitSize) - 1;
                quint16 value = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())) & mask;

                tempWord |= (value << param.startBit);

                // Проверка: есть ли ещё параметры в этом же слове?
                bool hasMoreParamsInSameWord = false;
                if (i + 1 < parameters.size()) {
                    if (parameters[i + 1].startWord == param.startWord) {
                        hasMoreParamsInSameWord = true;
                    }
                }

                if (!hasMoreParamsInSameWord) {
                    // Если это последний параметр в слове — дополняем нулями и записываем
                    result.append(tempWord);
                    tempWord = 0;
                }
            }
        } else {
            // Параметр занимает несколько слов
            for (int j = 0; j < hexData.size(); j += 2) {
                quint16 word = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData() + j));
                result.append(word);
            }
        }

        currentWord = param.endWord;
    }

    // Если в буфере осталось неполное слово, дополняем его нулями
    if (tempWord != 0) {
        result.append(tempWord);
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
        data.append(QByteArray(wordCount * 2 - data.size(), 0));  // Дополняем нулями
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