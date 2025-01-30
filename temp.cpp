#include "ParameterConverter.h"
#include <QtEndian>
#include <QDataStream>
#include <QDebug>

QVector<QByteArray> ParameterConverter::convertParameters(const QVector<Parameter>& parameters) {
    QVector<QByteArray> result;
    QByteArray bufferWord(2, 0); // 1 слово = 2 байта (16 бит)
    int currentWord = -1;

    for (const auto& param : parameters) {
        if (param.startWord == param.endWord) {
            // Параметр помещается в одно слово
            if (param.startBit == 0 && param.endBit == 15) {
                // Полное слово
                QByteArray word = convertToIEEE754(param.value, param.type);
                result.append(word.left(2)); // Берем первые 2 байта
            } else {
                // Частичное слово -> сборка буфера
                if (currentWord != param.startWord) {
                    if (currentWord != -1) {
                        result.append(bufferWord);
                    }
                    bufferWord.fill(0);
                    currentWord = param.startWord;
                }

                // Записываем часть параметра в нужные биты буферного слова
                quint16 wordValue = qFromBigEndian<quint16>(bufferWord.constData());
                quint16 paramBits = static_cast<quint16>(param.value) << (15 - param.endBit);
                wordValue |= paramBits;
                qToBigEndian(wordValue, bufferWord.data());
            }
        } else {
            // Параметр занимает несколько слов
            QByteArray converted = convertToIEEE754(param.value, param.type);
            int wordsRequired = (param.endWord - param.startWord) + 1;

            for (int i = 0; i < wordsRequired; ++i) {
                result.append(converted.mid(i * 2, 2)); // Записываем по 2 байта (слово)
            }
        }
    }

    if (!bufferWord.isEmpty()) {
        result.append(bufferWord);
    }

    // Применяем Endianness
    applyEndianSwap(result, result.size() / 2);

    return result;
}

QByteArray ParameterConverter::convertToIEEE754(double value, const QString& type) {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    if (type == "float") {
        float floatValue = static_cast<float>(value);
        stream << floatValue;
    } else if (type == "double") {
        stream << value;
    } else if (type == "int") {
        qint32 intValue = static_cast<qint32>(value);
        stream << intValue;
    } else if (type == "uint") {
        quint32 uintValue = static_cast<quint32>(value);
        stream << uintValue;
    }

    return byteArray;
}

void ParameterConverter::applyEndianSwap(QVector<QByteArray>& words, int wordCount) {
    for (int i = 0; i < wordCount / 2; ++i) {
        std::swap(words[i], words[wordCount - 1 - i]);
    }
}