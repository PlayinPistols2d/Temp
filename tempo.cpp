void SmartCardReader::readCardUID() {
    BYTE atr[MAX_ATR_SIZE] = { 0 };
    DWORD atrLength = sizeof(atr);
    char readerName[MAX_READERNAME] = { 0 };
    DWORD readerLength = sizeof(readerName);
    DWORD readerState, readerProtocol;

    LONG status = SCardStatus(connectionHandler, readerName, &readerLength, &readerState, &readerProtocol, atr, &atrLength);
    if (status == SCARD_S_SUCCESS) {
        std::cout << "Reading card UID...\nATR: ";
        for (DWORD i = 0; i < atrLength; i++) {
            std::printf("0x%02X ", atr[i]);
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Get card information error: " << pcsc_stringify_error(status) << std::endl;
        disconnectFromCard();
        state = ReaderState::DISCONNECTED;
    }
}