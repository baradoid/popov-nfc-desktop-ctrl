#include "dialog.h"
#include "ui_dialog.h"
#include <QProcess>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    settings("murinets", "popov-nfc"),
    curTcd(NULL)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    ui->lineEditLongPressDelta->setValidator(new QIntValidator(0,10000));
    ui->progressBar->setValue(0);

    int port = settings.value("port", 3600).toInt();
    ui->lineEditPort->setText(QString::number(port));
    ui->lineEditPort->setValidator(new QIntValidator(0,65535, this));

    int longPressThresh = settings.value("longPressThresh", 2000).toInt();
    ui->lineEditLongPressDelta->setText(QString::number(longPressThresh));
    ui->lineEditLongPressDelta->setValidator(new QIntValidator(0,10000, this));

    ui->lineEditTcpClientsCount->setText("0");


    QString contactHandle = settings.value("contactHandle").toString();
    ui->lineEditStartOnContact->setText(contactHandle);
    QString longContactHandle = settings.value("longContactHandle").toString();
    ui->lineEditStartOnLongContact->setText(longContactHandle);

    tcpServ = new QTcpServer(this);
    connect(tcpServ, SIGNAL(newConnection()), this, SLOT(handleNewTcpConnection()));
    if(tcpServ->listen(QHostAddress::Any, port) == true){
        addLogString(QString("TCP server start ok on port %1").arg(port));
    }
    else{
        addLogString(QString("TCP server start FAIL with: ") + tcpServ->errorString());
    }
//    if(tcpServ->isListening() == true){
//        addLogString("TCP server listen OK");
//    }
//    else{
//        addLogString("TCP server listen FAIL");
//    }

    progressTime.setInterval(100);
    progressTime.setSingleShot(false);
    connect(&progressTime, SIGNAL(timeout()), this, SLOT(handleProgressTick()));


    //ui->listWidget->setAttribute( Qt::WA_TransparentForMouseEvents );


//    getReadersList();

//    for(int i=0; i<readerList.size(); i++){
//        if(readerList[i].contains("PICC")){
//            iSelectedReader = i;
//            qDebug("select reader index %d", i);
//            break;
//        }
//    }

//    if(iSelectedReader == -1)
//        return;


//    getAutoPICCPolling();

//    buzzerSetCtrl(0x0);
//    buzzerGetStatus();


//    ledBuzIndSet(false);
//    ledBuzIndGetStatus();


    //QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    //timer.setInterval(100);
    //timer.start();

    w = new NfcWorkerThread(this);
    //connect(workerThread, &WorkerThread::resultReady, this, &MyObject::handleResults);
    connect(w, &NfcWorkerThread::finished, w, &QObject::deleteLater);
    connect(w, SIGNAL(debugMsg(QString)), this, SLOT(addLogString(QString)));

    connect(w, SIGNAL(cardDetected(quint64,quint8)), this, SLOT(handleCardDetected(quint64, quint8)));
    connect(w, SIGNAL(cardRemoved()), this, SLOT(handleCardRemoved()));
    w->start();

#if defined(Q_OS_LINUX)
    lw = new LibNfcWorkerThread(this);
    connect(lw, &NfcWorkerThread::finished, lw, &QObject::deleteLater);
    connect(lw, SIGNAL(debugMsg(QString)), this, SLOT(addLogString(QString)));

    connect(lw, SIGNAL(cardDetected(quint64,quint8)), this, SLOT(handleCardDetected(quint64,quint8)));
    connect(lw, SIGNAL(cardRemoved()), this, SLOT(handleCardRemoved()));
    lw->start();
#endif

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::red);
    ui->widgetIndic->setAutoFillBackground(true);
    ui->widgetIndic->setPalette(Pal);

}

Dialog::~Dialog()
{
    int port = ui->lineEditPort->text().toInt();
    settings.setValue("port", port);

    int longPressThresh = ui->lineEditLongPressDelta->text().toInt();
    settings.setValue("longPressThresh", longPressThresh);


    QString contactHandle = ui->lineEditStartOnContact->text();
    settings.setValue("contactHandle", contactHandle);
    QString longContactHandle = ui->lineEditStartOnLongContact->text();
    settings.setValue("longContactHandle", longContactHandle);

#if defined(Q_OS_LINUX)
    if(lw->isRunning()){
        lw->requestInterruption();
        lw->wait();
    }
#endif

    w->requestInterruption();
    w->wait();

    delete ui;
}

