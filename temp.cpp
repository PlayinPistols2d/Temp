#include <QVector>
#include <QDebug>
#include <QtEndian> // при необходимости для дополнительных функций

enum class ParamType {
    Int,
    UInt,
    Float32,
    Float64
};

struct Param {
    double value;      // хранит число (может быть целым в виде double)
    int startBit;      // начальный бит (0..15) в startWord
    int endBit;        // конечный бит (0..15) в endWord
    int startWord;     // начальное слово (0..N)
    int endWord;       // конечное слово (startWord..N)
    ParamType tp;      // тип, от которого зависит, как интерпретировать value
};

///
/// \brief Преобразует double в 64-битный паттерн IEEE754
///
static quint64 doubleToRaw64(double d)
{
    static_assert(sizeof(double) == 8, "Double must be 8 bytes");
    union {
        double d;
        quint64 u64;
    } conv;
    conv.d = d;
    return conv.u64;
}

///
/// \brief Преобразует double (считая его float) в 32-битный паттерн IEEE754
///   Важно: мы явно приводим double к float, затем берём биты
///
static quint32 floatToRaw32(double d)
{
    static_assert(sizeof(float) == 4, "Float must be 4 bytes");
    union {
        float f;
        quint32 u32;
    } conv;
    conv.f = static_cast<float>(d);
    return conv.u32;
}

