#include <QDebug>
#include <QVector>
#include <QString>
#include <cmath>      // для std::isnan, std::isinf и т.д. при желании

struct Param
{
    double value;      // Значение параметра
    int startBit;      // Начальный бит
    int endBit;        // Конечный бит
    int startWord;     // Начальное слово (16-бит)
    int endWord;       // Конечное слово (16-бит)
};

static quint64 doubleToIEEE754_64(double d)
{
    // Используем union, чтобы безопасно «достать» биты double (64 бит)
    union {
        double d;
        quint64 u;
    } conv;
    conv.d = d;
    return conv.u;
}

static quint32 floatToIEEE754_32(float f)
{
    // Аналогично, но для 32 бит
    union {
        float f;
        quint32 u;
    } conv;
    conv.f = f;
    return conv.u;
}

/*!
 * \brief Преобразовать значение в «сырые» биты IEEE754, учитывая кол-во бит (16, 32, 64 и т.д.).
 *
 * Если totalBits <= 32 — используем float (single precision) и берём младшие totalBits
 * Если totalBits > 32  — используем double (64-bit) и берём младшие totalBits
 *
 * \param value — число double
 * \param totalBits — сколько бит нужно
 * \return 64-битное число, в младших totalBits которого лежат нужные биты IEEE754
 */
static quint64 getIeee754Bits(double value, int totalBits)
{
    if (totalBits <= 32)
    {
        // Преобразуем в float
        float f = static_cast<float>(value);
        quint64 raw32 = floatToIEEE754_32(f);

        // Возвращаем как 64-битное, но значимые только младшие 32 бита
        quint64 result = static_cast<quint64>(raw32) & ((1ULL << totalBits) - 1ULL);
        return result;
    }
    else
    {
        // Преобразуем в double (64-бит)
        quint64 raw64 = doubleToIEEE754_64(value);

        // Если totalBits < 64, обрежем верхние
        if (totalBits < 64)
            raw64 &= ((1ULL << totalBits) - 1ULL);

        return raw64;
    }
}

void fillWords(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего слов (16-бит) надо
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }
    // Массив слов (каждое - 16 бит)
    QVector<quint16> words(maxWord + 1, 0);

    // 2) Заполнение
    for (const auto &p : params)
    {
        int totalBits = p.endBit - p.startBit + 1;  // сколько бит занимает параметр
        // Получаем биты IEEE754 для value
        quint64 rawBits = getIeee754Bits(p.value, totalBits);

        // Раскладываем по словам, начиная со startWord, с бита startBit
        // Идём с младшего бита (bitIndexInValue=0) к старшему (bitIndexInValue=totalBits-1)
        int bitIndexInValue = 0;
        int currentWord = p.startWord;
        int currentBitInWord = p.startBit;

        while (bitIndexInValue < totalBits)
        {
            bool bitSet = ( (rawBits >> bitIndexInValue) & 1ULL ) != 0ULL;
            if (bitSet)
            {
                words[currentWord] |= (1 << currentBitInWord);
            }

            ++bitIndexInValue;
            ++currentBitInWord;

            // Дошли до конца 16-битного слова?
            if (currentBitInWord > 15) {
                currentBitInWord = 0;
                ++currentWord;
            }
        }
    }

    // 3) Выводим результат
    for (int i = 0; i < words.size(); ++i)
    {
        // Вывод в стиле: "Word 0: 0x0000"
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'));
    }
}