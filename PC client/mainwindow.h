#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QBuffer>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QFileDialog>
#include <QTextEdit>
#include <QScrollBar>
#include "tcpclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void connectToHost(QString ip, QString port);
    void disconnectFromHost();
    void sendToHost(QString);
    void setFile(QString);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void displayText(QString);

    void on_disconnectButton_clicked();

    void on_uploadButton_clicked();

private:
   // QTcpServer *server;
    Ui::MainWindow *ui;
    QString fileName;
    TcpClient *client;
    void autoscroll();


};

#endif // MAINWINDOW_H
