#include <cmath> // std::round

static qint64 encodeToTwosComplement(double val, int bitCount)
{
    // 1. Округляем (при необходимости до int).
    double rounded = std::round(val);
    qint64 intVal = static_cast<qint64>(rounded);

    // Безопасность:
    if (bitCount < 1)  bitCount = 1;
    if (bitCount > 63) bitCount = 63;

    // 2. Диапазон для signed two's complement (bitCount бит).
    //    min = -(1 << (bitCount - 1))
    //    max =  (1 << (bitCount - 1)) - 1
    qint64 minVal = -(1LL << (bitCount - 1));
    qint64 maxVal =  (1LL << (bitCount - 1)) - 1;

    if (intVal < minVal) intVal = minVal;
    if (intVal > maxVal) intVal = maxVal;

    // 3. Оставляем только младшие bitCount бит 
    //    (C++ хранит отрицательные числа в two's complement, но "лишние" старшие биты нужно обрезать).
    quint64 mask = (bitCount == 64) ? 0xFFFFFFFFFFFFFFFFULL
                                    : ((1ULL << bitCount) - 1ULL);
    quint64 raw = static_cast<quint64>(intVal) & mask;
    return static_cast<qint64>(raw);
}






static void setBitsRange(std::vector<quint16> &words,
                         int globalStartBit,
                         int bitCount,
                         qint64 rawValue)
{
    // Превращаем rawValue в беззнаковое, чтобы удобно брать биты
    quint64 uval = static_cast<quint64>(rawValue);

    for (int i = 0; i < bitCount; i++)
    {
        // i-й бит (0 = младший бит)
        quint64 bit = (uval >> i) & 1ULL;

        int targetBit = globalStartBit + i; // глобальный индекс бита
        int wordIndex = targetBit / 16;     // какой word
        int bitInWord = targetBit % 16;     // какой бит внутри слова

        quint16 mask = (1 << bitInWord);
        if (bit) {
            words[wordIndex] |= mask;   // установить бит
        } else {
            words[wordIndex] &= ~mask;  // сбросить бит
        }
    }
}





#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <vector>

struct Param
{
    int  startWord;
    int  endWord;
    int  startBit;
    int  endBit;
    double value;
};

static qint64 encodeToTwosComplement(double val, int bitCount);
static void setBitsRange(std::vector<quint16> &words,
                         int globalStartBit,
                         int bitCount,
                         qint64 rawValue);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 1) Пример: читаем/формируем список Param.
    //    На практике вы возьмёте их из CSV или БД.
    QList<Param> params;
    params.append({0, 0, 0,  7,  12.0 });  // занимает 8 бит (0..7), в слове 0
    params.append({0, 1, 8, 15, -10.5 }); // пример: startWord=0, endWord=1, startBit=8, endBit=15
                                         // значит занимает больше 16 бит (на самом деле 24)
    // и т.д.

    // 2) Выясняем, сколько всего слов нам может понадобиться
    //    (по максимальному endWord)
    int maxW = 0;
    for (auto &p : params) {
        if (p.endWord > maxW) {
            maxW = p.endWord;
        }
    }

    // 3) Создаём массив слов [0..maxW], 16 бит каждое
    std::vector<quint16> words(maxW + 1, 0);

    // 4) Для каждого параметра:
    for (auto &p : params)
    {
        int globalStart = p.startWord * 16 + p.startBit;
        int globalEnd   = p.endWord   * 16 + p.endBit;
        int bitCount    = globalEnd - globalStart + 1;

        // Кодируем значение (two's complement) в bitCount бит.
        qint64 rawVal = encodeToTwosComplement(p.value, bitCount);

        // Укладываем эти биты в words
        setBitsRange(words, globalStart, bitCount, rawVal);
    }

    // 5) Выводим каждое слово в hex.
    //    По условию: слово за словом, внутри слова — младший бит = bit 0.
    for (int w = 0; w <= maxW; w++) {
        QString hexStr = QString("%1").arg(words[w], 4, 16, QLatin1Char('0')).toUpper();
        qDebug().noquote() << QString("Word[%1] = 0x%2").arg(w).arg(hexStr);
    }

    return 0;
}

// реализация функций (приведены выше)



