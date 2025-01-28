#include <QDebug>
#include <QVector>
#include <QtGlobal>  // для quint16, quint64 и т.п.
#include <cmath>     // или <cstring>

struct Param
{
    double value;    // Значение, которое надо поместить
    int startBit;    // Начальный бит (в рамках общего 64-битного поля) — LSB = 0
    int endBit;      // Конечный бит (тоже в LSB=0 ... MSB=63 системе)
    int startWord;   // Номер начального слова (16-бит)
    int endWord;     // Номер конечного слова (16-бит)
};

// Функция для помещения (bitCount) младших бит из 'valueBits' в буфер слов 'words',
// начиная с бита 'bitOffset' в слове 'wordIndex'. При «переполнении» переходим к
// следующему слову (wordIndex+1) и начинаем с бита 0 там.
static void placeBitsIntoWords(quint64 valueBits,
                               int bitCount,
                               int wordIndex,
                               int bitOffset,
                               QVector<quint16>& words)
{
    int bitsPlaced = 0;
    while (bitsPlaced < bitCount)
    {
        // Сколько бит можем записать в текущее слово, начиная с bitOffset:
        int capacity = 16 - bitOffset;
        int bitsToWrite = qMin(capacity, bitCount - bitsPlaced);

        // Маска на bitsToWrite младших бит
        quint64 mask = (1ULL << bitsToWrite) - 1;

        // Берём младшие bitsToWrite бит из valueBits
        quint64 portion = valueBits & mask;
        // Сразу «обрезаем» эти биты из valueBits (сдвигаем вправо):
        valueBits >>= bitsToWrite;

        // Записываем portion в текущее слово со сдвигом bitOffset
        words[wordIndex] |= static_cast<quint16>(portion << bitOffset);

        // Считаем, что часть бит уже поместили
        bitsPlaced += bitsToWrite;

        // Переходим к следующему слову
        ++wordIndex;
        // Там начинаем с нулевого бита
        bitOffset = 0;
    }
}

void fillWords(const QVector<Param>& params)
{
    // 1) Определим, сколько всего 16-битных слов нужно.
    //    Ищем maxEndWord = максимум по endWord во входных параметрах.
    int maxEndWord = 0;
    for (const Param& p : params) {
        if (p.endWord > maxEndWord) {
            maxEndWord = p.endWord;
        }
    }
    // Создаём вектор, который гарантированно вместит все слова, и обнулим их.
    QVector<quint16> words(maxEndWord + 1, 0);

    // 2) Заполняем words параметрами
    for (const Param& p : params)
    {
        // Сколько бит хотим взять вообще
        int bitCount = p.endBit - p.startBit + 1;
        if (bitCount <= 0) {
            qWarning() << "Некорректные startBit/endBit у параметра, пропускаем.";
            continue;
        }

        // Преобразуем double -> 64-бит по IEEE754
        union {
            double d;
            quint64 u;
        } converter;
        converter.d = p.value;
        quint64 allBits = converter.u;  // Здесь лежат 64 бита IEEE754

        // Вырежем из allBits нужный диапазон [startBit..endBit]
        // Считаем, что startBit=0 - это младший бит (LSB).
        // Сдвигаем вправо на startBit, чтобы нужный диапазон оказался в младших битах:
        quint64 paramBits = allBits >> p.startBit;

        // Теперь «откусываем» всё, что за пределами endBit.
        // Для bitCount берём маску (1 << bitCount) - 1:
        quint64 mask = (1ULL << bitCount) - 1;
        paramBits &= mask;

        // Вычислим, сколько бит мы впихиваем в первое слово
        // в смысле, с какого именно бита (offset внутри слова) начинаем.
        // Так как по условию задачи bit 0..15 в рамках слова – это его 16 бит.
        // Пример: если startBit=8, то внутри startWord начинаем с бита 8.
        // Но важно понимать, что "startBit=8" в самом параметре — это не всегда
        // "бит 8 внутри этого слова". Однако в примерах сказано, что если
        // startWord = endWord (т.е. параметр весь внутри одного слова),
        // то bits [startBit..endBit] как раз и будут позиционироваться внутри слова.
        //
        // Поэтому здесь предполагается, что если пользователь указал startBit=8,
        // а также startWord=6, endWord=6, значит эти биты действительно лежат
        // внутри 16-битного слова номер 6, начиная с бита 8 по бит 15 (или какой endBit).
        //
        // То есть bitOffset = (startBit % 16), если мы рассматриваем смещение в слове startWord.
        // Однако, если параметр простирается на несколько слов, то формально
        // "битовое" поле идёт сплошняком: слова идут подряд, в каждом 16 бит.
        // Соответственно, общий "номер бита" = 16*(startWord) + startBit (внутри слова).
        // Но в задаче примеры даны так, что startBit уже «привязан» к конкретному слову.
        // Поэтому обычно достаточно сделать:
        int startBitOffsetInWord = p.startBit % 16;

        // Теперь аккуратно раскладываем эти биты paramBits в слова [startWord..endWord]
        placeBitsIntoWords(paramBits,
                           bitCount,
                           p.startWord,
                           startBitOffsetInWord,
                           words);
    }

    // 3) Выводим полученные слова в шестнадцатеричном формате
    for (int i = 0; i < words.size(); ++i)
    {
        // Печатаем слово в hex, дополняя ведущими нулями до 4 символов
        // Пример формата: "Word 0: 0xABCD"
        qDebug() << QString("Word %1: 0x%2")
                        .arg(i)
                        .arg(words[i], 4, 16, QLatin1Char('0')).toUpper();
    }
}