case ParamType::DInt: {
    qint64 raw = static_cast<qint64>(value / cmr);
    quint64 magnitude = static_cast<quint64>(std::abs(raw)) & ((1ULL << (lenBit - 1)) - 1); // без знака

    val = magnitude;
    if (raw < 0) {
        val |= (1ULL << (lenBit - 1)); // знак ставим в старший бит в пределах lenBit
    }

    val <<= sbit;
    v |= val;
    break;
}