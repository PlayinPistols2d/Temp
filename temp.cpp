#include <QCoreApplication>
#include <QList>
#include <QVector>
#include <QDebug>
#include <algorithm> // std::reverse

// Структура описания одного параметра
struct Param
{
    double value;   // значение (всегда double, но логика ниже распознаёт int/float/double)
    int startBit;   // начальный бит
    int endBit;     // конечный бит
    int startWord;  // начальное 16-битное слово
    int endWord;    // конечное 16-битное слово
    QString tp;     // "int", "uint", "float", "double"
};

//-------------------------
// Вспомогательные функции
//-------------------------

// Преобразование float -> 4 байта (Little-Endian по байтам)
QByteArray floatToBytes(float f)
{
    union {
        float f;
        quint32 u32;
    } conv;
    conv.f = f;

    QByteArray arr(4, 0);
    // Младший байт в arr[0], старший в arr[3]
    for (int i = 0; i < 4; i++) {
        arr[i] = static_cast<char>((conv.u32 >> (8 * i)) & 0xFF);
    }
    return arr;
}

// Преобразование double -> 8 байт (Little-Endian по байтам)
QByteArray doubleToBytes(double d)
{
    union {
        double d;
        quint64 u64;
    } conv;
    conv.d = d;

    QByteArray arr(8, 0);
    for (int i = 0; i < 8; i++) {
        arr[i] = static_cast<char>((conv.u64 >> (8 * i)) & 0xFF);
    }
    return arr;
}

// Преобразование массива байтов (кратно 2) в массив 16-битных слов
QVector<quint16> bytesToWords(const QByteArray &bytes)
{
    QVector<quint16> words;
    int count = bytes.size()/2;
    words.resize(count);

    for (int i = 0; i < count; i++) {
        quint8 low  = static_cast<quint8>(bytes[2*i]);
        quint8 high = static_cast<quint8>(bytes[2*i + 1]);
        quint16 w   = static_cast<quint16>((high << 8) | low);
        words[i] = w;
    }
    return words;
}

// Разворачиваем порядок слов: w0,w1,w2,w3 -> w3,w2,w1,w0
void reverseWords(QVector<quint16> &words)
{
    std::reverse(words.begin(), words.end());
}

//------------------------------
// Вставка (до 16) бит в одно 16-битное слово
//------------------------------
void insertBitsInWord(QVector<quint16> &words,
                      int wordIndex,
                      int startBit,
                      int bitCount,
                      quint64 valBits)
{
    // Маска на bitCount
    quint64 mask = (1ULL << bitCount) - 1ULL;
    // Берём младшие bitCount бит
    quint16 part = static_cast<quint16>(valBits & mask);
    // Сдвигаем в нужную позицию в слове
    part <<= startBit;

    // Формируем "очиститель" соответствующих бит
    quint16 clearMask = static_cast<quint16>( ~(mask << startBit) );

    // Накладываем
    words[wordIndex] = (words[wordIndex] & clearMask) | part;
}

