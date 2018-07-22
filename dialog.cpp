#include "dialog.h"
#include "ui_dialog.h"
#include <winscard.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //ui->listWidget->setAttribute( Qt::WA_TransparentForMouseEvents );

    LONG            lReturn;
    // Establish the context.
    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                    NULL,
                                    NULL,
                                    &hSC);
    if ( SCARD_S_SUCCESS != lReturn )
        printf("Failed SCardEstablishContext\n");
    else
    {

        //QString readerName, readerName2;
        LPTSTR          pmszReaders = NULL;
        LPTSTR          pReader;
        LONG            lReturn, lReturn2;
        DWORD           cch = SCARD_AUTOALLOCATE;
        // Retrieve the list the readers.
        // hSC was set by a previous call to SCardEstablishContext.
        lReturn = SCardListReaders(hSC, NULL, (LPTSTR)&pmszReaders, &cch );

        switch( lReturn )
        {
        case SCARD_E_NO_READERS_AVAILABLE:
        printf("Reader is not in groups.\n");
        // Take appropriate action.
        // ...
        break;
        case SCARD_S_SUCCESS:
            // Do something with the multi string of readers.
            // Output the values.
            // A double-null terminates the list of values.
            pReader = pmszReaders;
            char ch;
            while ( '\0' != *pReader )
            {
                QString ss;
                //ss.append()
                ch = *(pReader)                        ;
                pReader++;
                readerName.append(ch);
                // Display the value.
                //qDebug("Reader: %s\n", pReader );
                // Advance to the next value.
                //pReader = pReader + wcslen((wchar_t  *)pReader) + 1;
            }
            pReader++;
            while ( '\0' != *pReader )
            {
                QString ss;
                //ss.append()
                ch = *(pReader)                        ;
                pReader++;
                readerName2.append(ch);
                // Display the value.
                //qDebug("Reader: %s\n", pReader );
                // Advance to the next value.
                //pReader = pReader + wcslen((wchar_t  *)pReader) + 1;
            }
            // F
            // Free the memory.
            lReturn2 = SCardFreeMemory( hSC,
                                       pmszReaders );
            if ( SCARD_S_SUCCESS != lReturn2 )
                printf("Failed SCardFreeMemory\n");
            break;
        default:
        printf("Failed SCardListReaders\n");
        // Take appropriate action.
        // ...
        break;
        }

    }

    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer.setInterval(100);
    timer.start();

}

Dialog::~Dialog()
{
    delete ui;
}


//__declspec(dllimport) extern const SCARD_IO_REQUEST g_rgSCardT1Pci;

