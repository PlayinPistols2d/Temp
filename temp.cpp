
std::vector<std::string> SmartCardReader::listReaders()
{
    std::vector<std::string> readers;
    LONG result;
    LPTSTR mszReaders = NULL;
    DWORD dwReaders = SCARD_AUTOALLOCATE;

    result = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardListReaders failed: " << result << std::endl;
        return readers;
    }

#ifdef _WIN32
    LPTSTR pReader = mszReaders;
    while (*pReader)
    {
        std::wstring wReaderName(pReader);
        std::string readerName(wReaderName.begin(), wReaderName.end());
        readers.push_back(readerName);
        pReader += wcslen(pReader) + 1;
    }
#else
    char* pReader = mszReaders;
    while (*pReader)
    {
        readers.push_back(std::string(pReader));
        pReader += strlen(pReader) + 1;
    }
#endif

    SCardFreeMemory(hContext, mszReaders);
    return readers;
}