#include "pack_parameters.h"

// Функция преобразования float в IEEE 754 (32-битный)
QByteArray floatToBytes(float value)
{
    QByteArray arr(4, 0);
    QDataStream stream(&arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return arr;
}

// Функция преобразования double в IEEE 754 (64-битный)
QByteArray doubleToBytes(double value)
{
    QByteArray arr(8, 0);
    QDataStream stream(&arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return arr;
}

// Преобразование массива байтов в 16-битные слова (Little Endian)
QVector<quint16> bytesToWords(const QByteArray &bytes)
{
    QVector<quint16> words;
    for (int i = 0; i < bytes.size(); i += 2)
    {
        quint16 word = static_cast<quint8>(bytes[i]) | (static_cast<quint8>(bytes[i + 1]) << 8);
        words.append(word);
    }
    return words;
}

// Разворот порядка слов для Endian-соответствия
void reverseWords(QVector<quint16> &words)
{
    std::reverse(words.begin(), words.end());
}

// Функция для вставки бит в битовый буфер (битовая маска)
void insertBitsInBuffer(QVector<quint16> &buffer, int startBit, int bitCount, quint64 value)
{
    int wordIndex = startBit / 16;
    int bitOffset = startBit % 16;

    while (bitCount > 0)
    {
        int bitsInThisWord = qMin(16 - bitOffset, bitCount);
        quint16 mask = (1 << bitsInThisWord) - 1;
        quint16 part = static_cast<quint16>((value & mask) << bitOffset);

        buffer[wordIndex] |= part;
        value >>= bitsInThisWord;
        bitCount -= bitsInThisWord;
        bitOffset = 0;
        wordIndex++;
    }
}

// Основная функция упаковки параметров
QVector<quint16> packParameters(const QList<Param> &params)
{
    int maxWordIndex = 0;
    for (const auto &p : params)
        maxWordIndex = qMax(maxWordIndex, p.endWord);

    QVector<quint16> buffer(maxWordIndex + 1, 0);

    for (const auto &param : params)
    {
        int bitCount = param.endBit - param.startBit + 1;
        if (bitCount <= 0)
            continue;

        QVector<quint16> paramWords;
        QByteArray arr;

        if (param.tp == "float")
            arr = floatToBytes(static_cast<float>(param.value));
        else if (param.tp == "double")
            arr = doubleToBytes(param.value);
        else
        {
            quint64 raw = static_cast<quint64>(param.value) & ((1ULL << bitCount) - 1ULL);
            int bytesNeeded = (bitCount + 7) / 8;
            arr.resize(bytesNeeded);
            for (int i = 0; i < bytesNeeded; i++)
                arr[i] = static_cast<char>((raw >> (8 * i)) & 0xFF);
        }

        paramWords = bytesToWords(arr);
        if (paramWords.size() > 1)
            reverseWords(paramWords);

        quint64 allBits = 0;
        for (int i = 0; i < paramWords.size(); i++)
            allBits |= (static_cast<quint64>(paramWords[i]) << (16 * i));

        insertBitsInBuffer(buffer, param.startWord * 16 + param.startBit, bitCount, allBits);
    }

    for (int i = 0; i < buffer.size(); i++)
        qDebug() << QString("Word[%1] = 0x%2").arg(i).arg(buffer[i], 4, 16, QChar('0')).toUpper();

    return buffer;
}