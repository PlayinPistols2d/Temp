#include <QVector>
#include <QDebug>
#include <algorithm> // std::reverse

// Структура параметра
struct Param
{
    double value;   // само значение (может быть 5.0, 3.14, 123.0 и т.д.)
    int startBit;   // начальный бит в слове
    int endBit;     // конечный бит в слове
    int startWord;  // индекс начального 16-битного слова
    int endWord;    // индекс конечного 16-битного слова
    QString tp;     // "int", "uint", "float", "double"
};

//------------------------------------------------------------------------------
// Вспомогательные функции
//------------------------------------------------------------------------------

// Преобразуем float -> 4 байта (Little-Endian по байтам)
QByteArray floatToBytes(float f)
{
    QByteArray arr;
    arr.resize(4);
    union {
        float f;
        quint32 u32;
    } convert;
    convert.f = f;
    for (int i = 0; i < 4; ++i) {
        arr[i] = static_cast<char>((convert.u32 >> (8 * i)) & 0xFF);
    }
    return arr;
}

// Преобразуем double -> 8 байт (Little-Endian по байтам)
QByteArray doubleToBytes(double d)
{
    QByteArray arr;
    arr.resize(8);
    union {
        double d;
        quint64 u64;
    } convert;
    convert.d = d;
    for (int i = 0; i < 8; ++i) {
        arr[i] = static_cast<char>((convert.u64 >> (8 * i)) & 0xFF);
    }
    return arr;
}

// Разбиваем массив байтов (количество байт кратно 2) на 16-битные слова.
// bytes[0] = младший байт слова0, bytes[1] = старший байт слова0, и т.д.
QVector<quint16> bytesToWords(const QByteArray &bytes)
{
    QVector<quint16> words;
    int n = bytes.size() / 2;
    words.resize(n);

    for (int i = 0; i < n; ++i) {
        quint8 low  = static_cast<quint8>(bytes[2*i]);
        quint8 high = static_cast<quint8>(bytes[2*i + 1]);
        quint16 w = (static_cast<quint16>(high) << 8) | low;
        words[i] = w;
    }
    return words;
}

// Переворачиваем порядок 16-битных слов (w0, w1, w2, w3 -> w3, w2, w1, w0)
void reverseWords(QVector<quint16> &words)
{
    std::reverse(words.begin(), words.end());
}

// Вставка кусочка (до 16 бит) в конкретное слово `words[wordIndex]`
// начиная с бита `startBit`.
void insertBitsInWord(QVector<quint16> &words,
                      int wordIndex,
                      int startBit,
                      int bitCount,
                      quint64 valBits)
{
    // Маска на bitCount
    quint64 mask = (bitCount == 64) ? 0xFFFFFFFFFFFFFFFFULL
                                    : ((1ULL << bitCount) - 1ULL);

    // Берём младшие bitCount бит из valBits
    quint16 part = static_cast<quint16>(valBits & mask);

    // Сдвигаем в нужное положение
    part <<= startBit;

    // Формируем маску очистки в итоговом слове
    quint16 clearMask = static_cast<quint16>( ~(mask << startBit) );

    words[wordIndex] = (words[wordIndex] & clearMask) | part;
}

