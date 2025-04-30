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
        // Прямой код: просто берем младшие биты знакового числа
        val = static_cast<quint64>(raw) & ((1ULL << lenBit) - 1);
        break;
    }
    case ParamType::RInt: {
        qint64 raw = static_cast<qint64>(value / cmr);
        // Доп. код: интерпретируем как signed, но просто берем как битовый шаблон
        val = static_cast<quint64>(static_cast<qint32>(raw)) & ((1ULL << lenBit) - 1);
        break;
    }
    case ParamType::Float: {
        float scaled = static_cast<float>(value / cmr);
        quint32 ieee = convertToIEEE754(scaled);
        val = static_cast<quint64>(ieee) & ((1ULL << lenBit) - 1);
        break;
    }
    default:
        qWarning() << "Неизвестный тип параметра";
        continue;
    }

    val <<= sbit;  // Сдвигаем в нужную позицию
    v |= val;
}
