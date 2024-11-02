bool ACR122UReader::connectCard() {
    DWORD dwReaderLen = 0;

    // First, retrieve the length of the available readers list
    LONG result = SCardListReadersW(hContext, NULL, NULL, &dwReaderLen);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to list readers (getting length):" << result;
        return false;
    }

    // Allocate memory for the readers list
    WCHAR *mszReaders = new WCHAR[dwReaderLen];

    // Now, get the actual readers list
    result = SCardListReadersW(hContext, NULL, mszReaders, &dwReaderLen);
    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to list readers:" << result;
        delete[] mszReaders;
        return false;
    }

    // Connect to the first reader found
    result = SCardConnectW(hContext, mszReaders, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    delete[] mszReaders;

    if (result != SCARD_S_SUCCESS) {
        qDebug() << "Failed to connect to card:" << result;
        return false;
    }

    return true;
}