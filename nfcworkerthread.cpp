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
    SCARD_READERSTATE_def *rgReaderStates_t = NULL;
    SCARD_READERSTATE_def rgReaderStates[2];

    LONG            lReturn;
    // Establish the context.
    lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSC);
    if ( SCARD_S_SUCCESS != lReturn ){
        qDebug("Failed SCardEstablishContext");
        return;
    }

    int nbReaders = 2;

    getReadersList(&hSC, &readerList);
    iSelectedReader = -1;
    for(int i=0; i<readerList.size(); i++){
        if(readerList[i].contains("PICC")){
            iSelectedReader = i;
            qDebug("select reader index %d", i);
            break;
        }
    }

    if(iSelectedReader == -1)
        return;

    char nnn[500];
    memcpy(&(nnn[0]), readerList[iSelectedReader].toLocal8Bit().constData(), readerList[iSelectedReader].length()+1);
    memset( &(rgReaderStates[0]), 0, 2*sizeof(SCARD_READERSTATE_def));
    rgReaderStates[0].szReader = &(nnn[0]);
    rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
    rgReaderStates[0].cbAtr = sizeof rgReaderStates_t[0].rgbAtr;

    rgReaderStates[1].szReader = "\\\\?PnP?\\Notification";
    rgReaderStates[1].dwCurrentState = SCARD_STATE_UNAWARE;

    int start  = QTime::currentTime().msecsSinceStartOfDay();

    while(isInterruptionRequested() == false){

        rv = SCardGetStatusChange_def(hSC,  250, rgReaderStates, 1);


        if(rv == SCARD_S_SUCCESS){
            QString debStr;
            //qDebug("msecs: %d", QTime::currentTime().msecsSinceStartOfDay() - start);
            //start = QTime::currentTime().msecsSinceStartOfDay();

            debStr.sprintf("%04d SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x ", QTime::currentTime().msecsSinceStartOfDay() - start,
                           rgReaderStates[0].dwCurrentState , rgReaderStates[0].dwEventState);
            start = QTime::currentTime().msecsSinceStartOfDay();
            //qDebug("SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x", rgReaderStates[0].dwEventState , rgReaderStates[1].dwEventState);


            //uint32_t xorState = (rgReaderStates[0].dwEventState&0x7ff) ^ (rgReaderStates[0].dwCurrentState&0x7ff);
            //qDebug() << qPrintable(QString::asprintf("0x%x xor:0x%x", rgReaderStates[0].dwEventState&0x7ff, xorState));

            if(rgReaderStates[0].dwEventState & SCARD_STATE_IGNORE)
                debStr+= QString("SCARD_STATE_IGNORE "); // qDebug("SCARD_STATE_IGNORE ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_CHANGED)
                debStr+= QString("SCARD_STATE_CHANGED ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_UNKNOWN)
                debStr+= QString("SCARD_STATE_UNKNOWN ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_UNAVAILABLE)
                debStr+= QString("SCARD_STATE_UNAVAILABLE ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_EMPTY)
                debStr+= QString("SCARD_STATE_EMPTY ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_PRESENT)
                debStr+= QString("SCARD_STATE_PRESENT ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_ATRMATCH)
                debStr+= QString("SCARD_STATE_ATRMATCH ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_EXCLUSIVE)
                debStr+= QString("SCARD_STATE_EXCLUSIVE ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_INUSE)
                debStr+= QString("SCARD_STATE_INUSE ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_MUTE)
                debStr+= QString("SCARD_STATE_MUTE ");
            if(rgReaderStates[0].dwEventState & SCARD_STATE_UNPOWERED)
                debStr+= QString("SCARD_STATE_UNPOWERED ");

            qDebug() << qPrintable(debStr);

            if( ((rgReaderStates[0].dwEventState&SCARD_STATE_PRESENT) != 0) &&
                ((rgReaderStates[0].dwCurrentState&SCARD_STATE_EMPTY) != 0)){


                QString ATRstr = "ATR: ";
                for(int i=0; i<rgReaderStates[0].cbAtr; i++){
                    ATRstr += QString::number(rgReaderStates[0].rgbAtr[i], 16) + " ";
                }
                qDebug() << qPrintable(ATRstr);

                quint64 uid;
                getUID(hSC, readerList[iSelectedReader], uid);
                emit cardDetected(uid);

            }
            if( ((rgReaderStates[0].dwEventState&SCARD_STATE_EMPTY) != 0) &&
                ((rgReaderStates[0].dwCurrentState&SCARD_STATE_PRESENT) != 0)){
                emit cardRemoved();
            }



//            if(rgReaderStates[0].dwEventState & SCARD_STATE_UNAVAILABLE){
//                rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
//            }
//            else{
                rgReaderStates[0].dwCurrentState = rgReaderStates[0].dwEventState;
//            }


        }
        else if(rv == SCARD_E_TIMEOUT){
            //qDebug("SCardGetStatusChange SCARD_E_TIMEOUT");
        }
        else
            qDebug("SCardGetStatusChange %x", rv);



    }
    qDebug("quit thread");
    //emit resultReady(result);
}



void NfcWorkerThread::getReadersList(SCARDCONTEXT *phSC, QStringList *readerList)
{
    //QString readerName, readerName2;
    LONG            lReturn;
    LPTSTR          pmszReaders = NULL;
    LPTSTR          pReader;
    DWORD           cch = SCARD_AUTOALLOCATE;
    // Retrieve the list the readers.
    // hSC was set by a previous call to SCardEstablishContext.
    lReturn = SCardListReaders(*phSC, NULL, (LPTSTR)&pmszReaders, &cch );
    if(lReturn != SCARD_S_SUCCESS){
        qDebug("Failed SCardListReaders");
        return;
    }

    // Do something with the multi string of readers.
    // Output the values.
    // A double-null terminates the list of values.

    pReader = pmszReaders;
    char ch;
    while ( '\0' != *pReader )
    {
        QString rname;
        while ( '\0' != *pReader )
        {
            //QString ss;
            //ss.append()
            ch = *(pReader)                        ;
            pReader++;
            rname.append(ch);
            // Display the value.
            // Advance to the next value.
            //pReader = pReader + wcslen((wchar_t  *)pReader) + 1;
        }
        qDebug("Reader: %s", qPrintable(rname));
        (*readerList).append(rname);
        pReader++;
    }

    // Free the memory.
    lReturn = SCardFreeMemory( *phSC,
                                pmszReaders );
    if ( SCARD_S_SUCCESS != lReturn )
        qDebug("Failed SCardFreeMemory\n");
}

void NfcWorkerThread::getUID(SCARDCONTEXT &hSC, QString rName, uint64_t &uid)
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