void Dialog::timeout()
{
    QPalette Pal(palette());

    LONG            lReturn;
    //qDebug("%s", qPrintable(readerName));
    wchar_t arr[500];
    memset(&(arr[0]), 0, sizeof(wchar_t)*500);
    readerName.toWCharArray(&(arr[0]));
    SCARDHANDLE     hCardHandle;
    //LONG            lReturn;
    DWORD           dwAP;
    DWORD dwsend, dwrecv;
    uint8_t buf[20], cardCtrl[20];

    lReturn = SCardConnect( hSC,
      &(arr[0]),
    SCARD_SHARE_DIRECT,
    SCARD_PROTOCOL_T1,
    &hCardHandle,
    &dwAP );

//    lReturn = SCardConnect( hSC,
//      &(arr[0]),
//    SCARD_SHARE_DIRECT,
//    SCARD_PROTOCOL_UNDEFINED,
//    &hCardHandle,
//    &dwAP );


    if ( SCARD_S_SUCCESS != lReturn )
    {
        //qDebug("Failed SCardConnect\n");
        //exit(1);  // Or other appropriate action.
        Pal.setColor(QPalette::Background, Qt::red);
    }
    else{

        uint8_t cmdBuf[6];
        cmdBuf[0] = 0xe0;
        cmdBuf[1] = 0x00;
        cmdBuf[2] = 0x00;
        cmdBuf[3] = 0x24;
        cmdBuf[4] = 0x00;

        lReturn = SCardControl( hCardHandle,
                                SCARD_CTL_CODE(3500),
                                &(cmdBuf[0]),
                                5,
                                &(cardCtrl[0]),
                                20,
                                &dwrecv );
        if ( SCARD_S_SUCCESS != lReturn ){
            qDebug("Failed SCardControl");
        }
        else{
            if(dwrecv == 9){
                //QString recvStr;
                //recvStr = QString::number(dwrecv) + ": ";
                //for(int i=0; i<dwrecv; i++){
                //    recvStr += QString::number(cardCtrl[i], 16) + " ";
                //}
                //qDebug(qPrintable(recvStr));
            }
        }


        uint8_t cardCtrlPolling[20];
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
                                20,
                                &dwrecv );
        if ( SCARD_S_SUCCESS != lReturn ){
            qDebug("Failed SCardControl");
        }
        else{
            //if(dwrecv == 9){
                QString recvStr = "auto polling settings: ";
                recvStr += QString::number(dwrecv) + ": ";
                for(int i=0; i<dwrecv; i++){
                    recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
                }
                qDebug(qPrintable(recvStr));
            //}
        }

//        cmdBuf[0] = 0xe0;
//        cmdBuf[1] = 0x00;
//        cmdBuf[2] = 0x00;
//        cmdBuf[3] = 0x23;
//        cmdBuf[4] = 0x01;
//        cmdBuf[5] = 0x8B;

//        lReturn = SCardControl( hCardHandle,
//                                SCARD_CTL_CODE(3500),
//                                &(cmdBuf[0]),
//                                6,
//                                &(cardCtrlPolling[0]),
//                                20,
//                                &dwrecv );
//        if ( SCARD_S_SUCCESS != lReturn ){
//            qDebug("Failed SCardControl");
//        }
//        else{
//            //if(dwrecv == 9){
//                QString recvStr = "auto polling settings resposnse: ";
//                recvStr += QString::number(dwrecv) + ": ";
//                for(int i=0; i<dwrecv; i++){
//                    recvStr += QString::number(cardCtrlPolling[i], 16) + " ";
//                }
//                qDebug(qPrintable(recvStr));
//            //}
//        }



        // Use the connection.
        // Display the active protocol.
        switch ( dwAP )
        {
        case SCARD_PROTOCOL_T0:

            //qDebug("Active protocol T0\n");
            break;
        case SCARD_PROTOCOL_T1:
            Pal.setColor(QPalette::Background, Qt::green);
            //qDebug("Active protocol T1\n");

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
            dwrecv = 20;
            lReturn = SCardTransmit(hCardHandle,
                                    SCARD_PCI_T1,
                                    (LPCBYTE)&CmdBytes,
                                    dwsend,
                                    NULL,
                                    &(buf[0]),
                                    &dwrecv );
            if ( SCARD_S_SUCCESS == lReturn )
            {
                //qDebug("read OK");
                if((dwrecv==9) || (dwrecv==6)){
                    uint16_t respCode = buf[dwrecv-2] | (buf[dwrecv-1]<<8);
                    if(respCode == 0x90){
                        uint64_t uid = 0;
                        //uint64_t uidTemp = 0;
                        for(int i=0; i<(dwrecv-2); i++){
                            //uidTemp = ((uint64_t)buf[i])<<(i*8);
                            uid |= ((uint64_t)buf[i]<<(i*8));
                            //uidStr+= QString::number(buf2[i], 16) + " ";
                        }

                        QString uidStr;
                        if(dwrecv == 9)
                            uidStr.sprintf("0x%014llX", uid);
                        else if(dwrecv == 6)
                            uidStr.sprintf("0x%08llX", uid);
                        //QString uidStr = qPrintable(QString::number(uid, 16));
                        //qDebug(qPrintable(uidStr));

                        bool bExist = false;
                        int lwInd = 0;
                        QListWidgetItem *lwi;
                        TCardDescription *tcd;
                        for(lwInd=0; lwInd<ui->listWidget->count(); lwInd++){
                            lwi = ui->listWidget->item(lwInd);
                            if(lwi->text() == uidStr){
                                bExist = true;
                                tcd = (TCardDescription*)lwi->data(Qt::UserRole).toInt();
                                break;
                            }
                        }
                        if(bExist == false){
                            ui->listWidget->addItem(uidStr);
                            lwi = ui->listWidget->item(ui->listWidget->count()-1);
                            tcd = new TCardDescription;
                            tcd->uid = uid;
                            tcd->uidStr = uidStr;
                            tcd->uidBytesLen = dwrecv - 2;
                            lwi->setData(Qt::UserRole, (int)tcd);

                        }
                        TConnectionDescr *connDescr = new TConnectionDescr;
                        connDescr->time = QTime::currentTime();
                        connDescr->maxTxSpeed = cardCtrl[5];
                        connDescr->currentTxSpeed = cardCtrl[6];
                        connDescr->maxRxSpeed = cardCtrl[7];
                        connDescr->currentRxSpeed = cardCtrl[8];
                        tcd->connList.push_front(*connDescr);
                        lwi->setSelected(true);
                        on_listWidget_currentRowChanged(lwInd);
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
                else{
                    qDebug("recvd bad arrLen %ld : %x %x ", dwrecv, buf[dwrecv-2],buf[dwrecv-1]);
                }
            }
            else{
                //qDebug("Failed read\n");
            }

            break;

        case SCARD_PROTOCOL_UNDEFINED:
        default:
            //qDebug("Active protocol unnegotiated or unknown\n");
            break;
        }



        lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
        if ( SCARD_S_SUCCESS != lReturn )
        {
            qDebug("Failed SCardDisconnect\n");
        }

    }




    ui->widgetIndic->setAutoFillBackground(true);
    ui->widgetIndic->setPalette(Pal);


}

void Dialog::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow == -1)
        return;
    QListWidgetItem *lwi = ui->listWidget->item(currentRow);
    TCardDescription *tcd = (TCardDescription*)lwi->data(Qt::UserRole).toInt();
    QString str;
    str += "UID: " + tcd->uidStr + "\n";
    str += "UID len:" + QString::number(tcd->uidBytesLen) + " bytes\n";
    str += "last seen, delta: \n";
    int count = tcd->connList.size();
    for(int i=0; i<count; i++){
        QTime tt = tcd->connList.at(i).time;
        QTime ttNext = tt;
        if((i+1)<=(count-1)){
            ttNext = tcd->connList.at(i+1).time;
        }
        int tDelta = tt.msecsSinceStartOfDay()-ttNext.msecsSinceStartOfDay();
        str += tt.toString("HH:mm:ss:zzz") + "  " + QString::number(tDelta) + "    ";
        str += "mt:" + QString::number(tcd->connList.at(i).maxTxSpeed, 16) + "  ";
        str += "ct:" + QString::number(tcd->connList.at(i).currentTxSpeed, 16) + "  ";
        str += "mr:" + QString::number(tcd->connList.at(i).maxRxSpeed, 16) + "  ";
        str += "cr:" + QString::number(tcd->connList.at(i).currentRxSpeed, 16) + "  ";
        str += "\n";
    }

    ui->textEdit->setText(str);
}
