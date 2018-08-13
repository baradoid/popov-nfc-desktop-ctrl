#include <QTime>
#include <QDebug>
#include "nfcworkerthread.h"


NfcWorkerThread::NfcWorkerThread(QObject* p): QThread(p)
{

}



void NfcWorkerThread::run() {

    SCARDCONTEXT    hSC;
    QStringList readerList;
    int iSelectedReader;

    uint32_t rv;
    //SCARD_READERSTATE_def *rgReaderStates_t = NULL;
    SCARD_READERSTATE_def rgReaderStates[10];

    LONG            lReturn;
    // Establish the context.
    lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSC);
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardEstablishContext");
        emit debugMsg(QString("Failed SCardEstablishContext"));
        return;
    }

    int nbReaders = 1;

//    getReadersList(&hSC, &readerList);
//    iSelectedReader = -1;
//    for(int i=0; i<readerList.size(); i++){
//        if(readerList[i].contains("PICC")){
//            iSelectedReader = i;
//            qDebug("select reader index %d", i);
//            emit debugMsg(QString("select reader index %1").arg(i));
//            break;
//        }
//    }

//    if(iSelectedReader == -1)
//        return;

    //char nnn[500];
    //memcpy(&(nnn[0]), readerList[iSelectedReader].toLocal8Bit().constData(), readerList[iSelectedReader].length()+1);
    //memset( &(rgReaderStates[0]), 0, 2*sizeof(SCARD_READERSTATE_def));
    //rgReaderStates[0].szReader = &(nnn[0]);
    //rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
    //rgReaderStates[0].cbAtr = sizeof rgReaderStates_t[0].rgbAtr;

    rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
    rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;

    int start  = QTime::currentTime().msecsSinceStartOfDay();

    while(isInterruptionRequested() == false){

        rv = SCardGetStatusChange_def(hSC,  250, &(rgReaderStates[0]), nbReaders);

        if(rv == SCARD_S_SUCCESS){
            for(int iCurReader=0; iCurReader<nbReaders; iCurReader++){
                SCARD_READERSTATE_def *rgRState_t = &(rgReaderStates[iCurReader]);
                QString debStr;
                //qDebug("msecs: %d", QTime::currentTime().msecsSinceStartOfDay() - start);
                //start = QTime::currentTime().msecsSinceStartOfDay();

                debStr.sprintf("%d: SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x ", iCurReader,
                               rgRState_t->dwCurrentState , rgRState_t->dwEventState);
                start = QTime::currentTime().msecsSinceStartOfDay();
                //qDebug("SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x", rgReaderStates[0].dwEventState , rgReaderStates[1].dwEventState);


                //uint32_t xorState = (rgReaderStates[0].dwEventState&0x7ff) ^ (rgReaderStates[0].dwCurrentState&0x7ff);
                //qDebug() << qPrintable(QString::asprintf("0x%x xor:0x%x", rgReaderStates[0].dwEventState&0x7ff, xorState));

                if(rgRState_t->dwEventState & SCARD_STATE_IGNORE)
                    debStr+= QString("SCARD_STATE_IGNORE "); // qDebug("SCARD_STATE_IGNORE ");
                if(rgRState_t->dwEventState & SCARD_STATE_CHANGED){
                    debStr += "SCARD_STATE_CHANGED ";
                    qDebug() << qPrintable(debStr);
                    //nbReaders = 1 +nNum;
                    //qDebug() << nbReaders;
                    //QString debEvStr;
                    //debEvStr.sprintf("SCARD_STATE_CHANGED %x", rgReaderStates[0].dwEventState);
                    emit debugMsg(debStr);

                    if(iCurReader == 0){
                        iSelectedReader = -1;
                        readerList.clear();
                        int nNum = (rgRState_t->dwEventState >> 16)&0xf;
                        //if(nNum > 0){
                            getReadersList(&hSC, &readerList);

                            for(int i=0; i<readerList.size(); i++){
                                if(readerList[i].contains("PICC")){
                                    iSelectedReader = i;

                                    char nnn[500];
                                    memcpy(&(nnn[0]), readerList[iSelectedReader].toLocal8Bit().constData(), readerList[iSelectedReader].length()+1);
                                    memset( &(rgReaderStates[1+0]), 0, 2*sizeof(SCARD_READERSTATE_def));

                                    rgReaderStates[1+0].szReader = &(nnn[0]);
                                    rgReaderStates[1+0].dwCurrentState = SCARD_STATE_UNAWARE;
                                    rgReaderStates[1+0].cbAtr = sizeof SCARD_READERSTATE_def::rgbAtr;

                                    qDebug("select reader index %d", i);
                                    emit debugMsg(QString("select reader index %1").arg(i));
                                }
                            }
                        //}
                        if(iSelectedReader == -1)
                            nbReaders = 1;
                        else
                            nbReaders = 2;
                    }
                }
                if(rgRState_t->dwEventState & SCARD_STATE_UNKNOWN){
                    debStr += "SCARD_STATE_UNKNOWN ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_UNAVAILABLE){
                    debStr += "SCARD_STATE_UNAVAILABLE ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_EMPTY){
                    debStr += "SCARD_STATE_EMPTY ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_PRESENT){
                    debStr += "SCARD_STATE_PRESENT ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_ATRMATCH){
                    debStr += "SCARD_STATE_ATRMATCH ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_EXCLUSIVE){
                    debStr += "SCARD_STATE_EXCLUSIVE ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_INUSE){
                    debStr += "SCARD_STATE_INUSE ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_MUTE){
                    debStr += "SCARD_STATE_MUTE ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }
                if(rgRState_t->dwEventState & SCARD_STATE_UNPOWERED){
                    debStr += "SCARD_STATE_UNPOWERED ";
                    qDebug() << qPrintable(debStr);
                    emit debugMsg(debStr);
                }

                if( ((rgRState_t->dwEventState&SCARD_STATE_PRESENT) != 0) &&
                        ((rgRState_t->dwCurrentState&SCARD_STATE_EMPTY) != 0)){


                    QString ATRstr = "ATR: ";
                    for(int i=0; i<rgRState_t->cbAtr; i++){
                        ATRstr += QString::number(rgRState_t->rgbAtr[i], 16) + " ";
                    }
                    qDebug() << qPrintable(ATRstr);

                    quint64 uid;
                    quint8 uidLen;
                    getUID(hSC, readerList[iSelectedReader], uid, uidLen);
                    emit cardDetected(uid, uidLen);

                }
                if( ((rgRState_t->dwEventState&SCARD_STATE_EMPTY) != 0) &&
                        ((rgRState_t->dwCurrentState&SCARD_STATE_PRESENT) != 0)){
                    emit cardRemoved();
                }



                //            if(rgReaderStates[0].dwEventState & SCARD_STATE_UNAVAILABLE){
                //                rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
                //            }
                //            else{
                rgRState_t->dwCurrentState = rgRState_t->dwEventState;
                //            }

            }

        }
        else if(rv == SCARD_E_INVALID_PARAMETER){
            qDebug("SCardGetStatusChange SCARD_E_INVALID_PARAMETER");
        }
        else if(rv == SCARD_E_UNKNOWN_READER){
            qDebug("SCardGetStatusChange SCARD_E_UNKNOWN_READER");
        }
        else if(rv == SCARD_E_TIMEOUT){
            //qDebug("SCardGetStatusChange SCARD_E_TIMEOUT");
            if(nbReaders == 1){
                iSelectedReader = -1;
                readerList.clear();
                //int nNum = (rgRState_t->dwEventState >> 16)&0xf;
                //if(nNum > 0){
                    getReadersList(&hSC, &readerList);

                    for(int i=0; i<readerList.size(); i++){
                        if(readerList[i].contains("PICC")){
                            iSelectedReader = i;

                            char nnn[500];
                            memcpy(&(nnn[0]), readerList[iSelectedReader].toLocal8Bit().constData(), readerList[iSelectedReader].length()+1);
                            memset( &(rgReaderStates[1+0]), 0, 2*sizeof(SCARD_READERSTATE_def));

                            rgReaderStates[1+0].szReader = &(nnn[0]);
                            rgReaderStates[1+0].dwCurrentState = SCARD_STATE_UNAWARE;
                            rgReaderStates[1+0].cbAtr = sizeof SCARD_READERSTATE_def::rgbAtr;

                            qDebug("select reader index %d", i);
                            emit debugMsg(QString("select reader index %1").arg(i));
                        }
                    }
                //}
                if(iSelectedReader == -1)
                    nbReaders = 1;
                else
                    nbReaders = 2;
            }
        }
        else if((rv == SCARD_E_SERVICE_STOPPED) || (rv == SCARD_E_NO_SERVICE) ){
            qDebug("SCardGetStatusChange SCARD_E_SERVICE_STOPPED");            
            //break;

            lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSC);
            if ( SCARD_S_SUCCESS != lReturn ){
                qDebug("Failed SCardEstablishContext");
                emit debugMsg(QString("Failed SCardEstablishContext"));
                break;
            }
        }
        else
            qDebug("SCardGetStatusChange %x", rv);



    }
    qDebug("quit thread");
    //emit resultReady(result);
}



