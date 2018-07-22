#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <winscard.h>
#include <QTime>
#include <QListWidgetItem>
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
    QString readerName, readerName2;
    SCARDCONTEXT    hSC;
private slots:
    void timeout();
    void on_listWidget_currentRowChanged(int currentRow);
};

#endif // DIALOG_H
