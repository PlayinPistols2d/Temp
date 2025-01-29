#include "pack_parameters.h"

// Функция для конвертации float в IEEE 754 (32 бит) и представление в виде массива байтов (Little Endian)
QByteArray floatToBytes(float value)
{
    QByteArray arr(4, 0);
    QDataStream stream(&arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return arr;
}

// Функция для конвертации double в IEEE 754 (64 бит) и представление в виде массива байтов (Little Endian)
QByteArray doubleToBytes(double value)
{
    QByteArray arr(8, 0);
    QDataStream stream(&arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << value;
    return arr;
}

// Преобразование массива байтов (кратно 2) в массив 16-битных слов (LE)
QVector<quint16> bytesToWords(const QByteArray &bytes)
{
    QVector<quint16> words;
    for (int i = 0; i < bytes.size(); i += 2)
    {
        quint16 word = (quint8)bytes[i] | ((quint8)bytes[i + 1] << 8);
        words.append(word);
    }
    return words;
}

// Разворот порядка слов в зависимости от длины (endian swap)
void reverseWords(QVector<quint16> &words)
{
    std::reverse(words.begin(), words.end());
}

// Функция вставки бит в 16-битное слово
void insertBitsInWord(QVector<quint16> &words, int wordIndex, int startBit, int bitCount, quint64 valBits)
{
    quint64 mask = (1ULL << bitCount) - 1ULL;
    quint16 part = static_cast<quint16>(valBits & mask) << startBit;
    quint16 clearMask = static_cast<quint16>(~(mask << startBit));
    words[wordIndex] = (words[wordIndex] & clearMask) | part;
}

// Основная функция упаковки параметров
QVector<quint16> packParameters(const QList<Param> &params)
{
    int maxWordIndex = 0;
    for (const auto &p : params)
        maxWordIndex = qMax(maxWordIndex, p.endWord);

    QVector<quint16> words(maxWordIndex + 1, 0);

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

        int bitsLeft = bitCount, currentBitPos = 0, wIndex = param.startWord, bitPosInWord = param.startBit;

        while (bitsLeft > 0 && wIndex <= param.endWord)
        {
            int freeBitsInThisWord = 16 - bitPosInWord;
            int putNow = qMin(freeBitsInThisWord, bitsLeft);
            quint64 mask = (1ULL << putNow) - 1ULL;
            quint64 part = (allBits >> currentBitPos) & mask;
            insertBitsInWord(words, wIndex, bitPosInWord, putNow, part);

            bitsLeft -= putNow;
            currentBitPos += putNow;
            bitPosInWord = 0;
            wIndex++;
        }
    }

    for (int i = 0; i < words.size(); i++)
        qDebug() << QString("Word[%1] = 0x%2").arg(i).arg(words[i], 4, 16, QChar('0')).toUpper();

    return words;
}