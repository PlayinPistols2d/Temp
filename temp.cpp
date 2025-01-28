ParamBits.h

#ifndef PARAMBITS_H
#define PARAMBITS_H

#include <QVector>

/*!
 * \brief Структура, описывающая один параметр, который будет уложен в диапазон бит
 *        [startBit..endBit] внутри 16-битных слов [startWord..endWord].
 *
 *  value     - вещественное число (double). Но по протоколу:
 *              если занимает одно 16-битное слово, считаем его "целым";
 *              если занимает больше слова, сохраняем его в IEEE754 (float или double).
 *  startBit  - начальный бит (0..15) внутри слова
 *  endBit    - конечный бит (включительно)
 *  startWord - индекс начального 16-битного слова
 *  endWord   - индекс конечного 16-битного слова
 */
struct Param
{
    double value;
    int startBit;
    int endBit;
    int startWord;
    int endWord;
};

/*!
 * \brief buildWordsFromParams
 *
 * Алгоритм:
 *  1) Если (startWord == endWord) => укладываем param.value как "целое"
 *     (2's complement при отрицательных), обрезав по нужному кол-ву бит.
 *
 *  2) Если (startWord != endWord) => укладываем param.value в формат IEEE754:
 *       - Если (endBit - startBit + 1) <= 32 => 32-битный float
 *       - Иначе => 64-битный double
 *     Если у поля бит меньше, чем 32/64, то "лишние" биты IEEE754 обрезаются,
 *     если больше — остаток нулями.
 *
 * \param params список параметров
 * \return вектор 16-битных слов, в которые все параметры уложены
 */
QVector<quint16> buildWordsFromParams(const QVector<Param> &params);

#endif // PARAMBITS_H

ParamBits.cpp

#include "ParamBits.h"
#include <QtGlobal> // quint16, quint64, ...
#include <QDebug>
#include <cmath>    // std::llround, std::fabs, etc.

//------------------------------------------------------------------------------
// Утилита: извлечь сырые 32 бита float (IEEE754)
static quint32 floatToRawBits(float f)
{
    static_assert(sizeof(float) == 4, "Float must be 4 bytes");
    union {
        float f;
        quint32 u32;
    } conv;
    conv.f = f;
    return conv.u32;
}

//------------------------------------------------------------------------------
// Утилита: извлечь сырые 64 бита double (IEEE754)
static quint64 doubleToRawBits(double d)
{
    static_assert(sizeof(double) == 8, "Double must be 8 bytes");
    union {
        double d;
        quint64 u64;
    } conv;
    conv.d = d;
    return conv.u64;
}

//------------------------------------------------------------------------------
// setBitsInBuffer - копирует биты из 'raw' в промежуток [globalStartBit..globalEndBit]
// внутри массива слов words, по одному биту
static void setBitsInBuffer(QVector<quint16> &words,
                            int globalStartBit,
                            int globalEndBit,
                            quint64 raw)
{
    int sizeInBits = globalEndBit - globalStartBit + 1;
    for (int bitIndex = 0; bitIndex < sizeInBits; ++bitIndex) {
        quint64 bitVal = (raw >> bitIndex) & 0x1;

        int globalBit = globalStartBit + bitIndex;
        int wordIndex = globalBit / 16;
        int bitInWord = globalBit % 16;

        if (bitVal == 1) {
            words[wordIndex] |= (1 << bitInWord);
        }
        // если 0, то ничего делать не нужно, там уже ноль
    }
}

//------------------------------------------------------------------------------
QVector<quint16> buildWordsFromParams(const QVector<Param> &params)
{
    // 1. Определяем, сколько всего слов понадобится
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // 2. Создаём вектор 16-битных слов (size = maxWord+1), заполненный нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 3. Обрабатываем параметры
    for (const auto &param : params) {
        int sizeInBits = param.endBit - param.startBit + 1;
        // Глобальное начало и конец бит
        int globalStartBit = param.startWord * 16 + param.startBit;
        int globalEndBit   = param.endWord   * 16 + param.endBit;

        // Сколько 16-битных слов реально нужно параметру
        int numWords = (param.endWord - param.startWord + 1);

        if (numWords == 1) {
            //------------------------------------------------------------------
            // 1) Параметр умещается в одно 16-битное слово
            // => трактуем значение как "целое" (2's complement при отриц.)
            //------------------------------------------------------------------
            // Округлим double -> qint64
            qint64 signedVal = static_cast<qint64>(std::llround(param.value));
            quint64 raw = static_cast<quint64>(signedVal);

            // Обрезаем до sizeInBits
            if (sizeInBits < 64) {
                quint64 mask = (1ULL << sizeInBits) - 1ULL;
                raw &= mask;
            }

            setBitsInBuffer(words, globalStartBit, globalEndBit, raw);
        }
        else {
            //------------------------------------------------------------------
            // 2) Параметр занимает более одного слова => используем IEEE754
            //------------------------------------------------------------------
            // Если поле <= 32 бита, пишем float, иначе double
            if (sizeInBits <= 32) {
                // 32-битный float
                float fVal = static_cast<float>(param.value);
                quint32 bits32 = floatToRawBits(fVal);
                quint64 raw = bits32; // расширим до 64 для удобства

                // Если поле меньше 32 бит, обрежем лишнее
                if (sizeInBits < 32) {
                    quint64 mask = (1ULL << sizeInBits) - 1ULL;
                    raw &= mask;
                }
                // Если поле >32 бит, старшие биты останутся нулями
                setBitsInBuffer(words, globalStartBit, globalEndBit, raw);
            }
            else {
                // 64-битный double
                quint64 bits64 = doubleToRawBits(param.value);
                quint64 raw = bits64;

                // Если поле меньше 64 бит, обрежем
                if (sizeInBits < 64) {
                    quint64 mask = (1ULL << sizeInBits) - 1ULL;
                    raw &= mask;
                }
                // Если поле >64, сверх 64 бит у нас всё равно некуда брать, 
                // они останутся нулями
                setBitsInBuffer(words, globalStartBit, globalEndBit, raw);
            }
        }
    }

    // 4. (Опционально) Выводим результат в debug (можно убрать или модифицировать)
    for (int i = 0; i < words.size(); ++i) {
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'))
                    .toUpper();
    }

    return words;
}

К