void Dialog::timeout()
{

    QPalette Pal(palette());


//    uint32_t rv;
//    SCARD_READERSTATE_def *rgReaderStates_t = NULL;
//    SCARD_READERSTATE_def rgReaderStates[2];
//    int nbReaders = 2;

//    memset( &(rgReaderStates[0]), 0, 2*sizeof(SCARD_READERSTATE_def));
//    rgReaderStates[0].szReader = readerList[iSelectedReader].toLocal8Bit().constData();
//    rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
//    rgReaderStates[0].cbAtr = sizeof rgReaderStates_t[0].rgbAtr;

//    rgReaderStates[1].szReader = "\\\\?PnP?\\Notification";
//    rgReaderStates[1].dwCurrentState = SCARD_STATE_UNAWARE;


//    rv = SCardGetStatusChange_def(hSC,  100, rgReaderStates, nbReaders);

//    if(rv == SCARD_S_SUCCESS){
//        QString debStr;
//        debStr.sprintf("SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x ", rgReaderStates[0].dwCurrentState , rgReaderStates[0].dwEventState);
//        //qDebug("SCardGetStatusChange SCARD_S_SUCCESS, curState:0x%x 0x%x", rgReaderStates[0].dwEventState , rgReaderStates[1].dwEventState);
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_IGNORE)
//            debStr+= QString("SCARD_STATE_IGNORE "); // qDebug("SCARD_STATE_IGNORE ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_CHANGED)
//            debStr+= QString("SCARD_STATE_CHANGED ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_UNKNOWN)
//            debStr+= QString("SCARD_STATE_UNKNOWN ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_UNAVAILABLE)
//            debStr+= QString("SCARD_STATE_UNAVAILABLE ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_EMPTY)
//            debStr+= QString("SCARD_STATE_EMPTY ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_PRESENT)
//            debStr+= QString("SCARD_STATE_PRESENT ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_ATRMATCH)
//            debStr+= QString("SCARD_STATE_ATRMATCH ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_EXCLUSIVE)
//            debStr+= QString("SCARD_STATE_EXCLUSIVE ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_INUSE)
//            debStr+= QString("SCARD_STATE_INUSE ");
//        if(rgReaderStates[0].dwEventState & SCARD_STATE_MUTE)
//            debStr+= QString("SCARD_STATE_MUTE ");
//        qDebug() << qPrintable(debStr);

//    }
//    else if(rv == SCARD_E_TIMEOUT)
//        qDebug("SCardGetStatusChange SCARD_E_TIMEOUT");
//    else
//        qDebug("SCardGetStatusChange %x", rv);


    return;
    LONG            lReturn;
    //qDebug("%s", qPrintable(readerName));

    SCARDHANDLE     hCardHandle;  
    DWORD dwsend, dwrecv, dwAP;
    uint8_t buf[50], cardCtrl[50];

//    lReturn = SCdConn( hSC,
//                            readerList[iSelectedReader].toLocal8Bit().constData(),
//    SCARD_SHARE_DIRECT,
//    SCARD_PROTOCOL_T1,
//    &hCardHandle,
//    (DWORD*)&dwAP );

//    if (lReturn == SCARD_S_SUCCESS)
//        qDebug("SCardConnect success 0x%x", (uint32_t)hCardHandle);
//    else{
//        qDebug("Failed SCardConnect");
//        Pal.setColor(QPalette::Background, Qt::red);

//        for(int i=0; i<ui->listWidget->count(); i++){
//            QListWidgetItem *wi = ui->listWidget->item(i);
//            TCardDescription *tc = (TCardDescription*)wi->data(Qt::UserRole).toInt();
//            if(tc->bSeenOnLastPoll == true){
//                for(int i=0; i<clientSockList.size(); i++){
//                    clientSockList[i]->write(qPrintable(tc->uidStr+" "));
//                    clientSockList[i]->write(qPrintable(QString::number(0)+"\n\n"));
//                }
//            }
//            tc->bSeenOnLastPoll = false;
//        }
//        ui->progressBar->setValue(0);
//        return;
//    }