//void NfcWorkerThread::getReadersList()
//{
//    QStringList readerList;
//    LONG            lReturn;
//    // Establish the context.
//    lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM,
//                                    NULL,
//                                    NULL,
//                                    &hSC);
//    if ( SCARD_S_SUCCESS != lReturn ){
//        qDebug("Failed SCardEstablishContext");
//        return;
//    }

//    //QString readerName, readerName2;
//    LPTSTR          pmszReaders = NULL;
//    LPTSTR          pReader;
//    DWORD           cch = SCARD_AUTOALLOCATE;
//    // Retrieve the list the readers.
//    // hSC was set by a previous call to SCardEstablishContext.
//    lReturn = SCardListReaders(hSC, NULL, (LPTSTR)&pmszReaders, &cch );
//    if(lReturn != SCARD_S_SUCCESS){
//        qDebug("Failed SCardListReaders");
//        return;
//    }

//    // Do something with the multi string of readers.
//    // Output the values.
//    // A double-null terminates the list of values.

//    pReader = pmszReaders;
//    char ch;
//    while ( '\0' != *pReader )
//    {
//        QString rname;
//        while ( '\0' != *pReader )
//        {
//            //QString ss;
//            //ss.append()
//            ch = *(pReader)                        ;
//            pReader++;
//            rname.append(ch);
//            // Display the value.
//            // Advance to the next value.
//            //pReader = pReader + wcslen((wchar_t  *)pReader) + 1;
//        }
//        qDebug("Reader: %s", qPrintable(rname));
//        readerList.append(rname);
//        pReader++;
//    }