///
/// \brief Формирует вектор 16-битных слов по списку параметров
///        с учётом startBit, endBit, startWord, endWord и типа tp.
///
QVector<quint16> packParamsToWords(const QVector<Param> &params)
{
    // Определяем, сколько всего слов нам потенциально нужно.
    int maxWord = 0;
    for (auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // Буфер итоговых слов, инициализируем нулями.
    // +1, т.к. если maxWord=5, то нам нужны индексы [0..5].
    QVector<quint16> words(maxWord + 1, 0);

    // Функция "установить бит в нужное место" (в 16-битном слове)
    auto setBitInWord = [&](quint16 &word, int bitPos, bool bitVal){
        // bitPos от 0 до 15
        if (bitVal) {
            word |= (1 << bitPos);
        } else {
            word &= ~(1 << bitPos);
        }
    };

    // Для удобства — вспомогательная лямбда, чтобы выводить слово в hex
    auto debugWordHex = [&](int wIndex){
        // Допустим, хотим вывести в формате 0xABCD
        qDebug() << QString("word[%1] = 0x%2")
                    .arg(wIndex)
                    .arg(words[wIndex], 4, 16, QLatin1Char('0')).toUpper();
    };

    // Обрабатываем каждый параметр по очереди
    for (int pIndex = 0; pIndex < params.size(); ++pIndex) {
        const Param &p = params[pIndex];
        int paramBits = p.endBit - p.startBit + 1;

        // 1. Получаем "сырой" битовый паттерн из p.value согласно типу
        //    Здесь предполагается, что если тип int/uint, то можно взять
        //    нижние 32 (или 64) бит в зависимости от того, что вам нужно.
        //    Часто под int подразумевают 32 бита, но это уже детали протокола.
        //    Покажем оба случая условно.
        quint64 rawBits = 0; // будем хранить максимум 64 бита

        switch (p.tp) {
        case ParamType::Int: {
            // Предположим 32-битный int
            qint32 tmp = static_cast<qint32>(p.value); 
            // Приводим к 64, чтобы удобно дальше сдвигать
            rawBits = static_cast<quint64>(static_cast<qint64>(tmp));
            break;
        }
        case ParamType::UInt: {
            // Предположим тоже 32 бита
            quint32 tmp = static_cast<quint32>(p.value);
            rawBits = tmp;
            break;
        }
        case ParamType::Float32: {
            quint32 tmp = floatToRaw32(p.value);
            rawBits = tmp;
            break;
        }
        case ParamType::Float64: {
            quint64 tmp = doubleToRaw64(p.value);
            rawBits = tmp;
            break;
        }
        }

        // 2. Теперь у нас есть rawBits (до 64 бит). В реальности,
        //    если paramBits < 64, мы возьмём только младшие paramBits.
        //    Часто в протоколах, если это float32, то paramBits = 32 и т.д.
        //    Но допустим, что параметр может не все биты использовать.
        quint64 mask = 0;
        if (paramBits == 64) {
            mask = 0xFFFFFFFFFFFFFFFFull;
        } else {
            mask = ((1ull << paramBits) - 1ull);  // paramBits не более 64
        }
        quint64 paramValueBits = rawBits & mask;  // нужные нам биты

        // 3. Записываем эти биты в итоговый буфер в диапазон [startBit..endBit],
        //    распространяясь по словам [startWord..endWord].
        //    Идём по всем битам paramValueBits (от 0 до paramBits-1).
        //    bit i в paramValueBits пойдёт в общий буфер в позицию (startBit + i).
        for (int i = 0; i < paramBits; ++i) {
            // Берём i-й бит из paramValueBits
            bool bitVal = ((paramValueBits >> i) & 1ull) != 0;
            // Глобальная позиция бита относительно начала параметра:
            int globalBitPos = p.startBit + i;
            // Какое слово (16-бит) и какой бит внутри слова
            int wordIndex = p.startWord + (globalBitPos / 16);
            int bitInWord = globalBitPos % 16;
            // Устанавливаем
            setBitInWord(words[wordIndex], bitInWord, bitVal);
        }

        // 4. Если параметр "целиком" занимает несколько слов, делаем перестановку слов
        //    согласно правилу endian swap: для float32 (2 слова), для double64 (4 слова).
        //    (При условии, что startBit=0 и endBit=31 или 63, иначе будьте аккуратны!)
        int numWords = p.endWord - p.startWord + 1;
        // Для float32 ожидаем numWords=2, для float64 - 4.
        // Если это int/uint, смотрите по ситуациям.
        if ((p.tp == ParamType::Float32 && numWords == 2 && p.startBit == 0 && p.endBit == 31)
            || (p.tp == ParamType::Float64 && numWords == 4 && p.startBit == 0 && p.endBit == 63))
        {
            // Переворачиваем
            for (int i = 0; i < numWords/2; ++i) {
                qSwap(words[p.startWord + i], words[p.startWord + (numWords - 1 - i)]);
            }
        }
        // Аналогично можно прописать логику для int/uint, если они занимают ровно 2 или 4 слова.

        // 5. Вывод в debug. Здесь по условию может быть разная логика.
        //    Например, выводить все затронутые слова. Или только те, что "закрылись".
        //    Упростим: просто выведем все слова [startWord..endWord], раз уж мы их
        //    дописали.
        for (int w = p.startWord; w <= p.endWord; ++w) {
            debugWordHex(w);
        }
    }

    // Если нужно, можно в конце дополнительно вывести все слова:
    // for (int w = 0; w < words.size(); ++w) {
    //     debugWordHex(w);
    // }

    return words;
}

// ------------------------------------------------------------------
// Пример вызова:

void exampleUsage()
{
    QVector<Param> params;

    // Пример 1. Параметр float32, занимает 32 бита (2 слова):
    {
        Param p;
        p.value = 3.14;         // как double, но мы будем трактовать его как float
        p.startBit = 0;
        p.endBit = 31;
        p.startWord = 0;
        p.endWord = 1;          // итого 2 слова
        p.tp = ParamType::Float32;
        params.push_back(p);
    }

    // Пример 2. Два мелких int-параметра, влезают в одно слово (word=2):
    {
        // a) 8 бит, занимает биты [0..7] внутри word2
        Param p;
        p.value = 5.0;          // int 5 в double виде
        p.startBit = 0;
        p.endBit = 7;
        p.startWord = 2;
        p.endWord = 2;          // только одно слово
        p.tp = ParamType::Int;  // условно 32-бит int, но возьмём лишь 8 бит
        params.push_back(p);

        // b) 7 бит, биты [8..14] в том же word2
        Param p2;
        p2.value = 0x3F;        // 0b111111 = 63, но возьмём 7 бит
        p2.startBit = 8;
        p2.endBit = 14;
        p2.startWord = 2;
        p2.endWord = 2;
        p2.tp = ParamType::Int;
        params.push_back(p2);

        // c) 1 бит, бит [15..15] => добивает слово2
        Param p3;
        p3.value = 1.0;
        p3.startBit = 15;
        p3.endBit = 15;
        p3.startWord = 2;
        p3.endWord = 2;
        p3.tp = ParamType::UInt;
        params.push_back(p3);
    }

    // Пример 3. Параметр double64, занимает 64 бита (4 слова):
    {
        Param p;
        p.value = 123.456;
        p.startBit = 0;
        p.endBit = 63;
        p.startWord = 3;
        p.endWord = 6;           // 4 слова подряд: 3,4,5,6
        p.tp = ParamType::Float64;
        params.push_back(p);
    }

    // Вызываем сборку
    QVector<quint16> result = packParamsToWords(params);

    // Здесь result содержит все сформированные 16-битные слова.
    // В отладке мы уже увидели логи при формировании.
}