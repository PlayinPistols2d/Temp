#include <QDebug>
#include <QVector>
#include <QString>

struct Param
{
    double value;      // Значение параметра
    int startBit;      // Начальный бит в слове
    int endBit;        // Конечный бит в слове
    int startWord;     // Начальное 16-битное слово
    int endWord;       // Конечное 16-битное слово
};

//-------------------------------------------
//  Вспомогательные функции для IEEE754
//-------------------------------------------
static quint64 doubleToIEEE754_64(double d)
{
    union {
        double d;
        quint64 u;
    } conv;
    conv.d = d;
    return conv.u;
}

static quint32 floatToIEEE754_32(float f)
{
    union {
        float f;
        quint32 u;
    } conv;
    conv.f = f;
    return conv.u;
}

/*!
 * \brief Получить биты IEEE754 (32 или 64), затем обрезать до totalBits (если нужно).
 *
 * Здесь, упрощённо:
 * - если totalBits <= 32, то берём float (32-бит) и обрезаем.
 * - если > 32, берём double (64-бит) и обрезаем до нужного количества бит.
 *
 * Для настоящего half-precision (16 бит) нужен отдельный алгоритм.
 */
static quint64 getIeee754Bits(double value, int totalBits)
{
    if (totalBits <= 32) {
        float f = static_cast<float>(value);
        quint64 raw32 = floatToIEEE754_32(f);
        if (totalBits < 32) {
            // Обнуляем старшие биты, оставляя только нужное количество
            quint64 mask = (1ULL << totalBits) - 1ULL;
            return raw32 & mask;
        }
        return raw32;  // Используем все 32 бита
    } else {
        quint64 raw64 = doubleToIEEE754_64(value);
        if (totalBits < 64) {
            quint64 mask = (1ULL << totalBits) - 1ULL;
            return raw64 & mask;
        }
        return raw64;  // Используем все 64 бита
    }
}

//-------------------------------------------
//  Основная функция
//-------------------------------------------
void fillWords(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего 16-битных слов понадобится
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // Создаём вектор для хранения 16-битных слов, инициализируем нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 2) Заполняем
    for (const auto &p : params)
    {
        int totalBits = p.endBit - p.startBit + 1;  // Сколько бит нужно

        // Получаем биты IEEE754 (или их подмножество)
        quint64 rawBits = getIeee754Bits(p.value, totalBits);

        // Проверяем, занимает ли параметр несколько слов
        if (p.startWord == p.endWord)
        {
            // --- (A) ПАРАМЕТР В ОДНОМ СЛОВЕ ---
            // Просто запишем нужные биты в words[startWord], c позиции startBit
            // по позицию endBit (включительно). Идём с младшего бита rawBits -> старший.
            int bitIndexInValue = 0;
            int currentBit = p.startBit;  // Внутри этого же слова

            while (bitIndexInValue < totalBits) {
                bool bitSet = ((rawBits >> bitIndexInValue) & 1ULL) != 0ULL;
                if (bitSet) {
                    words[p.startWord] |= (1 << currentBit);
                }
                ++bitIndexInValue;
                ++currentBit;
            }
        }
        else
        {
            // --- (B) ПАРАМЕТР НА НЕСКОЛЬКИХ СЛОВАХ ---
            // Раскладываем биты последовательно: сначала дополняем первый word (с startBit),
            // потом переходим к следующему word (с бита 0) и т.д.
            int bitIndexInValue = 0;
            int currentWord = p.startWord;
            int currentBit = p.startBit;

            while (bitIndexInValue < totalBits)
            {
                bool bitSet = ((rawBits >> bitIndexInValue) & 1ULL) != 0ULL;
                if (bitSet) {
                    words[currentWord] |= (1 << currentBit);
                }

                ++bitIndexInValue;
                ++currentBit;

                // Если дошли до конца 16-битного слова:
                if (currentBit > 15) {
                    currentBit = 0;
                    ++currentWord;
                }
            }
        }
    }

    // 3) Выводим результат
    for (int i = 0; i < words.size(); ++i) {
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'));
    }
}