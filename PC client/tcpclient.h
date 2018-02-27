#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>
class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

signals:
    void displayText(QString);

public slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void connectToHost(QString ip, QString port);
    void disconnectFromHost();
    void sendToHost(QString);
    void setFile(QString);


private:
    QTcpSocket *sock; //сокет
    quint16 _blockSize;//текущий размер блока данных
    QString _name;//имя клиента
    QString sockIp;
    QString sockPort;
    QString fileName;
    quint64 sizeoffile = 0;
    QString command[1] = {
        "start"
    };
};

#endif // TCPCLIENT_H
