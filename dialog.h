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
    uint64_t uid;
    uint8_t uidBytesLen;
    QString uidStr;
    QTime lastSeenTime;
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
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidget_currentRowChanged(int currentRow);
};

#endif // DIALOG_H
