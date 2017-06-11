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

    checkboxOnline = new QCheckBox(this);
    layout = new QHBoxLayout(this);
    layout->addWidget( checkboxOnline );
    setLayout(layout);

    setEnabled(false);

    connect(checkboxOnline, SIGNAL(clicked(bool)), this, SLOT(online(bool)));

    checkboxOnline->setMinimumHeight( 19 );
    checkboxOnline->setMaximumHeight( 19 );
    setMinimumHeight( 19 );
    setMaximumHeight( 19 );
    checkboxOnline->setStyleSheet("margin: 0px; border: 1px red solid;");
    setStyleSheet("margin: 0px; border: 1px red solid;");

    layout->setMargin(0);
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

bool TcpConnector::sendData( QByteArray& data)
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

    bool ret = socket->write(data) == data.length();
    socket->flush();

    return ret;
}

bool TcpConnector::sendMessage(QString data)
{
    return sendData(data.toUtf8());
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


// checkbox event
void TcpConnector::online(bool val)
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
        checkboxOnline->setCheckState(Qt::Unchecked);
    }
    else
    if(state == QAbstractSocket::ConnectedState)
    {
        checkboxOnline->setCheckState(Qt::Checked);
        //button->setStyleSheet("background-color: #33ff33; color: #000000");
    }
    else
    {
        checkboxOnline->setCheckState(Qt::PartiallyChecked);
    }

    if(mConnect && state == QAbstractSocket::UnconnectedState)
    {
        mConnect = false;
        _connectSocket();
    }

    //button->setText( socketState( state ));
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


