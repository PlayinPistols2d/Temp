#ifndef HEXGENERATOR_H
#define HEXGENERATOR_H

#include <QVector>
#include <QString>
#include <QDebug>
#include <QtEndian>

struct Parameter {
    double value;       // Значение параметра
    int startBit;       // Начальный бит
    int endBit;         // Конечный бит
    int startWord;      // Начальное слово
    int endWord;        // Конечное слово
};

class HexGenerator {
public:
    static void generateHexWords(const QVector<Parameter> &params);
};

#endif // HEXGENERATOR_H



#include "HexGenerator.h"

void HexGenerator::generateHexWords(const QVector<Parameter> &params) {
    QMap<int, quint32> words; // Хранение слов с номерами

    for (const auto &param : params) {
        // Преобразуем значение в IEEE754 (float)
        float floatValue = static_cast<float>(param.value);
        quint32 ieee754Value = qToLittleEndian(*reinterpret_cast<quint32*>(&floatValue));

        // Пройдемся по диапазону слов
        for (int wordIndex = param.startWord; wordIndex <= param.endWord; ++wordIndex) {
            int wordStartBit = (wordIndex == param.startWord) ? param.startBit : 0;
            int wordEndBit = (wordIndex == param.endWord) ? param.endBit : 31;

            // Определяем маску для извлечения нужных битов
            quint32 mask = ((1u << (wordEndBit - wordStartBit + 1)) - 1) << wordStartBit;
            quint32 paramBits = (ieee754Value & mask) >> wordStartBit;

            // Обновляем или заполняем слово
            words[wordIndex] |= paramBits;
        }
    }

    // Выводим слова в debug
    for (auto it = words.constBegin(); it != words.constEnd(); ++it) {
        qDebug() << QString("Word %1: 0x%2")
                        .arg(it.key())
                        .arg(it.value(), 8, 16, QLatin1Char('0')).toUpper();
    }
}