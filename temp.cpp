#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

// Для удобства храним результат вычислений в структуре
struct ParsedData {
    int startWord;
    int startBit;
    int endWord;
    int endBit;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Укажите путь к вашему csv-файлу
    QString csvFilePath = "data.csv";

    QFile file(csvFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Не удалось открыть файл:" << csvFilePath;
        return 1;
    }

    QTextStream in(&file);
    QList<ParsedData> parsedResults;  // список результатов

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        // Пропускаем пустые строки и, например, комментарии (если есть)
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // Разбиваем строку по запятой
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() < 3) {
            // Недостаточно данных в строке
            continue;
        }

        // Читаем SW, SB, LEN
        bool okSW = false, okSB = false, okLEN = false;
        int sw  = parts.at(0).toInt(&okSW);
        int sb  = parts.at(1).toInt(&okSB);
        int len = parts.at(2).toInt(&okLEN);

        if (!okSW || !okSB || !okLEN) {
            // Ошибка преобразования — пропускаем такую строку
            continue;
        }

        // --------------------------
        // Логика вычисления EndWord/EndBit
        // --------------------------
        int bitsPerWord = 16;

        // Всего бит от начала всего массива до стартовой позиции
        int totalStart = sw * bitsPerWord + sb;

        // Индекс последнего бита (в абсолютном счёте)
        int totalEnd   = totalStart + len - 1;

        // Конечные позиции
        int endBit     = totalEnd % bitsPerWord;
        int endWord    = totalEnd / bitsPerWord;

        ParsedData data;
        data.startWord = sw;
        data.startBit  = sb;
        data.endWord   = endWord;
        data.endBit    = endBit;

        parsedResults.append(data);
    }

    file.close();

    // Выведем полученный результат на консоль
    for (const ParsedData &pd : parsedResults) {
        qDebug() << "StartWord:" << pd.startWord
                 << "StartBit:" << pd.startBit
                 << "EndWord:" << pd.endWord
                 << "EndBit:" << pd.endBit;
    }

    return 0;
}