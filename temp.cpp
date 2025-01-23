#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QList>
#include <QDebug>

// Структура для хранения результатов
struct ParsedData {
    int StartBit;
    int EndBit;
    int StartWord;
    int EndWord;
};

// Функция для парсинга CSV и вычисления конечных значений
QList<ParsedData> parseCSV(const QString &filePath) {
    QList<ParsedData> results;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return results;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(",");
        if (fields.size() < 3) {
            qWarning() << "Некорректная строка:" << line;
            continue;
        }

        // Чтение полей из строки
        int SW = fields[0].toInt(); // Номер слова
        int SB = fields[1].toInt(); // Номер разряда
        int LEN = fields[2].toInt(); // Длина бит

        // Вычисление конечных бит и слова
        int startWord = SW;
        int startBit = SB;
        int endBit = SB + LEN - 1;
        int endWord = SW + endBit / 16; // 16 бит в слове
        endBit %= 16; // Корректировка конечного бита

        // Запись результата
        results.append({startBit, endBit, startWord, endWord});
    }

    file.close();
    return results;
}

// Пример использования
int main(int argc, char *argv[]) {
    QString csvPath = "path/to/your/file.csv";
    QList<ParsedData> parsedData = parseCSV(csvPath);

    for (const auto &data : parsedData) {
        qDebug() << "StartBit:" << data.StartBit
                 << "EndBit:" << data.EndBit
                 << "StartWord:" << data.StartWord
                 << "EndWord:" << data.EndWord;
    }

    return 0;
}