//------------------------------------------------------------------------------
// Основная функция упаковки
//------------------------------------------------------------------------------
QVector<quint16> packParameters(const QList<Param> &params)
{
    // 1) Найдём максимальное число слов
    int maxWordIndex = 0;
    for (auto &p : params) {
        if (p.endWord > maxWordIndex) {
            maxWordIndex = p.endWord;
        }
    }

    // 2) Создаём буфер слов, заполняем нулями
    QVector<quint16> words(maxWordIndex + 1, 0);

    // 3) Перебираем параметры по порядку
    for (const auto &param : params)
    {
        // Сколько бит всего занимает параметр
        int bitCount = param.endBit - param.startBit + 1;

        // Сформируем "сырые" 16-битные слова, соответствующие значению параметра
        QVector<quint16> paramWords;

        if (param.tp == "float")
        {
            float fVal = static_cast<float>(param.value);
            QByteArray arr = floatToBytes(fVal);  // 4 байта
            paramWords = bytesToWords(arr);       // => 2 слова
            reverseWords(paramWords);             // word-level swap (2 слова меняем местами)
        }
        else if (param.tp == "double")
        {
            double dVal = param.value;
            QByteArray arr = doubleToBytes(dVal); // 8 байт
            paramWords = bytesToWords(arr);       // => 4 слова
            reverseWords(paramWords);             // word-level swap (4 слова переворачиваем)
        }
        else if (param.tp == "int")
        {
            // Считаем, что value содержит целое в виде double (например, 5.0).
            // Переводим его в signed 64-бит.
            qint64 ival = static_cast<qint64>(param.value);
            // Оставляем только нужные bitCount младших бит
            quint64 mask = (bitCount < 64) ? ((1ULL << bitCount) - 1ULL)
                                           : 0xFFFFFFFFFFFFFFFFULL;
            quint64 raw = static_cast<quint64>(ival & mask);

            // Сформируем массив байтов (минимум, чтобы уместить bitCount)
            int bytesNeeded = (bitCount + 7) / 8; // округление вверх
            QByteArray arr(bytesNeeded, 0);
            for (int i = 0; i < bytesNeeded; ++i) {
                arr[i] = static_cast<char>((raw >> (8 * i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);
            // Если param занимает более 16 бит, можно тоже «перевернуть» слова
            // (для единообразия с float/double). Обычно, если bitCount > 16, 
            // вы хотите 2+ слов => перевернём:
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else if (param.tp == "uint")
        {
            // Аналогично, но без знака
            quint64 uval = static_cast<quint64>(param.value);
            quint64 mask = (bitCount < 64) ? ((1ULL << bitCount) - 1ULL)
                                           : 0xFFFFFFFFFFFFFFFFULL;
            quint64 raw = uval & mask;

            int bytesNeeded = (bitCount + 7) / 8;
            QByteArray arr(bytesNeeded, 0);
            for (int i = 0; i < bytesNeeded; ++i) {
                arr[i] = static_cast<char>((raw >> (8 * i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else
        {
            // неизвестный тип - пропускаем или обрабатываем как ошибку
            continue;
        }

        // Теперь у нас есть QVector<quint16> paramWords, где:
        //   - paramWords[0] = младшие 16 бит параметра (уже с учётом «word swap»)
        //   - paramWords[1] = следующие 16 бит и т.д.

        // Осталось «рассыпать» эти биты по словам [startWord..endWord], битам [startBit..endBit].
        // С учётом того, что параметры идут строго друг за другом, 
        // нам нужно просто аккуратно разложить bitCount бит.

        // Упакуем все 16-битные слова paramWords в одно 64-битное число (если bitCount <= 64),
        // либо можно делать по кускам в цикле. Для наглядности — пример с 64-бит.

        // Если bitCount может быть > 64, нужно будет расширять логику (QVector<quint16> -> 128 бит и т.д.).
        // Для простоты предположим, что здесь не бывает параметров длиннее 64 бит.
        quint64 allBits = 0;
        {
            // paramWords[0] идут в младшие 16 бит,
            // paramWords[1] идут в следующие 16 бит и т.д.
            // В том порядке, в каком сейчас лежат paramWords (уже перевёрнутом).
            for (int i = 0; i < paramWords.size(); ++i) {
                quint64 w = static_cast<quint64>(paramWords[i]);
                allBits |= (w << (16 * i)); 
            }
        }

        // Теперь разложим allBits в итоговый буфер words.
        // Нужно заполнить диапазон слов от startWord до endWord,
        // и бит от startBit до endBit. Пойдём побитовому циклу:
        int bitsLeft = bitCount;
        int currentBitPos = 0;           // сколько бит уже «забрали» из allBits
        int wIndex = param.startWord;    
        int bitPosInWord = param.startBit;

        while (bitsLeft > 0 && wIndex <= param.endWord)
        {
            int freeBitsInThisWord = 16 - bitPosInWord;
            int putNow = qMin(freeBitsInThisWord, bitsLeft);

            // Выдёргиваем putNow бит из allBits, начиная с currentBitPos
            quint64 mask = (1ULL << putNow) - 1ULL;
            quint64 part = (allBits >> currentBitPos) & mask;

            // Вставляем part в слово words[wIndex], начиная с bitPosInWord
            insertBitsInWord(words, wIndex, bitPosInWord, putNow, part);

            // Сдвигаемся
            bitsLeft       -= putNow;
            currentBitPos  += putNow;
            bitPosInWord   = 0;         // в следующем слове начинаем с бита 0
            wIndex++;
        }
    }

    // 4) Выводим результат для отладки
    for (int i = 0; i < words.size(); ++i) {
        // Печатаем, например, так: "Word 0: 0x0000", "Word 1: 0x00A3"...
        QString hexStr = QString("Word %1: 0x%2")
                .arg(i)
                .arg(words[i], 4, 16, QChar('0')).toUpper();
        qDebug() << hexStr;
    }

    // 5) Возвращаем готовый вектор слов
    return words;
}