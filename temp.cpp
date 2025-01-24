#include <QDebug>
#include <QVector>
#include <QString>

struct Param
{
    double value;      // Значение параметра
    int startBit;      // Начальный бит
    int endBit;        // Конечный бит
    int startWord;     // Начальное слово
    int endWord;       // Конечное слово
};

void fillWords(const QVector<Param> &params)
{
    // 1) Определяем, сколько всего слов понадобится
    int maxWord = 0;
    for (const auto &p : params) {
        if (p.endWord > maxWord) {
            maxWord = p.endWord;
        }
    }

    // Создаём вектор для хранения слов (16-бит каждый), инициализируем нулями
    QVector<quint16> words(maxWord + 1, 0);

    // 2) Заполняем слова побитово для каждого параметра
    for (const auto &p : params) {
        // Преобразуем double -> 64-битное целое (можно скорректировать логику,
        // если необходимо не просто "обрезание" double, а, например, округление или reinterpret_cast битов)
        quint64 rawValue = static_cast<quint64>(p.value);

        // Сколько бит надо перенести?
        int totalBits = p.endBit - p.startBit + 1;

        // Позиция по битам внутри rawValue
        int bitIndexInValue = 0;

        // Текущее слово и бит, куда пишем
        int currentWord = p.startWord;
        int currentBitInWord = p.startBit;

        // Переносим все нужные биты
        while (bitIndexInValue < totalBits) {
            // Проверяем, установлена ли младшая часть этого бита в rawValue
            bool bitSet = ((rawValue >> bitIndexInValue) & 1ULL) != 0;

            if (bitSet) {
                // Устанавливаем соответствующий бит в words[currentWord]
                words[currentWord] |= (1 << currentBitInWord);
            }

            ++bitIndexInValue;
            ++currentBitInWord;

            // Если дошли до конца 16-битного слова, переходим на следующее слово
            if (currentBitInWord > 15) {
                currentBitInWord = 0;
                ++currentWord;
            }
        }
    }

    // 3) Выводим результат для каждого слова в hex-формате
    for (int i = 0; i < words.size(); ++i) {
        // Формируем строку вида: "Word 0: 0xABCD"
        // Аргумент '%1' -> номер слова
        // Аргумент '%2' -> значение слова в hex, шириной 4 символа с ведущими нулями
        qDebug() << QString("Word %1: 0x%2")
                    .arg(i)
                    .arg(words[i], 4, 16, QChar('0'));
    }
}