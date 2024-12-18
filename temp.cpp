#include <QCoreApplication>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QDebug>
#include "CustomType.h"

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

// Сравнительная функция для сортировки объектов CustomType
bool compareCustomTypes(const CustomType& a, const CustomType& b) {
    auto rangeA = extractRange(a.rangeStr);
    auto rangeB = extractRange(b.rangeStr);

    if (rangeA.first != rangeB.first)
        return rangeA.first < rangeB.first; // Сравниваем по x
    return rangeA.second < rangeB.second;  // Если x равны, сравниваем по y
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Пример списка объектов
    QList<CustomType> list = {
        {1, "example[5-10]"},
        {2, "test[1-3]"},
        {3, "sample[2-4]"},
        {4, "demo[0-1]"},
        {5, "data[3-8]"}
    };

    // Сортировка списка
    std::sort(list.begin(), list.end(), compareCustomTypes);

    // Вывод отсортированного списка
    for (const CustomType& obj : list) {
        qDebug() << "ID:" << obj.id << "Range:" << obj.rangeStr;
    }

    return a.exec();
}