//------------------------------
// Основная функция упаковки
//------------------------------
QVector<quint16> packParameters(const QList<Param> &params)
{
    // 1) Вычислим, сколько слов нужно (максимальный endWord)
    int maxWordIndex = 0;
    for (auto &p : params) {
        if (p.endWord > maxWordIndex) {
            maxWordIndex = p.endWord;
        }
    }

    // 2) Создаём буфер слов (16-бит), инициализируем нулями
    QVector<quint16> words(maxWordIndex + 1, 0);

    // 3) Перебираем параметры
    for (const auto &param : params)
    {
        int bitCount = param.endBit - param.startBit + 1;
        if (bitCount <= 0) {
            // некорректный параметр, пропускаем
            continue;
        }

        //--------------------------------------
        // 3a) Преобразуем param.value в набор 16-битных слов (paramWords)
        //--------------------------------------
        QVector<quint16> paramWords;

        if (param.tp == "float")
        {
            float fVal = static_cast<float>(param.value);
            QByteArray arr = floatToBytes(fVal); // 4 байта
            paramWords = bytesToWords(arr);      // => 2 слова
            // word-level swap
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else if (param.tp == "double")
        {
            double dVal = param.value;
            QByteArray arr = doubleToBytes(dVal); // 8 байт
            paramWords = bytesToWords(arr);       // => 4 слова
            // word-level swap
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else if (param.tp == "int")
        {
            qint64 ival = static_cast<qint64>(param.value);
            // Сформируем bitCount младших бит
            quint64 mask = (bitCount >= 64) 
                           ? 0xFFFFFFFFFFFFFFFFULL
                           : ((1ULL << bitCount) - 1ULL);
            quint64 raw = static_cast<quint64>(ival) & mask;

            // Сколько байт нам нужно?
            int bytesNeeded = (bitCount + 7)/8; // округление вверх
            QByteArray arr(bytesNeeded, 0);
            // Раскладываем raw (LE по байтам)
            for (int i = 0; i < bytesNeeded; i++) {
                arr[i] = static_cast<char>((raw >> (8*i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);

            // Если занимает более 16 бит, то обычно делаем reverseWords
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else if (param.tp == "uint")
        {
            quint64 uval = static_cast<quint64>(param.value);
            quint64 mask = (bitCount >= 64)
                           ? 0xFFFFFFFFFFFFFFFFULL
                           : ((1ULL << bitCount) - 1ULL);
            quint64 raw = uval & mask;

            int bytesNeeded = (bitCount + 7)/8;
            QByteArray arr(bytesNeeded, 0);
            for (int i = 0; i < bytesNeeded; i++) {
                arr[i] = static_cast<char>((raw >> (8*i)) & 0xFF);
            }
            paramWords = bytesToWords(arr);

            // Аналогично - если много слов, реверсим
            if (paramWords.size() > 1) {
                reverseWords(paramWords);
            }
        }
        else {
            // неизвестный тип, пропускаем
            continue;
        }

        //--------------------------------------
        // 3b) "Склеим" paramWords в одно 64-битное число allBits,
        //     чтобы побитово раскидать.
        //     (Если bitCount может быть >64, нужна более сложная логика).
        //--------------------------------------
        quint64 allBits = 0;
        for (int i = 0; i < paramWords.size(); i++) {
            quint64 w = static_cast<quint64>(paramWords[i]);
            // paramWords[0] - младшие 16 бит, paramWords[1] - следующие 16 и т.д.
            allBits |= (w << (16 * i));
        }

        //--------------------------------------
        // 3c) Побитово вставляем allBits в words[startWord..endWord]
        //     с учётом startBit..endBit.
        //--------------------------------------
        int bitsLeft      = bitCount;
        int currentBitPos = 0; // сколько бит уже взяли из allBits
        int wIndex        = param.startWord;
        int bitPosInWord  = param.startBit;

        while (bitsLeft > 0 && wIndex <= param.endWord)
        {
            int freeBitsInThisWord = 16 - bitPosInWord;
            int putNow = qMin(freeBitsInThisWord, bitsLeft);

            // выдёргиваем putNow бит из allBits, начиная с currentBitPos
            quint64 mask = (1ULL << putNow) - 1ULL;
            quint64 part = (allBits >> currentBitPos) & mask;

            // вставляем (putNow бит) в слово words[wIndex] c позиции bitPosInWord
            insertBitsInWord(words, wIndex, bitPosInWord, putNow, part);

            bitsLeft      -= putNow;
            currentBitPos += putNow;
            bitPosInWord   = 0; // в следующем слове начинаем с бита 0
            wIndex++;
        }
    }

    // 4) Отладочный вывод
    for (int i = 0; i < words.size(); i++) {
        QString hexStr = QString("Word[%1] = 0x%2")
                .arg(i)
                .arg(words[i], 4, 16, QChar('0'))
                .toUpper();
        qDebug() << hexStr;
    }

    // 5) Возвращаем готовый буфер
    return words;
}

//------------------------------
// Пример использования
//------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QList<Param> parameters;

    // Пример 1: Один 32-битный int (TP="int"), лежит в 2 словах [word=0..1]
    {
        Param p;
        p.value     = 123456.0; // как int
        p.startBit  = 0;
        p.endBit    = 31;       // 32 бита
        p.startWord = 0;
        p.endWord   = 1;        // занимает слова 0 и 1
        p.tp        = "int";
        parameters.append(p);
    }

    // Пример 2: 3 мелких параметра в одном слове (word=2):
    //    * 8 бит [0..7], value=0xAA (170)
    //    * 7 бит [8..14], value=0x55 (85)
    //    * 1 бит [15..15], value=1
    {
        Param p1;
        p1.value     = 170.0;     // uint
        p1.startBit  = 0;
        p1.endBit    = 7;         // 8 бит
        p1.startWord = 2;
        p1.endWord   = 2;
        p1.tp        = "uint";
        parameters.append(p1);

        Param p2;
        p2.value     = 85.0;      // 0x55
        p2.startBit  = 8;
        p2.endBit    = 14;        // 7 бит
        p2.startWord = 2;
        p2.endWord   = 2;
        p2.tp        = "uint";
        parameters.append(p2);

        Param p3;
        p3.value     = 1.0;       // 1 бит
        p3.startBit  = 15;
        p3.endBit    = 15;
        p3.startWord = 2;
        p3.endWord   = 2;
        p3.tp        = "uint";
        parameters.append(p3);
    }

    // Пример 3: Один float (32 бита), лежит в словах [3..4]
    {
        Param p;
        p.value     = 3.14; // float
        p.startBit  = 0;
        p.endBit    = 31;
        p.startWord = 3;
        p.endWord   = 4;   // займёт 2 слова
        p.tp        = "float";
        parameters.append(p);
    }

    // Пример 4: Один double (64 бита), лежит в словах [5..8]
    {
        Param p;
        p.value     = 1.234567; // double
        p.startBit  = 0;
        p.endBit    = 63;
        p.startWord = 5;
        p.endWord   = 8;  // займёт 4 слова
        p.tp        = "double";
        parameters.append(p);
    }

    // Теперь упакуем
    QVector<quint16> result = packParameters(parameters);

    return 0; // конец
}