bool SmartCardReader::connect(const std::string& readerName)
{
    disconnect();

#ifdef _WIN32
#ifdef UNICODE
    // Convert std::string to std::wstring
    std::wstring wReaderName(readerName.begin(), readerName.end());
    LPCWSTR szReader = wReaderName.c_str();
#else
    LPCSTR szReader = readerName.c_str();
#endif
#else
    const char* szReader = readerName.c_str();
#endif

    LONG result = SCardConnect(hContext, szReader, SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS)
    {
        std::cerr << "SCardConnect failed: " << result << std::endl;
        return false;
    }
    return true;
}