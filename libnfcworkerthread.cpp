#include <QDebug>
#include "libnfcworkerthread.h"

#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
//#include "utils/nfc-utils.h"


LibNfcWorkerThread::LibNfcWorkerThread(QObject* p): QThread(p)
{

}

void LibNfcWorkerThread::run()
{
    nfc_context *context;
    nfc_device *pnd = NULL;

    nfc_init(&context);
    if(context == NULL){
        qDebug() << qPrintable("unable to init libnfc (malloc)");
    }

    pnd = nfc_open(context, NULL);
    if(pnd == NULL){
        qDebug() << qPrintable("unable to open NFC device");
        //nfc_exit(context);
        return;
    }

    if(nfc_initiator_init(pnd) < 0){
        nfc_perror(pnd, "nfc_initiator_init");
        nfc_close(pnd);
        return;
        //nfc_exit(context);
    }

    uint8_t uiPollNr = 1;
    uint8_t uiPeriod = 1;
    nfc_modulation nmModulations[5] = {
        {.nmt = NMT_ISO14443A, .nbr = NBR_106},
        {.nmt = NMT_ISO14443B, .nbr = NBR_106},
        {.nmt = NMT_FELICA, .nbr = NBR_212},
        {.nmt = NMT_FELICA, .nbr = NBR_424},
        {.nmt = NMT_JEWEL, .nbr = NBR_106},
    };
    size_t szModulations = 5;
    nfc_target nt;
    int res = 0;

    qDebug() << qPrintable(QString("NFC reader: %1 opened").arg(nfc_device_get_name(pnd)));

    while(isInterruptionRequested() == false){
        //qDebug() << qPrintable("NFC device will poll during");

        if((res = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt)) < 0){
            //nfc_perror(pnd, "nfc_initiator_poll_target");
            //nfc_close(pnd);
            //nfc_exit(context);
        }


        if(res > 0){
            char *s;
            str_nfc_target(&s, &nt, true);
            qDebug() << qPrintable(QString("str: %1").arg(s));
            //
            //nt.nti.nai.szUidLen
            quint64 uid = 0;
            quint8 uidLen = 9;
            uidLen = nt.nti.nai.szUidLen;

            qDebug() << qPrintable(QString("uidLen: %1").arg(uidLen));

            for(uint32_t i=0; i<uidLen; i++){
                //uidTemp = ((uint64_t)buf[i])<<(i*8);
                uid |= ((uint64_t)nt.nti.nai.abtUid[i]<<(i*8));
                //uidStr+= QString::number(buf2[i], 16) + " ";
            }


            emit cardDetected(uid, uidLen+2);
            //print_nfc_target(&nt, true);
            qDebug() << qPrintable("Waiting for card removing...");
            while(0 == nfc_initiator_target_is_present(pnd, NULL)) {}
            nfc_perror(pnd, "nfc_initiator_target_is_present");
            qDebug() << qPrintable("done.");
            emit cardRemoved();
        }
        else{
            //qDebug() << qPrintable("No target found.");
        }

        //QThread::msleep(250);
    }

    nfc_close(pnd);
    nfc_exit(context);
    qDebug() << qPrintable("NFC thread stopped");
}


