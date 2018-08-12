#pragma once

#include <QThread>

#if defined(Q_OS_LINUX)
#include <PCSC/winscard.h>
#define SCARD_CTL_CODE(code) (0x42000000+(code-100))

#define SCdConn SCardConnect
#define SCARD_READERSTATE_def SCARD_READERSTATE
#define SCardGetStatusChange_def SCardGetStatusChange
#define SCardListReaders_def SCardListReaders

#elif defined(Q_OS_WIN)

#include <winscard.h>
#define SCdConn SCardConnectA
#define SCARD_READERSTATE_def SCARD_READERSTATE_A
#define SCardGetStatusChange_def SCardGetStatusChangeA
#define SCardListReaders_def SCardListReadersA
#endif

class NfcWorkerThread : public QThread
{  
    Q_OBJECT

    void run() override;

public:
    NfcWorkerThread(QObject*);

signals:
    void resultReady(const QString &result);
    void cardDetected(quint64 uid, quint8 uidLen);
    void cardRemoved();
    void debugMsg(QString);

private:    
    void getReadersList(SCARDCONTEXT *phSC, QStringList *readerList);
    void getUID(SCARDCONTEXT &hSC, QString rName, uint64_t &uid, quint8 &uidLen);


    void getAutoPICCPolling(SCARDCONTEXT &hSC, QString rName);
    void buzzerSetCtrl(SCARDCONTEXT &hSC, QString rName, uint8_t buzDuration);
    void buzzerGetStatus(SCARDCONTEXT &hSC, QString rName);
    void ledBuzIndSet(SCARDCONTEXT &hSC, QString rName, bool bEventBuzzer);
    void ledBuzIndGetStatus(SCARDCONTEXT &hSC, QString rName);

};
