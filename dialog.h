#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <winscard.h>
#include <QTime>
#include <QListWidgetItem>

#include <QTcpServer>
#include <QTcpSocket>

#include <QSettings>

namespace Ui {
class Dialog;
}

typedef struct {
    uint8_t maxTxSpeed;
    uint8_t currentTxSpeed;
    uint8_t maxRxSpeed;
    uint8_t currentRxSpeed;
    QTime time;
} TConnectionDescr;

typedef struct {
    uint64_t uid;
    uint8_t uidBytesLen;
    QString uidStr;
    bool bSeenOnLastPoll;
    QTime currentSessionStart;
    QList<TConnectionDescr> connList;
} TCardDescription;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QTimer timer;
    QStringList readerList;
    int iSelectedReader;
    SCARDCONTEXT    hSC;

    void buzzerGetStatus();
    void buzzerSetCtrl(uint8_t buzDuration);

    void ledBuzIndSet(bool bEventBuzzer);
    void ledBuzIndGetStatus();

    void getAutoPICCPolling();
    void getReadersList();

    QTcpServer *tcpServ;
    QList<QTcpSocket*> clientSockList;

    QSettings settings;

private slots:
    void timeout();
    void on_listWidget_currentRowChanged(int currentRow);
    void handleNewTcpConnection();
};

#endif // DIALOG_H
