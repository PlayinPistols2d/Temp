case ParamType::DInt: {
    qint64 raw = static_cast<qint64>(value / cmr);
    quint64 magnitude = static_cast<quint64>(std::abs(raw)) & ((1ULL << (lenBit - 1)) - 1); // модуль

    val = magnitude;
    if (raw < 0) {
        val |= (1ULL << (lenBit - 1)); // знак в старший бит
    }

    // Вставка
    val <<= sbit;
    quint64 mask = ((1ULL << lenBit) - 1) << sbit;
    v &= ~mask;
    v |= val;
    break;
}