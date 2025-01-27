#include <QDebug>
#include <QVector>
#include <QString>
#include <cmath>      // для std::isnan, std::isinf и т.д. при желании

struct Param
{
    double value;      // Значение параметра
    int startBit;      // Начальный бит (0..15)
    int endBit;        // Конечный бит (0..15), >= startBit
    int startWord;     // Начальное слово (номер 16-битного слова)
    int endWord;       // Конечное слово (номер 16-битного слова)
};

//------------------------------------------------------------------------------
//  Функции для получения "сырых" битов IEEE754 (как раньше)
//------------------------------------------------------------------------------
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
 * \brief Преобразовать значение double в биты IEEE754 на столько бит, сколько нужно.
 *
 * - Если totalBits <= 32, интерпретируем как float (single precision, 32 бита).
 *   Берём из полученных 32 только младшие totalBits.
 * - Если totalBits > 32, берём double (64 бита) и,
 *   если totalBits < 64, отрезаем верхние (64 - totalBits) биты.
 */
static quint64 getIeee754Bits(double value, int totalBits)
{
    if (totalBits <= 32)
    {
        // Используем float (32 бита)
        float f = static_cast<float>(value);
        quint64 raw32 = floatToIEEE754_32(f);

        quint64 mask = (1ULL << totalBits) - 1ULL;  
        return (raw32 & mask);
    }
    else
    {
        // Используем double (64 бита)
        quint64 raw64 = doubleToIEEE754_64(value);

        if (totalBits < 64)
        {
            quint64 mask = (1ULL << totalBits) - 1ULL;
            raw64 &= mask;
        }
        return raw64;
    }
}

//------------------------------------------------------------------------------
//  Основная функция
//------------------------------------------------------------------------------
void fillWords(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего 16-битных слов нужно
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // Создаём вектор для слов (каждое - 16 бит), инициализируем нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 2) Заполняем слова для каждого параметра
    for (const auto &p : params)
    {
        int totalBits = p.endBit - p.startBit + 1;  // длина в битах

        // --- Проверяем: это "побитовый" параметр или нет? ---
        if (totalBits == 1)
        {
            // --- Кейс 1: Один бит ---
            // Здесь НЕ делаем IEEE754, просто устанавливаем бит, если value != 0
            if (p.value != 0.0)
            {
                // Ставим этот 1 бит
                words[p.startWord] |= (1 << p.startBit);
            }
        }
        else
        {
            // --- Кейс 2: Более одного бита ---
            // Идём по прежней схеме: берём сырые биты IEEE754
            quint64 rawBits = getIeee754Bits(p.value, totalBits);

            // Раскладываем их, начиная с младшего бита
            int bitIndexInValue = 0;
            int currentWord = p.startWord;
            int currentBitInWord = p.startBit;

            while (bitIndexInValue < totalBits)
            {
                bool bitSet = ((rawBits >> bitIndexInValue) & 1ULL) != 0ULL;
                if (bitSet)
                {
                    words[currentWord] |= (1 << currentBitInWord);
                }

                ++bitIndexInValue;
                ++currentBitInWord;

                if (currentBitInWord > 15)
                {
                    currentBitInWord = 0;
                    ++currentWord;
                }
            }
        }
    }

    // 3) Выводим результат (каждое слово в hex-формате)
    for (int i = 0; i < words.size(); ++i)
    {
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'));
    }
}