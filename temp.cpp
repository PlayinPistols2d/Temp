#define PARAMBITS_H

#include <QtGlobal>    // Для quint16, quint64, qint64
#include <QVector>

// Признак, как интерпретировать значение
enum class ParamType {
    Integer,
    Float32
};

// Описание одного параметра
struct Param
{
    double value;   // Исходное значение (вещественное или целое)
    int startBit;   // Начальный бит (0..15) внутри слова
    int endBit;     // Конечный бит (включительно)
    int startWord;  // Индекс начального 16-битного слова
    int endWord;    // Индекс конечного 16-битного слова
    ParamType type; // Тип данных: Integer или Float32
};

// Основная функция. Собирает все параметры в вектор 16-битных слов.
QVector<quint16> buildWordsFromParams(const QVector<Param> &params);

#endif // PARAMBITS_H

ParamBits.cpp (исходный файл)

#include "ParamBits.h"
#include <cmath>       // std::llround и т.п.
#include <QDebug>      // qDebug()
#include <QString>

//------------------------------------------------------------------------------
// Служебная функция: берёт "сырые" биты float (IEEE754, 32 бита)
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
// Служебная функция: "раскладывает" значение (raw) по битам
// с globalStartBit по globalEndBit (включительно), двигаясь по words[...].
static void setBitsInBuffer(QVector<quint16> &words,
                            int globalStartBit,
                            int globalEndBit,
                            quint64 raw)
{
    int sizeInBits = globalEndBit - globalStartBit + 1;
    for (int bitIndex = 0; bitIndex < sizeInBits; ++bitIndex) {
        // Берём один бит из raw
        quint64 bitVal = (raw >> bitIndex) & 0x1;

        // Текущий абсолютный номер бита среди всех слов
        int globalBit = globalStartBit + bitIndex;
        // Определяем, в каком слове и каком бите внутри слова мы находимся
        int wordIndex = globalBit / 16;
        int bitInWord = globalBit % 16;

        if (bitVal == 1) {
            words[wordIndex] |= (1 << bitInWord);
        }
        // Если bitVal == 0, ничего делать не нужно, там уже 0
    }
}

//------------------------------------------------------------------------------
// Основная функция
QVector<quint16> buildWordsFromParams(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего слов понадобится
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // 2) Создаём вектор (maxWord+1) слов, заполненных нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 3) Обрабатываем каждый параметр
    for (const auto &param : params) {
        int sizeInBits = param.endBit - param.startBit + 1;

        // Глобальная нумерация бит: какие биты мы занимаем
        int globalStartBit = param.startWord * 16 + param.startBit;
        int globalEndBit   = param.endWord   * 16 + param.endBit;

        if (param.type == ParamType::Float32)
        {
            // Всегда берём 32-битное представление (float) в формате IEEE754
            float fVal = static_cast<float>(param.value);
            quint32 bits32 = floatToRawBits(fVal);

            // Преобразуем в 64-битную переменную, чтобы удобно "рассыпать" биты
            quint64 raw = static_cast<quint64>(bits32);

            // Если поле меньше 32 бит, обрежем
            if (sizeInBits < 32) {
                quint64 mask = ((1ULL << sizeInBits) - 1ULL);
                raw &= mask;
            }
            // Если поле больше 32 бит - всё равно запишем только 32 бита,
            // остальные (старшие) биты так и останутся нулями.

            setBitsInBuffer(words, globalStartBit, globalEndBit, raw);
        }
        else
        {
            // Параметр типа Integer (целое число)
            // Считаем, что при этом value может быть и отрицательным, округлим:
            qint64 signedVal = static_cast<qint64>(std::llround(param.value));

            // Переводим в беззнаковый для удобства битовых операций
            quint64 raw = static_cast<quint64>(signedVal);

            // Обрезаем по нужному количеству бит (если надо)
            if (sizeInBits < 64) {
                quint64 mask = ((1ULL << sizeInBits) - 1ULL);
                raw &= mask;
            }

            setBitsInBuffer(words, globalStartBit, globalEndBit, raw);
        }
    }

    // 4) Отладочный вывод слов в hex (можно убрать или переделать при необходимости)
    for (int i = 0; i < words.size(); ++i) {
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'))
                    .toUpper();
    }

    return words;
}

П
