#ifndef TCPCONNECTOR_H
#define TCPCONNECTOR_H

#include <QWidget>
#include <QString>
#include <QTcpSocket>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QStringList>

class TcpConnector : public QWidget
{
    Q_OBJECT
public:
    explicit TcpConnector( QWidget *parent = 0);

    void setName(QString);
    QTcpSocket *socket;

    QHBoxLayout* layout;
    QCheckBox*   checkboxOnline;
    QStringList  messages;

protected:
    QString mName;
    QString mHost;
    qint16  mPort;
    bool    mConnect;

private:


public:
    void    logMessage(QString);
    void    connectSocket();
    void    _connectSocket();
    void    disconnectSocket();
    void    initialize(QString host, qint16 port);
    void    initialize(QPair<QString, int>);
    bool    sendMessage(QString);
    bool    sendData( QByteArray& );
    QString socketState(QAbstractSocket::SocketState state);
    bool    isConnected();



protected slots:

    void sConnect();
    void sDisconnect();

    void online(bool);
    void readAndSplit();

    void sState(QAbstractSocket::SocketState);
    void sError(QAbstractSocket::SocketError);

signals:
    void socketConnect();
    void socketDisconnect();
    void socketError(QString);
    void outLog(QString);
    void stateChange(QString);
    void outData(QByteArray);

};

#endif // TCPCONNECTOR_H
