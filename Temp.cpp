quint64 v = 0;

for (int p = 0; p < i->getParams().count(); p++) {
    auto pdesc = i->getParameter(p);
    double value = pdesc.getValue();
    ParamType type = convertStringToParamType(pdesc.getTp());
    float cmr = static_cast<float>(pdesc.getCmr());
    int sbit = pdesc.getStartBit();
    int lenBit = (pdesc.getEndBit() - sbit) + 1;

    quint64 val = 0;

    switch (type) {
    case ParamType::UInt: {
        quint64 raw = static_cast<quint64>(value / cmr);
        val = raw & ((1ULL << lenBit) - 1);
        break;
    }

    case ParamType::DInt: {
        qint64 raw = static_cast<qint64>(value / cmr);
        // прямой код: старший бит — знак, остальные — модуль
        quint64 magnitude = static_cast<quint64>(std::abs(raw));
        if (raw < 0) {
            val = magnitude & ((1ULL << (lenBit - 1)) - 1); // без знакового бита
            val |= (1ULL << (lenBit - 1)); // ставим знак
        } else {
            val = magnitude & ((1ULL << lenBit) - 1);
        }
        break;
    }

    case ParamType::RInt: {
        qint32 raw = static_cast<qint32>(value / cmr);
        // дополнительный код автоматически сохраняется в qint32
        val = static_cast<quint64>(raw) & ((1ULL << lenBit) - 1);
        break;
    }

    case ParamType::Float: {
        float scaled = static_cast<float>(value / cmr);
        quint32 ieee = convertToIEEE754(scaled); // функция должна возвращать 32-битное представление
        val = static_cast<quint64>(ieee) & ((1ULL << lenBit) - 1);
        break;
    }

    default:
        qWarning() << "Неизвестный тип параметра";
        continue;
    }

    // Сдвигаем значение на нужную позицию и вставляем в итоговое
    val <<= sbit;
    v |= val;
}