//    uint8_t cmdBuf[6];
//    cmdBuf[0] = 0xe0;
//    cmdBuf[1] = 0x00;
//    cmdBuf[2] = 0x00;
//    cmdBuf[3] = 0x24;
//    cmdBuf[4] = 0x00;

//    lReturn = SCardControl( hCardHandle,
//                            SCARD_CTL_CODE(3500),
//                            &(cmdBuf[0]), 5, &(cardCtrl[0]),
//                            50, &dwrecv );
//    if ( lReturn != SCARD_S_SUCCESS ){
//        qDebug("Failed SCardControl %x", lReturn);
//        return;
//    }
//    else{
//        qDebug("Success SCardControl %x", dwrecv);
//        //if(dwrecv == 9){
//        QString recvStr;
//        recvStr = QString::number(dwrecv) + ": ";
//        for(int i=0; i<dwrecv; i++){
//            recvStr += QString::number(cardCtrl[i], 16) + " ";
//        }
//        qDebug(qPrintable(recvStr));

//        //}
//    }

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

        qDebug("Active protocol T0\n");
        break;


    case SCARD_PROTOCOL_T1:
        Pal.setColor(QPalette::Background, Qt::green);
        qDebug("Active protocol T1\n");

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
        if ( SCARD_S_SUCCESS == lReturn ){
            qDebug("read OK");
            if((dwrecv==9) || (dwrecv==6)){
                uint16_t respCode = buf[dwrecv-2] | (buf[dwrecv-1]<<8);
                if(respCode == 0x90){
                    uint64_t uid = 0;
                    //uint64_t uidTemp = 0;
                    for(uint32_t i=0; i<(dwrecv-2); i++){
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
                    for(int i=0; i<ui->listWidget->count(); i++){
                        QListWidgetItem *wi = ui->listWidget->item(i);
                        TCardDescription *tc = (TCardDescription*)wi->data(Qt::UserRole).toInt();
                        if(wi->text() == uidStr){
                            bExist = true;
                            lwInd = i;
                            lwi = wi;
                            tcd = tc;
                            //tcd->bSeenOnLastPoll = true;
                            //break;
                        }
                        else{
                            tc->bSeenOnLastPoll = false;
                        }
                    }

                    if(bExist == false){
                        ui->listWidget->addItem(uidStr);
                        lwi = ui->listWidget->item(ui->listWidget->count()-1);
                        tcd = new TCardDescription;
                        tcd->uid = uid;
                        tcd->uidStr = uidStr;
                        tcd->uidBytesLen = dwrecv - 2;
                        tcd->bSeenOnLastPoll = false;
                        lwi->setData(Qt::UserRole, (quint64)tcd);

                    }

                    if(tcd->bSeenOnLastPoll == false){
                        tcd->currentSessionStart = QTime::currentTime();
                    }
                    else{
                        int deltaElapsed = tcd->currentSessionStart.elapsed();
                        int deltaThresh = ui->lineEditLongPressDelta->text().toInt();
                        int prcnt = ((float)deltaElapsed/deltaThresh)*100;
                        if(prcnt > 100){
                            prcnt = 100;
                            Pal.setColor(QPalette::Background, Qt::blue);

                            QString startStr =QString(ui->lineEditStartOnContact->text()).arg(tcd->uidStr);
                            addLogString(QString("handle long contact: + \"") + startStr + QString("\""));
                            QProcess::startDetached(startStr);
                        }
                        qDebug("delta: %d %d", deltaElapsed, prcnt);
                        ui->progressBar->setValue(prcnt);
                        //qDebug("delta ch %d", ui->lineEditLongPressDelta->text().toInt());

                        for(int i=0; i<clientSockList.size(); i++){
                            clientSockList[i]->write(qPrintable(uidStr+" "));
                            clientSockList[i]->write(qPrintable(QString::number(prcnt)+"\n\n"));
                        }
                    }
                    tcd->bSeenOnLastPoll = true;

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
            qDebug("Failed read\n");
        }

        break;

    case SCARD_PROTOCOL_UNDEFINED:
    default:
        qDebug("Active protocol unnegotiated or unknown\n");
        break;
    }

    lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if ( SCARD_S_SUCCESS != lReturn )
    {
        qDebug("Failed SCardDisconnect\n");
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


void Dialog::handleNewTcpConnection()
{
    while(tcpServ->hasPendingConnections()){
        QTcpSocket *ts = tcpServ->nextPendingConnection();
        clientSockList.append(ts);
        //connect(ts, SIGNAL(disconnected()), this, SLOT(handleDisconnected()));
        QObject::connect(ts, &QTcpSocket::disconnected, [=](){
            clientSockList.removeOne(ts);
            ui->lineEditTcpClientsCount->setText(QString::number(clientSockList.size()));
        });
    }
    //tcpServ->nextPendingConnection();

    //qDebug("new conn");
    ui->lineEditTcpClientsCount->setText(QString::number(clientSockList.size()));
}

void Dialog::handleCardDetected(quint64 uid, quint8 uidLen)
{
    qDebug("handleCardDetected");
    QString uidStr;
    if(uidLen == 9)
        uidStr.sprintf("0x%014llX", uid);
    else if(uidLen == 6)
        uidStr.sprintf("0x%08llX", uid);
    //QString uidStr = qPrintable(QString::number(uid, 16));
    //qDebug(qPrintable(uidStr));

    bool bExist = false;
    int lwInd = -1;
    QListWidgetItem *lwi;
    TCardDescription *tcd = NULL;
    for(int i=0; i<ui->listWidget->count(); i++){
        QListWidgetItem *wi = ui->listWidget->item(i);
        TCardDescription *tc = (TCardDescription*)wi->data(Qt::UserRole).toInt();
        if(wi->text() == uidStr){
            bExist = true;
            lwInd = i;
            lwi = wi;
            tcd = tc;
            //tcd->bSeenOnLastPoll = true;
            break;
        }
        else{
            tc->bSeenOnLastPoll = false;
        }
    }

    if(bExist == false){
        ui->listWidget->addItem(uidStr);
        lwInd = ui->listWidget->count()-1;
        lwi = ui->listWidget->item(lwInd);
        tcd = new TCardDescription;
        tcd->uid = uid;
        tcd->uidStr = uidStr;
        tcd->uidBytesLen = uidLen - 2;
        tcd->bSeenOnLastPoll = false;
        lwi->setData(Qt::UserRole, (quint64)tcd);
    }

    curTcd = tcd;
    if(tcd->bSeenOnLastPoll == false){
        tcd->currentSessionStart.start();
    }
    else{
        int deltaElapsed = tcd->currentSessionStart.elapsed();
        int deltaThresh = ui->lineEditLongPressDelta->text().toInt();
        int prcnt = ((float)deltaElapsed/deltaThresh)*100;
        if(prcnt > 100){
            prcnt = 100;
        }
        qDebug("delta: %d %d", deltaElapsed, prcnt);
        ui->progressBar->setValue(prcnt);
        //qDebug("delta ch %d", ui->lineEditLongPressDelta->text().toInt());

        for(int i=0; i<clientSockList.size(); i++){
            clientSockList[i]->write(qPrintable(uidStr+" "));
            clientSockList[i]->write(qPrintable(QString::number(prcnt)+"\n\n"));
        }
    }
    tcd->bSeenOnLastPoll = true;

    TConnectionDescr *connDescr = new TConnectionDescr;
    connDescr->time = QTime::currentTime();
    //connDescr->maxTxSpeed = cardCtrl[5];
    //connDescr->currentTxSpeed = cardCtrl[6];
    //connDescr->maxRxSpeed = cardCtrl[7];
    //connDescr->currentRxSpeed = cardCtrl[8];
    tcd->connList.push_front(*connDescr);
    lwi->setSelected(true);
    on_listWidget_currentRowChanged(lwInd);
    progressTime.start();

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::green);
    ui->widgetIndic->setAutoFillBackground(true);
    ui->widgetIndic->setPalette(Pal);

    QString startStr = ui->lineEditStartOnContact->text();
    if(startStr.isEmpty() == false){
        startStr = QString(startStr).arg(tcd->uidStr);
        addLogString(QString("handle short contact: + \"") + startStr + QString("\""));
        QProcess::startDetached(startStr);
    }
}

void Dialog::handleCardRemoved()
{
    qDebug("handleCardRemoved");
    progressTime.stop();

    for(int i=0; i<ui->listWidget->count(); i++){
        QListWidgetItem *wi = ui->listWidget->item(i);
        TCardDescription *tc = (TCardDescription*)wi->data(Qt::UserRole).toInt();
        if(tc->bSeenOnLastPoll == true){
            for(int i=0; i<clientSockList.size(); i++){
                clientSockList[i]->write(qPrintable(tc->uidStr+" "));
                clientSockList[i]->write(qPrintable(QString::number(0)+"\n\n"));                                
            }

            int deltaElapsed = tc->currentSessionStart.elapsed();
            int deltaThresh = ui->lineEditLongPressDelta->text().toInt();
            int prcnt = ((float)deltaElapsed/deltaThresh)*100;
            if(prcnt >= 100){
                prcnt = 100;

                QString startStr = ui->lineEditStartOnLongContact->text();
                if(startStr.isEmpty() == false){
                    startStr = QString(startStr).arg(tc->uidStr);
                    addLogString(QString("handle long contact: + \"") + startStr + QString("\""));
                    QProcess::startDetached(startStr);
                }
            }
            else{
//                QString startStr = ui->lineEditStartOnContact->text();
//                if(startStr.isEmpty() == false){
//                    startStr = QString(startStr).arg(tc->uidStr);
//                    addLogString(QString("handle short contact: + \"") + startStr + QString("\""));
//                    QProcess::startDetached(startStr);
//                }
            }

        }

        tc->bSeenOnLastPoll = false;
    }
    ui->progressBar->setValue(0);

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::red);
    ui->widgetIndic->setAutoFillBackground(true);
    ui->widgetIndic->setPalette(Pal);


    curTcd = NULL;
}

void Dialog::handleProgressTick()
{
    //qDebug("handleProgressTick");
    int deltaElapsed = curTcd->currentSessionStart.elapsed();
    int deltaThresh = ui->lineEditLongPressDelta->text().toInt();
    int prcnt = ((float)deltaElapsed/deltaThresh)*100;
    if(prcnt > 100){
        prcnt = 100;

        QPalette Pal(palette());
        Pal.setColor(QPalette::Background, Qt::blue);
        ui->widgetIndic->setAutoFillBackground(true);
        ui->widgetIndic->setPalette(Pal);
    }
    //qDebug("delta: %d %d", deltaElapsed, prcnt);
    ui->progressBar->setValue(prcnt);
    //qDebug("delta ch %d", ui->lineEditLongPressDelta->text().toInt());

    for(int i=0; i<clientSockList.size(); i++){
        clientSockList[i]->write(qPrintable(curTcd->uidStr+" "));
        clientSockList[i]->write(qPrintable(QString::number(prcnt)+"\n\n"));
    }

}

void Dialog::addLogString(QString s)
{

    ui->textEditLog->append(QTime::currentTime().toString() + "> " + s);

}


//void Dialog::on_pushButton_clicked()
//{
//    QProcess::startDetached(ui->lineEditStartOnContact->text());

//}

void Dialog::on_pushButton_clicked()
{
    QString startStr =QString(ui->lineEditStartOnContact->text()).arg("lala");
    addLogString(QString("handle contact: + \"") + startStr + QString("\""));
    QProcess::startDetached(startStr);
}

void Dialog::on_pushButtonSaveConfig_clicked()
{
    QString contactHandle = ui->lineEditStartOnContact->text();
    settings.setValue("contactHandle", contactHandle);
    QString longContactHandle = ui->lineEditStartOnLongContact->text();
    settings.setValue("longContactHandle", longContactHandle);
}
