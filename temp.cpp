#include "HexConverter.h"
#include <QtEndian>
#include <QDataStream>
#include <QDebug>

QVector<quint16> HexConverter::ConvertParametersToHex(const QVector<Parameter>& parameters) {
    QVector<quint16> result;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian); // Используем младший порядок байтов

    int currentWord = -1;  // Текущий индекс слова
    QByteArray wordBuffer(2, 0); // Буфер для незаполненного слова (2 байта = 16 бит)

    for (int i = 0; i < parameters.size(); ++i) {
        const Parameter& param = parameters[i];

        // Проверяем разрыв в словах (если param.GetStartWord() > currentWord + 1)
        while (currentWord != -1 && param.GetStartWord() > currentWord + 1) {
            result.append(0);  // Добавляем пустое слово
            currentWord++;
        }

        int wordCount = param.GetEndWord() - param.GetStartWord() + 1;
        QByteArray hexData = ConvertToHexBytes(param.GetValue(), param.GetTp(), wordCount);

        if (param.GetStartWord() == param.GetEndWord()) {
            // Параметр занимает одно слово
            if (param.GetStartBit() == 0 && param.GetEndBit() == 15) {
                // Полностью занимает 16 бит
                stream.writeRawData(hexData.constData(), 2);
                result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())));
            } else {
                // Часть слова (добавляем битовую маску)
                int bitSize = param.GetEndBit() - param.GetStartBit() + 1;
                quint16 mask = (1 << bitSize) - 1;
                quint16 value = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData())) & mask;

                if (param.GetStartWord() != currentWord) {
                    if (!wordBuffer.isEmpty() && currentWord != -1) {
                        result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData())));
                    }
                    wordBuffer.fill(0);
                    currentWord = param.GetStartWord();
                }

                quint16 oldValue = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData()));
                oldValue |= (value << param.GetStartBit());
                qToLittleEndian(oldValue, reinterpret_cast<uchar*>(wordBuffer.data()));
            }
        } else {
            // Параметр занимает несколько слов
            for (int j = 0; j < hexData.size(); j += 2) {
                quint16 word = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(hexData.constData() + j));
                result.append(word);
                currentWord = param.GetStartWord() + j / 2;
            }
        }
    }

    // Добавление последнего буферного слова
    if (!wordBuffer.isEmpty() && currentWord != -1) {
        result.append(qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(wordBuffer.constData())));
    }

    return ApplyEndianSwap(result);
}

QByteArray HexConverter::ConvertToHexBytes(double value, const QString& type, int wordCount) {
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

QVector<quint16> HexConverter::ApplyEndianSwap(const QVector<quint16>& words) {
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