//    // Free the memory.
//    lReturn = SCardFreeMemory( hSC,
//                                pmszReaders );
//    if ( SCARD_S_SUCCESS != lReturn )
//        qDebug("Failed SCardFreeMemory\n");
//}


void NfcWorkerThread::getReadersList(SCARDCONTEXT *phSC, QStringList *readerList)
{
    //QString readerName, readerName2;
    LONG            lReturn;
    LPSTR          pmszReaders = NULL;
    LPSTR          pReader;
    DWORD           cch = SCARD_AUTOALLOCATE;
    // Retrieve the list the readers.
    // hSC was set by a previous call to SCardEstablishContext.
    lReturn = SCardListReaders_def(*phSC, NULL, (LPSTR)&pmszReaders, &cch );
    if(lReturn != SCARD_S_SUCCESS){
        //qDebug("Failed SCardListReaders");
        //emit debugMsg(QString("Failed SCardListReaders"));
        return;
    }

    // Do something with the multi string of readers.
    // Output the values.
    // A double-null terminates the list of values.

    QString debugStr("SCardListReaders list:");

    int readerInd = 0;
    pReader = pmszReaders;
    while ( '\0' != *pReader )
    {
        QString rname((char*)pReader);

        pReader += rname.length();
        qDebug("Reader: %s", qPrintable(rname));
        debugStr += QString("\nReader %1: ").arg(readerInd) + rname;
        (*readerList).append(rname);
        readerInd++;
        pReader++;
    }
    debugStr += "\n--- end";
    emit debugMsg(debugStr);

    // Free the memory.
    lReturn = SCardFreeMemory( *phSC,
                                pmszReaders );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardFreeMemory\n");
        emit debugMsg(QString("Failed SCardFreeMemory"));
    }
}

