#include <QList>
#include <QVector>
#include <QDebug>
#include <algorithm> // std::reverse

struct Param
{
    double value;
    int startBit;
    int endBit;
    int startWord;
    int endWord;
    QString tp;  // "int", "uint", "float", "double"
};

// ---------- Вспомогательные функции ----------

// int->байты (LE), float->байты(IEEE754), double->байты(IEEE754) и т.п.
QByteArray floatToBytes(float f);
QByteArray doubleToBytes(double d);

// Преобразовать массив байтов (кол-во байт кратно 2) в массив 16-битных слов
QVector<quint16> bytesToWords(const QByteArray &bytes)
{
    QVector<quint16> words;
    words.resize(bytes.size()/2);

    for (int i = 0; i < words.size(); ++i) {
        quint8 low  = static_cast<quint8>(bytes[2*i]);
        quint8 high = static_cast<quint8>(bytes[2*i + 1]);
        quint16 w   = (static_cast<quint16>(high) << 8) | low;
        words[i] = w;
    }
    return words;
}

// Переворот порядка 16-битных слов, если надо
void reverseWords(QVector<quint16> &words)
{
    std::reverse(words.begin(), words.end());
}

// Вставить (до 16) бит в одно 16-битное слово.
void insertBitsInWord(QVector<quint16> &words,
                      int wordIndex,
                      int startBit,
                      int bitCount,
                      quint64 valBits)
{
    // сформируем маску на bitCount
    quint64 mask = (1ULL << bitCount) - 1ULL;
    // берём нужные (младшие) bitCount бит
    quint16 part = static_cast<quint16>(valBits & mask);

    // сдвигаем в нужную позицию
    part <<= startBit;

    // подготавливаем "очиститель" нужных бит в target-слове
    quint16 clearMask = static_cast<quint16>( ~(mask << startBit) );

    words[wordIndex] = (words[wordIndex] & clearMask) | part;
}

// ------------------------------------------------
// Основная функция, показывающая, как обрабатывается
// и полный параметр (занимающий 1-2-4 слова), и
// "кусочный" (несколько бит в слове).
// ------------------------------------------------

QVector<quint16> packParameters(const QList<Param> &params)
{
    // 1) Найдём, сколько всего слов нужно
    int maxWord = 0;
    for (auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // 2) Создадим буфер (maxWord+1) 16-битных слов, обнулим
    QVector<quint16> words(maxWord + 1, 0);

    // 3) По каждому параметру отдельно
    for (auto &param : params)
    {
        int bitCount = param.endBit - param.startBit + 1;
        if (bitCount <= 0) {
            // некорректный параметр
            continue;
        }

        // Шаг а) Преобразовать Param.value к "сырым" 16-битным словам:
        QVector<quint16> paramWords;  // здесь окажутся слова (уже без дырок)

        if (param.tp == "float")
        {
            float fVal = static_cast<float>(param.value);
            QByteArray arr = floatToBytes(fVal);   // 4 байта
            paramWords = bytesToWords(arr);        // -> 2 слова
            reverseWords(paramWords);              // если в протоколе нужно "слово0,слово1 -> слово1,слово0"
        }
        else if (param.tp == "double")
        {
            double dVal = param.value;
            QByteArray arr = doubleToBytes(dVal);  // 8 байт
            paramWords = bytesToWords(arr);        // -> 4 слова
            reverseWords(paramWords);
        }
        else if (param.tp == "int")
        {
            qint64 ival = static_cast<qint64>(param.value);
            // ограничим числом bitCount (если 32 бита, или 5 бит, как угодно)
            quint64 mask = (bitCount < 64) ? ((1ULL << bitCount) - 1ULL) 
                                           : 0xFFFFFFFFFFFFFFFFULL;
            quint64 raw = static_cast<quint64>(ival) & mask;

            // сделаем массив байтов
            int bytesNeeded = (bitCount + 7)/8; // округл. вверх
            QByteArray arr(bytesNeeded, 0);
            for (int i = 0; i < bytesNeeded; ++i) {
                arr[i] = static_cast<char>((raw >> (8*i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);

            // Если нужно, переворачиваем:
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else if (param.tp == "uint")
        {
            quint64 uval = static_cast<quint64>(param.value);
            quint64 mask = (bitCount < 64) ? ((1ULL << bitCount) - 1ULL)
                                           : 0xFFFFFFFFFFFFFFFFULL;
            quint64 raw = uval & mask;

            int bytesNeeded = (bitCount + 7)/8;
            QByteArray arr(bytesNeeded, 0);
            for (int i = 0; i < bytesNeeded; ++i) {
                arr[i] = static_cast<char>((raw >> (8*i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else {
            // неизвестный тип
            continue;
        }

        // Шаг б) Теперь у нас есть paramWords — массив 16-битных слов,
        // содержащий bitCount нужных бит (с учётом того, что "лишние" биты
        // могут быть обрезаны маской выше).

        // Склеим paramWords в одно 64-битное число (allBits).
        // (Если bitCount может превышать 64, нужно будет расширять логику, это пример.)
        quint64 allBits = 0;
        for (int i = 0; i < paramWords.size(); ++i) {
            quint64 w = static_cast<quint64>(paramWords[i]);
            allBits |= (w << (16 * i));
        }

        // Шаг в) Побитово вставляем allBits в итоговый буфер words
        // в диапазон (startWord..endWord, startBit..endBit)
        // Даже если это один и тот же word, всё равно сработает цикл:

        int bitsLeft       = bitCount;
        int currentBitPos  = 0;                // сколько бит уже "забрали" из allBits
        int wIndex         = param.startWord;  
        int bitPosInWord   = param.startBit;   

        while (bitsLeft > 0 && wIndex <= param.endWord)
        {
            int freeBitsInThisWord = 16 - bitPosInWord;  // сколько бит свободно от bitPosInWord до конца 16 бит
            int putNow = qMin(freeBitsInThisWord, bitsLeft);

            // вытащим putNow бит из allBits, начиная с currentBitPos
            quint64 mask = (1ULL << putNow) - 1ULL;
            quint64 part = (allBits >> currentBitPos) & mask;

            // вставляем part в words[wIndex], начиная с бита bitPosInWord
            insertBitsInWord(words, wIndex, bitPosInWord, putNow, part);

            bitsLeft      -= putNow;
            currentBitPos += putNow;
            // для следующего слова начнём с бита 0:
            bitPosInWord   = 0;
            wIndex++;
        }
    }

    // 4) Отладочный вывод
    for (int i = 0; i < words.size(); ++i) {
        qDebug().nospace() << "Word[" << i << "] = 0x"
                           << QString::number(words[i], 16).toUpper().rightJustified(4, '0');
    }

    return words;
}