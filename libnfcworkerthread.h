#pragma once

#include <QObject>
#include <QThread>

class LibNfcWorkerThread : public QThread
{
    Q_OBJECT
    void run() override;

public:
    LibNfcWorkerThread(QObject*);

signals:
    void resultReady(const QString &result);
    void cardDetected(quint64 uid, quint8 uidLen);
    void cardRemoved();
    void debugMsg(QString);

private:
    //void libNfcInit();
};