void NfcWorkerThread::getUID(SCARDCONTEXT &hSC, QString rName, uint64_t &uid, quint8 &uidLen)
{
    LONG lReturn;
    SCARDHANDLE hCardHandle;
    DWORD dwsend, dwrecv, dwAP;
    uint8_t buf[50], cardCtrl[50];

    lReturn = SCdConn( hSC, qPrintable(rName),
                        SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1,
                       &hCardHandle, (DWORD*)&dwAP );
     if (lReturn != SCARD_S_SUCCESS){
         qDebug("Failed SCardConnect");
         return;
     }
     //qDebug("SCardConnect success 0x%x, dwAp 0x%x", (uint32_t)hCardHandle, dwAP);


//     uint8_t cmdBuf[6];
//     cmdBuf[0] = 0xe0;
//     cmdBuf[1] = 0x00;
//     cmdBuf[2] = 0x00;
//     cmdBuf[3] = 0x24;
//     cmdBuf[4] = 0x00;

//     lReturn = SCardControl( hCardHandle,
//                             SCARD_CTL_CODE(3500),
//                             &(cmdBuf[0]), 5, &(cardCtrl[0]),
//                             50, &dwrecv );
//     if ( lReturn != SCARD_S_SUCCESS ){
//         qDebug("Failed SCardControl %x", lReturn);
//         return;
//     }
//     qDebug("SCardControl success %x", dwrecv);
//     //if(dwrecv == 9){
//     QString recvStr;
//     recvStr = QString::number(dwrecv) + ": ";
//     for(int i=0; i<dwrecv; i++){
//         recvStr += QString::number(cardCtrl[i], 16) + " ";
//     }
//     qDebug(qPrintable(recvStr));


     struct {
         BYTE
                 bCla,   // the instruction class
                 bIns,   // the instruction code
                 bP1,    // parameter to the instruction
                 bP2,    // parameter to the instruction
                 bP3;    // size of I/O transfer
     } CmdBytes;
     CmdBytes.bCla = 0xff;
     CmdBytes.bIns = 0xCA;
     CmdBytes.bP1 = 0x00;
     CmdBytes.bP2 = 0x00;
     CmdBytes.bP3 = 0x00;

     dwsend =  sizeof(CmdBytes);
     dwrecv = 50;
     lReturn = SCardTransmit(hCardHandle,
                             SCARD_PCI_T1,
                             (LPCBYTE)&CmdBytes,
                             dwsend,
                             NULL,
                             &(buf[0]),
             &dwrecv );
     if ( lReturn != SCARD_S_SUCCESS ){
         qDebug("SCardTransmit Failed 0x%x\n", lReturn);
     }

     //qDebug("SCardTransmit success OK");




     if((dwrecv==9) || (dwrecv==6)){
         uidLen = dwrecv;
         uint16_t respCode = buf[dwrecv-2] | (buf[dwrecv-1]<<8);
         if(respCode == 0x90){
             uid = 0;
             //uint64_t uidTemp = 0;
             for(uint32_t i=0; i<(dwrecv-2); i++){
                 //uidTemp = ((uint64_t)buf[i])<<(i*8);
                 uid |= ((uint64_t)buf[i]<<(i*8));
                 //uidStr+= QString::number(buf2[i], 16) + " ";
             }

//             QString uidStr;
//             if(dwrecv == 9)
//                 uidStr.sprintf("0x%014llX", uid);
//             else if(dwrecv == 6)
//                 uidStr.sprintf("0x%08llX", uid);
//             //QString uidStr = qPrintable(QString::number(uid, 16));
//             qDebug(qPrintable(uidStr));


         }
         else{
             qDebug("bad resp code: %x", respCode);
         }
         //                QString uidStr = "uid: ";
         //                for(int i=0; i<dwrecv; i++){
         //                    uidStr+= QString::number(buf2[i], 16) + " ";
         //                }
         //                qDebug(qPrintable(uidStr));
     }





     lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
     if ( lReturn != SCARD_S_SUCCESS )
     {
         qDebug("SCardDisconnect failed %x\n", lReturn);
     }

}



