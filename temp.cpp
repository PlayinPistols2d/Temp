#include <QString>
#include <QStringList>
#include <QDebug>

bool parseRange(const QString& input, double& minValue, double& maxValue) {
    // Инициализация значений
    minValue = 0.0;
    maxValue = 0.0;

    // Убираем квадратные скобки
    QString cleanedInput = input;
    cleanedInput.remove('[').remove(']');

    // Заменяем запятую на точку для корректного парсинга чисел
    cleanedInput.replace(',', '.');

    // Разделяем строку по символу #
    QStringList parts = cleanedInput.split('#');
    if (parts.size() != 2) {
        qWarning() << "Некорректный формат строки диапазона:" << input;
        return false; // Возвращаем false, если строка некорректна
    }

    // Парсим минимальное и максимальное значения
    bool minOk = false, maxOk = false;
    minValue = parts[0].toDouble(&minOk);
    maxValue = parts[1].toDouble(&maxOk);

    // Проверяем успешность преобразования
    if (!minOk || !maxOk) {
        qWarning() << "Ошибка преобразования значений в числа:" << input;
        return false; // Возвращаем false при ошибке парсинга
    }

    // Возвращаем true, если минимальное значение отрицательное
    return minValue < 0;
}