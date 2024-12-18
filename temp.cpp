#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QDebug>

// Функция для извлечения числового диапазона из строки
std::pair<int, int> extractRange(const QString& str) {
    QRegularExpression regex("\\[(\\d+)-(\\d+)\\]");
    QRegularExpressionMatch match = regex.match(str);

    if (match.hasMatch()) {
        int x = match.captured(1).toInt();
        int y = match.captured(2).toInt();
        return {x, y};
    }
    return {0, 0}; // Возвращаем {0, 0}, если диапазон не найден
}

// Сравнительная функция для сортировки
bool compareStrings(const QString& a, const QString& b) {
    auto rangeA = extractRange(a);
    auto rangeB = extractRange(b);

    if (rangeA.first != rangeB.first)
        return rangeA.first < rangeB.first; // Сравниваем по x
    return rangeA.second < rangeB.second;  // Если x равны, сравниваем по y
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Пример списка строк
    QStringList list = {
        "example[5-10]",
        "test[1-3]",
        "sample[2-4]",
        "demo[0-1]",
        "data[3-8]"
    };

    // Сортировка списка
    std::sort(list.begin(), list.end(), compareStrings);

    // Вывод отсортированного списка
    for (const QString& str : list) {
        qDebug() << str;
    }

    return a.exec();
}