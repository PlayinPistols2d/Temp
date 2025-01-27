#include <QDebug>
#include <QVector>
#include <QString>
#include <QStringList>
#include <cmath>

struct Param
{
    double value;      // Значение параметра
    int startBit;      // Начальный бит (0..15)
    int endBit;        // Конечный бит (0..15)
    int startWord;     // Начальное слово (номер 16-битного слова)
    int endWord;       // Конечное слово (номер 16-битного слова)
};

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
 * - Если totalBits == 1 — считаем, что это "побитовой" параметр (см. код ниже),
 *   и обойдёмся без IEEE754 в основной логике.
 * - Если totalBits <= 32, интерпретируем как float (single precision, 32 бита).
 * - Если totalBits > 32, берём double (64 бита).
 *
 * Из полученного "сыра" (32/64 бита) берём только младшие totalBits (маской).
 */
static quint64 getIeee754Bits(double value, int totalBits)
{
    // Если (по логике) не отсекаем 1-битные параметры здесь, а обрабатываем в основном коде
    // — пропускаем этот случай. Иначе можно тут же проверять, if (totalBits == 1) ...
    if (totalBits <= 32)
    {
        float f = static_cast<float>(value);
        quint64 raw32 = floatToIEEE754_32(f);
        quint64 mask = (1ULL << totalBits) - 1ULL;
        return (raw32 & mask);
    }
    else
    {
        quint64 raw64 = doubleToIEEE754_64(value);
        if (totalBits < 64)
        {
            quint64 mask = (1ULL << totalBits) - 1ULL;
            raw64 &= mask;
        }
        return raw64;
    }
}

/*!
 * \brief Заполняет 16-битные слова на основе параметров, записывая результат в QStringList.
 *
 * Если параметр занимает ровно 1 бит (startBit == endBit), считаем, что это "побитовый" режим
 * и не используем IEEE754-представление, а просто ставим 1 или 0 (в зависимости от value).
 *
 * Если параметр занимает 2 и более бит, тогда:
 *   - Берём IEEE754-представление (32 или 64 бита),
 *   - Отрезаем лишние старшие биты (если totalBits < 32/64),
 *   - Раскладываем младшие биты по слову(ам).
 *
 * Результирующие слова (по количеству maxWord+1) складываются в 'outputWords'
 * как строки вида "0x0000" и т.д.
 */
void fillWords(const QVector<Param> &params, QStringList &outputWords)
{
    // 1) Вычисляем, сколько нужно 16-битных слов
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // 2) Создаём вектор слов (16-бит), инициализируем нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 3) Заполняем слова
    for (const auto &p : params)
    {
        int totalBits = p.endBit - p.startBit + 1;

        // Проверка: если ровно 1 бит, значит "побитовой" режим
        if (totalBits == 1)
        {
            // Ставим бит, если value != 0
            if (p.value != 0.0)
                words[p.startWord] |= (1 << p.startBit);
        }
        else
        {
            // Берём биты IEEE754 (32 или 64), обрезаем лишние
            quint64 rawBits = getIeee754Bits(p.value, totalBits);

            // Раскладываем
            int bitIndexInValue = 0;
            int currentWord = p.startWord;
            int currentBitInWord = p.startBit;

            while (bitIndexInValue < totalBits)
            {
                bool bitSet = ((rawBits >> bitIndexInValue) & 1ULL) != 0ULL;
                if (bitSet)
                    words[currentWord] |= (1 << currentBitInWord);

                ++bitIndexInValue;
                ++currentBitInWord;

                // Следующее 16-битное слово?
                if (currentBitInWord > 15)
                {
                    currentBitInWord = 0;
                    ++currentWord;
                }
            }
        }
    }

    // 4) Формируем строки вида "0x0000" для каждого слова и кладём в outputWords
    outputWords.clear();
    for (int i = 0; i < words.size(); ++i)
    {
        // Можно в верхнем регистре: .toUpper()
        QString hexStr = QString("0x%1").arg(words[i], 4, 16, QChar('0')).toUpper();
        outputWords.append(hexStr);
    }
}