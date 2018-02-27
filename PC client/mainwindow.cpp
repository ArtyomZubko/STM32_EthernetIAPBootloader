#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    client = new TcpClient(this);

    connect(this, SIGNAL(connectToHost(QString, QString)), client, SLOT(connectToHost(QString, QString)));
    connect(client, SIGNAL(displayText(QString)), this, SLOT(displayText(QString)));
    connect(this, SIGNAL(disconnectFromHost()), client, SLOT(disconnectFromHost()));
    connect(this, SIGNAL(sendToHost(QString)), client, SLOT(sendToHost(QString)));
    connect(this, SIGNAL(setFile(QString)), client, SLOT(setFile(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}




//void MainWindow::readyRead()
//{
//    QByteArray buffer;
//    QTcpSocket* clientSocket = (QTcpSocket*)sender();
//    buffer.append(clientSocket->readAll());
//    qDebug() << "incoming from client";
//    ui->textEdit->insertPlainText("Incoming from client:\n");
//    autoscroll();
//    qDebug() << buffer;
//    ui->textEdit->insertPlainText(buffer + "\n");
//    autoscroll();

//    if (buffer == "connected" && ui->checkBox->checkState() == Qt::Checked) clientSocket->write("clear");
//    if (buffer == "connected" && ui->checkBox->checkState() == Qt::Unchecked) clientSocket->write("none");
//    if (buffer == "done") {
//                QFile file(fileName);

//                if(file.open(QIODevice::ReadOnly))
//                    {
//                     while (!file.atEnd()) {
//                           QByteArray sendBuffer = file.readAll(); //capture one frame
//                           clientSocket->write(sendBuffer);
//                      }
//                    }
//                    file.close();
//                    clientSocket->write("end");
//        }
//}

void MainWindow::on_pushButton_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Binary firmware (*.bin)"));
    emit setFile(fileName);//передать имя файла
}

void MainWindow::on_pushButton_2_clicked()
{
    emit connectToHost(ui->lineEdit->text(), ui->port_name->text());
}

void MainWindow::displayText(QString msg)
{
    ui->textEdit->insertPlainText(msg + "\n");


    autoscroll();
}

void MainWindow::autoscroll(){
    QScrollBar *vert = ui->textEdit->verticalScrollBar();
    vert->setValue(vert->maximum());
}


void MainWindow::on_disconnectButton_clicked()
{
    emit disconnectFromHost();
}

void MainWindow::on_uploadButton_clicked()
{
    emit sendToHost("clear");
}