void NfcWorkerThread::buzzerSetCtrl(SCARDCONTEXT &hSC, QString rName, uint8_t buzDuration)
{
    qDebug("\nbuzzerSetCtrl");
    SCARDHANDLE     hCardHandle;
    uint8_t cardCtrlPolling[50];
    LONG    lReturn;

    uint32_t dwrecv, dwAP;

    lReturn = SCdConn( hSC, qPrintable(rName),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_UNDEFINED, //SCARD_PROTOCOL_UNDEFINED,
    &hCardHandle,
    (DWORD*)&dwAP );

    if (lReturn == SCARD_S_SUCCESS)
        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
    else{
        qDebug("Failed SCardConnect");
        return;
    }

    uint8_t cmdBuf[6];
    cmdBuf[0] = 0xe0;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x28;
    cmdBuf[4] = 0x01;
    cmdBuf[5] = buzDuration;

    lReturn = SCardControl( hCardHandle,
                            SCARD_CTL_CODE(3500),
                            &(cmdBuf[0]),
                            6,
                            &(cardCtrlPolling[0]),
                            50,
                            (DWORD*)&dwrecv );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardControl %x", lReturn);
    }
    else{
        QString recvStr = "buzzer set ctrl resp: ";
        recvStr += QString::number(dwrecv) + ": ";
        for(uint32_t i=0; i<dwrecv; i++){
            recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
        }
        qDebug(qPrintable(recvStr));
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
    }

}

void NfcWorkerThread::buzzerGetStatus(SCARDCONTEXT &hSC, QString rName)
{
    qDebug("\nbuzzerGetStatus");

    SCARDHANDLE     hCardHandle;
    uint8_t cardCtrlPolling[50];
    LONG    lReturn;

    uint32_t dwrecv, dwAP;

    lReturn = SCdConn( hSC, qPrintable(rName),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_UNDEFINED,
    &hCardHandle,
    (DWORD*)&dwAP );

    if (lReturn == SCARD_S_SUCCESS)
        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
    else{
        qDebug("Failed SCardConnect");
        return;
    }

    uint8_t cmdBuf[6];
    cmdBuf[0] = 0xe0;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x28;
    cmdBuf[4] = 0x00;

    lReturn = SCardControl( hCardHandle,
                            SCARD_CTL_CODE(3500),
                            &(cmdBuf[0]),
                            5,
                            &(cardCtrlPolling[0]),
                            50,
                            (DWORD*)&dwrecv );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardControl %x", lReturn);
    }
    else{
        //if(dwrecv == 9){
            QString recvStr = "buzzer status: ";
            recvStr += QString::number(dwrecv) + ": ";
            for(uint32_t i=0; i<dwrecv; i++){
                recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
            }
            qDebug(qPrintable(recvStr));
        //}
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
    }

}

