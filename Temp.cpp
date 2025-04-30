case ParamType::DInt: {
    qint64 raw = static_cast<qint64>(value / cmr);
    quint64 valWithoutSign = static_cast<quint64>(std::abs(raw)) & ((1ULL << (lenBit - 1)) - 1);

    val = valWithoutSign;

    if (raw < 0) {
        val |= (1ULL << (lenBit - 1)); // ставим бит знака в пределах lenBit
    }

    // Сдвиг в нужное место и вставка
    val <<= sbit;
    quint64 mask = ((1ULL << lenBit) - 1) << sbit;
    v &= ~mask; // очищаем
    v |= val;   // вставляем
    break;
}