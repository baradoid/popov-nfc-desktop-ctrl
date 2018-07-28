#pragma once

#include <QThread>

#if defined(Q_OS_LINUX)
#include <PCSC/winscard.h>
#define SCARD_CTL_CODE(code) (0x42000000+(code-100))

#define SCdConn SCardConnect
#define SCARD_READERSTATE_def SCARD_READERSTATE
#define SCardGetStatusChange_def SCardGetStatusChange

#elif defined(Q_OS_WIN)

#include <winscard.h>
#define SCdConn SCardConnectA
#define SCARD_READERSTATE_def SCARD_READERSTATE_A
#define SCardGetStatusChange_def SCardGetStatusChangeA
#endif

class NfcWorkerThread : public QThread
{  
    Q_OBJECT

    void run() override;

public:
    NfcWorkerThread(QObject*);

signals:
    void resultReady(const QString &result);
    void cardDetected(quint64 uid);
    void cardRemoved();
private:
    void getReadersList(SCARDCONTEXT *phSC, QStringList *readerList);
    void getUID(SCARDCONTEXT &hSC, QString rName, uint64_t &uid);
};