void NfcWorkerThread::ledBuzIndSet(SCARDCONTEXT &hSC, QString rName, bool bEventBuzzer)
{
    qDebug("\nledBuzIndSet");
    SCARDHANDLE     hCardHandle;
    uint8_t cardCtrlPolling[50];
    LONG    lReturn;

    uint32_t dwrecv, dwAP;

    lReturn = SCdConn( hSC, qPrintable(rName),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_UNDEFINED, //SCARD_PROTOCOL_UNDEFINED,
    &hCardHandle,
    (DWORD*)&dwAP );

    if (lReturn == SCARD_S_SUCCESS)
        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
    else{
        qDebug("Failed SCardConnect");
        return;
    }

    uint8_t cmdBuf[6];
    cmdBuf[0] = 0xe0;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x21;
    cmdBuf[4] = 0x01;
    cmdBuf[5] = 0x77 | (bEventBuzzer? 0x08:0x00);

    lReturn = SCardControl( hCardHandle,
                            SCARD_CTL_CODE(3500),
                            &(cmdBuf[0]),
                            6,
                            &(cardCtrlPolling[0]),
                            50,
                            (DWORD*)&dwrecv );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardControl");
    }
    else{
        //if(dwrecv == 9){
            QString recvStr = "ledBuzIndStatus resp: ";
            recvStr += QString::number(dwrecv) + ": ";
            for(uint32_t i=0; i<dwrecv; i++){
                recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
            }
            qDebug(qPrintable(recvStr));
        //}
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
    }

}

void NfcWorkerThread::ledBuzIndGetStatus(SCARDCONTEXT &hSC, QString rName)
{
    qDebug("\nledBuzIndGetStatus");
    SCARDHANDLE     hCardHandle;
    uint8_t cardCtrlPolling[50];
    LONG    lReturn;

    uint32_t dwrecv, dwAP;

    lReturn = SCdConn( hSC, qPrintable(rName),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_UNDEFINED, //SCARD_PROTOCOL_UNDEFINED,
    &hCardHandle,
    (DWORD*)&dwAP );

    if (lReturn == SCARD_S_SUCCESS)
        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
    else{
        qDebug("Failed SCardConnect");
        return;
    }
    uint8_t cmdBuf[6];
    cmdBuf[0] = 0xe0;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x21;
    cmdBuf[4] = 0x00;

    lReturn = SCardControl( hCardHandle,
                            SCARD_CTL_CODE(3500),
                            &(cmdBuf[0]),
                            5,
                            &(cardCtrlPolling[0]),
                            50,
                            (DWORD*)&dwrecv );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardControl");
    }
    else{
        //if(dwrecv == 9){
            QString recvStr = "ledBuzIndStatus: ";
            recvStr += QString::number(dwrecv) + ": ";
            for(uint32_t i=0; i<dwrecv; i++){
                recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
            }
            qDebug(qPrintable(recvStr));
        //}
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
    }

}

void NfcWorkerThread::getAutoPICCPolling(SCARDCONTEXT &hSC, QString rName)
{
    qDebug("\ngetAutoPICCPolling");
    SCARDHANDLE     hCardHandle;
    uint8_t cardCtrlPolling[50];
    LONG    lReturn;

    uint32_t dwrecv, dwAP;

    lReturn = SCdConn(hSC, qPrintable(rName),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_UNDEFINED, //SCARD_PROTOCOL_UNDEFINED,
    &hCardHandle,
    (DWORD*)&dwAP );

    if (lReturn == SCARD_S_SUCCESS)
        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
    else{
        qDebug("Failed SCardConnect");
        return;
    }

    uint8_t cmdBuf[6];
    cmdBuf[0] = 0xe0;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x23;
    cmdBuf[4] = 0x00;

    lReturn = SCardControl( hCardHandle,
                            SCARD_CTL_CODE(3500),
                            &(cmdBuf[0]),
                            5,
                            &(cardCtrlPolling[0]),
                            50,
                            (DWORD*)&dwrecv );
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardControl %x", lReturn);
    }
    else{
        //if(dwrecv == 9){
            QString recvStr = "auto polling settings: ";
            recvStr += QString::number(dwrecv) + ": ";
            for(uint32_t i=0; i<dwrecv; i++){
                recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
            }
            qDebug(qPrintable(recvStr));
        //}
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
    }
}



