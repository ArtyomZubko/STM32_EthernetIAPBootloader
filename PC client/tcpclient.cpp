#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    sock = new QTcpSocket(this);

    connect(sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(sock, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(sock, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

}

void TcpClient::onConnected()
{
   emit displayText("Connected to " + sockIp + ":" + sockPort);
}

void TcpClient::onDisconnected()
{
   emit displayText("Disconected from " + sockIp + ":" + sockPort);
}

void TcpClient::onReadyRead()
{
    QString incomng = sock->readAll();
    emit displayText(sockIp + ":" + sockPort + " >> " + incomng);

    if (incomng == "uploaded"){
        emit disconnectFromHost();
    }
    if (incomng == "ready"){
        QFile file(fileName);
//        QByteArray arr;
//        qint64 size = file.size();
//        QString s = QString::number(size);
//        arr.append(s);
//        sock->write(arr);

        if(file.open(QIODevice::ReadOnly))
        {
           while (!file.atEnd())
           {

               sock->write(file.readAll() + "datend");
           }
        }
        file.close();
    }



}


void TcpClient::connectToHost(QString ip, QString port){
    sockIp = ip;
    sockPort = port;
    sock->connectToHost(ip,port.toInt());
    //qDebug() << sock->readAll();
    //emit getError(sock->errorString());
}

void TcpClient::disconnectFromHost()
{
    sock->disconnectFromHost();
}

void TcpClient::sendToHost(QString msg) {
    sock->write("clear");
}

void TcpClient::setFile(QString name)
{
    fileName = name;
}


