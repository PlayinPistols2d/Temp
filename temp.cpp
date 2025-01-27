#include <QDebug>
#include <QVector>
#include <QtEndian>  // на случай, если захотите пользоваться qToLittleEndian и т.п.
#include <cmath>     // может понадобиться при округлениях

struct Param
{
    double value;   // Числовое значение параметра
    int startBit;   // Номер начального бита (от 0)
    int endBit;     // Номер конечного бита (включительно)
    int startWord;  // Индекс начального 16-битного слова
    int endWord;    // Индекс конечного 16-битного слова
};

/// Утилита, извлекающая биты double как 64-битное целое
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

/// Утилита, извлекающая биты float как 32-битное целое
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

/// Функция, которая «рассыпает» (set) биты value в общий буфер words[ wordIndex ].
/// - Гарантированно кладём в промежуток [startBit, endBit] (в глобальной нумерации бит).
/// - При этом "младший бит value" уходит в startBit, следующий бит – в startBit+1, ...
static void setBitsInBuffer(QVector<quint16> &words,
                            int startBit,
                            int endBit,
                            quint64 value)
{
    // Сколько всего бит нужно записать
    int sizeInBits = endBit - startBit + 1;

    // Бежим по битам value с нуля до (sizeInBits-1).
    // bitIndex == 0 -> младший бит value
    // bitIndex == 1 -> следующий бит value
    for (int bitIndex = 0; bitIndex < sizeInBits; ++bitIndex) {
        // Берём соответствующий бит из value
        quint64 bitVal = (value >> bitIndex) & 0x1;

        // Глобальный номер бита (все слова подряд)
        int globalBit = startBit + bitIndex;
        // Определяем, в каком слове мы находимся
        int wordIndex = globalBit / 16;
        // Определяем, какой бит внутри этого слова
        int bitInWord = globalBit % 16;

        // Устанавливаем бит bitInWord в words[wordIndex]
        if (bitVal == 1) {
            words[wordIndex] |= (1 << bitInWord);
        }
        // если bitVal == 0, то ничего делать не надо, там уже 0
    }
}

/// Основная функция, которая формирует массив 16-битных слов
/// и печатает их в debug в шестнадцатеричном формате.
QVector<quint16> buildWordsFromParams(const QVector<Param> &params)
{
    // 1) Определяем максимальное слово, которое нам может понадобиться
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord)
            maxWord = p.endWord;
    }

    // 2) Создаём вектор нужного размера, инициализированный нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 3) Обрабатываем каждый параметр
    for (const auto &param : params) {
        int sizeInBits = param.endBit - param.startBit + 1;
        // Считаем, сколько 16-битных слов реально занимает параметр
        int numWords = param.endWord - param.startWord + 1;

        // Подготовим «сырые» биты для записи
        // ----------------------------------
        // Ситуация 1: параметр умещается в одно 16-битное слово
        // => берём value как целое (при необходимости округляем/кастим).
        if (numWords == 1) {
            // Просто берём целочисленное представление (обрезаем/транкируем)
            // Можно округлять, если нужно.
            quint64 raw = static_cast<quint64>(param.value);
            // Но нам нужно только sizeInBits (могут быть 8, 10, 15 и т.п.).
            // Поэтому давайте обрежем лишние старшие биты.
            // Чтобы не записать случайно больше, чем нужно.
            quint64 mask = (sizeInBits >= 64)
                           ? 0xFFFFFFFFFFFFFFFFULL
                           : ((1ULL << sizeInBits) - 1ULL);
            raw &= mask;

            // Теперь «рассыпаем» raw по bits [startBit..endBit]
            setBitsInBuffer(words, 
                            param.startBit + param.startWord * 16,  // глобальный бит начала
                            param.endBit   + param.startWord * 16,  // глобальный бит конца
                            raw);
        }
        else {
            // Ситуация 2: параметр занимает более одного слова => работаем с IEEE754
            // Предположим, что если sizeInBits <= 32, то это float,
            // если 33..64 — то double. (Можно дорабатывать логику по нуждам.)
            quint64 raw = 0;

            if (sizeInBits <= 32) {
                // Трактуем param.value как float
                float f = static_cast<float>(param.value);
                quint32 bits32 = floatToRawBits(f);
                // Если, например, у нас ровно 24 бита, то просто отбросим старшие 8 бит
                quint64 mask = (sizeInBits == 32)
                               ? 0xFFFFFFFFULL
                               : ((1ULL << sizeInBits) - 1ULL);
                raw = bits32 & mask;
            }
            else {
                // Трактуем param.value как double (64 бита)
                quint64 bits64 = doubleToRawBits(param.value);
                // Если реально нужно меньше 64 бит, то обрежем
                if (sizeInBits < 64) {
                    quint64 mask = (1ULL << sizeInBits) - 1ULL;
                    bits64 &= mask;
                }
                raw = bits64;
            }

            // Записываем raw в диапазон [startBit..endBit], учитывая,
            // что начало = param.startBit + param.startWord*16,
            // а конец   = param.endBit   + param.startWord*16 (или endWord*16).
            // На практике удобно именно «сдвигать» глобальные биты для слова startWord:
            int globalStart = param.startWord * 16 + param.startBit;
            int globalEnd   = param.endWord   * 16 + param.endBit;
            setBitsInBuffer(words, globalStart, globalEnd, raw);
        }
    }

    // 4) Выводим результат (каждое слово в hex)
    // Обратите внимание, что слово 0 - это младшие 16 бит, слово 1 - следующие 16 бит и т.д.
    for (int i = 0; i < words.size(); ++i) {
        // Печатаем, скажем, в виде "Word i: 0xXXXX"
        // с ведущими нулями до 4 символов (16-бит => 4 hex цифры)
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    // arg(..., fieldWidth=4, base=16, fillChar='0')
                    .arg(words[i], 4, 16, QChar('0')).toUpper();
    }

    return words;
}