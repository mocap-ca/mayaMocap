#include "tcpconnector.h"

TcpConnector::TcpConnector(QWidget *parent)
: QWidget(parent)
, mConnect(false)
{
    socket = new QTcpSocket(this);
    connect(socket , SIGNAL(connected()),    this, SLOT(sConnect()));
    connect(socket,  SIGNAL(disconnected()), this, SLOT(sDisconnect()));
    connect(socket,  SIGNAL(readyRead()),    this, SLOT(readAndSplit()));
    connect(socket,  SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sError(QAbstractSocket::SocketError)));
    connect(socket,  SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT( sState(QAbstractSocket::SocketState)));

    button = new QPushButton("Connect", this);
    layout = new QHBoxLayout(this);
    layout->addWidget( button );
    setLayout(layout);

    setEnabled(false);

    connect(button, SIGNAL(pressed()), this, SLOT(connect_button()));
}

void TcpConnector::setName( QString name )
{
    mName = name;
}




// called by the socket when connected
void TcpConnector::sConnect()
{
    for(QStringList::iterator i = messages.begin(); i != messages.end(); i++)
    {
        sendMessage((*i).toUtf8());
    }
    messages.clear();
    logMessage("connected");    
    emit socketConnect();
}

// called by the socket when disconnected
void TcpConnector::sDisconnect()
{
    logMessage("disconneted");
    emit socketDisconnect();
}


void TcpConnector::logMessage(QString message)
{
    emit outLog(mName + ": " + message);
}

// External call.  Disconnect and queue the connection request if already connected.
void TcpConnector::connectSocket()
{
    if(isConnected())
    {
        mConnect = true;
        disconnectSocket();
    }
    else
    {
        _connectSocket();
    }
}

void TcpConnector::_connectSocket()
{
    QAbstractSocket::SocketState s = socket->state();
    logMessage(QString("Connecting to %1:%2").arg(mHost).arg(mPort));

    if(socket->state() != QAbstractSocket::UnconnectedState)
    {
        logMessage(QString("Socket already connected while connecting - state is: %1").arg( socketState(s)));
        return;
    }

    socket->connectToHost( mHost, mPort );
}

void TcpConnector::disconnectSocket()
{
    if (socket->state() == QAbstractSocket::UnconnectedState) return;
    socket->abort();
}

void TcpConnector::initialize(QString host, qint16 port)
{
    mHost = host;
    mPort = port;
    setEnabled(true);
}

void TcpConnector::initialize(QPair<QString, int> hostport)
{
    mHost    = hostport.first;
    mPort    = hostport.second;
    setEnabled(true);
}

bool TcpConnector::sendMessage(QString data)
{
    if(!this->isEnabled()) return false;

    QAbstractSocket::SocketState s = socket->state();

    if(s != QAbstractSocket::ConnectedState)
    {
        // cache the message, and try to connect
        messages.append(data);
        if( s == QAbstractSocket::ConnectingState ) return false;
        if( s == QAbstractSocket::HostLookupState ) return false;
        connectSocket();
        return false;
    }

    QByteArray x = data.toUtf8();
    bool ret = socket->write(x) == x.length();
    socket->flush();
    return ret;
}

QString TcpConnector::socketState(QAbstractSocket::SocketState state)
{
    switch(state)
    {
    case QAbstractSocket::UnconnectedState: return QString("Disconnected");
    case QAbstractSocket::HostLookupState:  return QString("Finding Host");
    case QAbstractSocket::ConnectingState:  return QString("Connecting");
    case QAbstractSocket::ConnectedState:   return QString("Connected");
    case QAbstractSocket::BoundState:       return QString("Bound");
    case QAbstractSocket::ClosingState:     return QString("Closing");
    case QAbstractSocket::ListeningState:   return QString("Listening");
    }

    return QString("Error");
}


// button event
void TcpConnector::connect_button()
{
    QAbstractSocket::SocketState s = socket->state();
    if(s == QAbstractSocket::UnconnectedState)
    {
        logMessage("Connecting...");
        this->connectSocket();
    }
    else
    {
        logMessage("Disconnecting...");
        socket->disconnectFromHost();
    }
}


void TcpConnector::sError(QAbstractSocket::SocketError e)
{
    emit socketError( QString("error: " + socket->errorString()));
}

// Called by the socket on state change
void TcpConnector::sState(QAbstractSocket::SocketState state)
{
//    logMessage( socketState( state ));
    if(state == QAbstractSocket::UnconnectedState)
    {
        button->setStyleSheet("");
    }
    else
    if(state == QAbstractSocket::ConnectedState)
    {
        button->setStyleSheet("background-color: #33ff33; color: #000000");
    }
    else
    {
       button->setStyleSheet("background-color: #882222; color: #ffffff");
    }

    if(mConnect && state == QAbstractSocket::UnconnectedState)
    {
        mConnect = false;
        _connectSocket();
    }

    button->setText( socketState( state ));
    emit stateChange( socketState(state) );
}

void TcpConnector::readAndSplit()
{
    QByteArray data = socket->readAll();

    data.replace( QByteArray("\r\n"), QByteArray("\n") );
    data.replace( QByteArray("\n\r"), QByteArray("\n") );

    QList<QByteArray> sp = data.split('\n');
    for( QList<QByteArray>::iterator i = sp.begin(); i != sp.end(); i++)
    {
        QByteArray packet = (*i);
        if(packet.startsWith((char)12)) packet = packet.right( packet.length()-1);
        if(packet.length() == 0) continue;
        emit outData(packet);
    }
}

bool TcpConnector::isConnected()
{
    if(!isEnabled()) return false;
    QAbstractSocket::SocketState state = socket->state();
    return state != QAbstractSocket::UnconnectedState;
}


