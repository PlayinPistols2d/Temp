bool SmartCardReader::readCardData(QByteArray &cardData)
{
    SCARDHANDLE hCard;
    DWORD dwActiveProtocol;

    LONG lReturn = SCardConnect(hContext,
                                (LPCWSTR)readerName.utf16(),
                                SCARD_SHARE_SHARED,
                                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                &hCard,
                                &dwActiveProtocol);
    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to connect to card: %1").arg(lReturn);
        return false;
    }

    // APDU command to get the UID
    QByteArray apduCommand = QByteArray::fromHex("FFCA000000");

    SCARD_IO_REQUEST pioSendPci;
    if (dwActiveProtocol == SCARD_PROTOCOL_T0) {
        pioSendPci = *SCARD_PCI_T0;
    } else if (dwActiveProtocol == SCARD_PROTOCOL_T1) {
        pioSendPci = *SCARD_PCI_T1;
    } else {
        errorMessage = "Unknown active protocol.";
        SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        return false;
    }

    BYTE pbRecvBuffer[256];
    DWORD cbRecvLength = sizeof(pbRecvBuffer);

    lReturn = SCardTransmit(hCard,
                            &pioSendPci,
                            (LPCBYTE)apduCommand.constData(),
                            apduCommand.size(),
                            NULL,
                            pbRecvBuffer,
                            &cbRecvLength);

    SCardDisconnect(hCard, SCARD_LEAVE_CARD);

    if (lReturn != SCARD_S_SUCCESS) {
        errorMessage = QString("Failed to transmit APDU: %1").arg(lReturn);
        return false;
    }

    // Check for success status word (SW1 SW2)
    if (cbRecvLength >= 2) {
        BYTE sw1 = pbRecvBuffer[cbRecvLength - 2];
        BYTE sw2 = pbRecvBuffer[cbRecvLength - 1];
        if (sw1 == 0x90 && sw2 == 0x00) {
            // Success, extract UID
            cardData = QByteArray(reinterpret_cast<char*>(pbRecvBuffer), cbRecvLength - 2);
            return true;
        } else {
            errorMessage = QString("APDU command failed with status: %1 %2")
                               .arg(QString::number(sw1, 16).toUpper(), 2, QLatin1Char('0'))
                               .arg(QString::number(sw2, 16).toUpper(), 2, QLatin1Char('0'));
            return false;
        }
    } else {
        errorMessage = "Invalid response from card.";
        return false;
    }
}







QObject::connect(&reader, &SmartCardReader::cardDataRead, [](const QByteArray &data) {
    qDebug() << "Card UID:" << data.toHex().toUpper();
});