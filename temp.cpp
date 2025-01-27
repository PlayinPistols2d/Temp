#include <QVector>
#include <QDebug>
#include <QString>
#include <cstdint>

static quint64 doubleToIEEE754_64(double d)
{
    // Union для «достать» 64-битное представление double
    union {
        double d;
        quint64 u;
    } conv;
    conv.d = d;
    return conv.u;
}

static quint32 floatToIEEE754_32(float f)
{
    // Union для «достать» 32-битное представление float
    union {
        float f;
        quint32 u;
    } conv;
    conv.f = f;
    return conv.u;
}

/*!
 * \brief Преобразовать double в биты IEEE754, учитывая желаемое кол-во бит (16, 32, 64…).
 *
 * Если totalBits <= 16 — (пример: half-precision) здесь нет готовой функции в стандартном C++,
 *   придётся реализовать вручную. Для примера — просто «обрежем» 32-битный float или 64-битный double,
 *   но корректной конверсии half-precision тут не будет.
 *
 * Если 16 < totalBits <= 32 — используем float (single precision, 32 бита).
 * Если totalBits > 32        — используем double (64 бита).
 *
 * Возвращаем результат в 64-битном числе \a rawBits; реально значимые — только младшие totalBits.
 */
static quint64 getIeee754Bits(double value, int totalBits)
{
    if (totalBits <= 16)
    {
        //
        // Условно: half-precision (не совсем корректная реализация),
        // но в качестве примера — берём float32 и обрезаем верхние 16 бит
        //
        float f = static_cast<float>(value);
        quint32 raw32 = floatToIEEE754_32(f);

        // Возьмём только младшие 16 бит (это грубое «обрезание» float до half)
        // В реальной задаче можно написать отдельную функцию, которая
        // по правилам half-precision всё упакует (экспонента, мантисса и т.д.)
        quint64 result = raw32 & 0xFFFFu;
        return result;
    }
    else if (totalBits <= 32)
    {
        float f = static_cast<float>(value);
        quint32 raw32 = floatToIEEE754_32(f);

        // Обрежем, если totalBits < 32
        if (totalBits < 32) {
            quint64 mask = (1ULL << totalBits) - 1ULL;
            return static_cast<quint64>(raw32) & mask;
        } else {
            return static_cast<quint64>(raw32);
        }
    }
    else
    {
        // totalBits > 32 => double (64 бита)
        quint64 raw64 = doubleToIEEE754_64(value);
        if (totalBits < 64) {
            quint64 mask = (1ULL << totalBits) - 1ULL;
            raw64 &= mask;
        }
        return raw64;
    }
}




void fillWords(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего 16-битных слов нам понадобится
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }
    QVector<quint16> words(maxWord + 1, 0);

    // 2) Заполняем слова для каждого параметра
    for (const auto &p : params)
    {
        int totalBits = p.endBit - p.startBit + 1;  // сколько бит занимает параметр

        // Получим «сырой» набор бит для этого параметра
        quint64 rawBits = 0;

        if (p.isIeee754)
        {
            // Интерпретируем value как float/double (IEEE 754)
            // и берём ровно totalBits из младших бит
            rawBits = getIeee754Bits(p.value, totalBits);
        }
        else
        {
            // Обычный «целочисленный» способ:
            // просто берём value как 64-битное целое и обрезаем сверху, если totalBits < 64
            quint64 tmp = static_cast<quint64>(p.value); 
            if (totalBits < 64) {
                quint64 mask = (1ULL << totalBits) - 1ULL;
                tmp &= mask;
            }
            rawBits = tmp;
        }

        // Теперь кладём эти (totalBits) бит в общий массив words
        // Логика: идём с младшего бита rawBits (LSB) к старшему
        int bitIndexInValue = 0;
        int currentWord = p.startWord;
        int currentBitInWord = p.startBit;

        while (bitIndexInValue < totalBits)
        {
            bool bitSet = ((rawBits >> bitIndexInValue) & 1ULL) != 0ULL;
            if (bitSet) {
                words[currentWord] |= (1 << currentBitInWord);
            }

            ++bitIndexInValue;
            ++currentBitInWord;

            // Если вышли за границы 16-битного слова, переходим к следующему слову
            if (currentBitInWord > 15) {
                currentBitInWord = 0;
                ++currentWord;
            }
        }
    }

    // 3) Отладочный вывод
    for (int i = 0; i < words.size(); ++i) {
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'));
    }
}

