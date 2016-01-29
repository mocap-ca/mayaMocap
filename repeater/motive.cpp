#include "motive.h"
#include "item.h"

void dataCallback( sFrameOfMocapData* data, void *ptr  )
{
    Motive *m = (Motive*)ptr;
    m->dataCallback( data );
}



Motive *gMotive;

void messageCallback( int id, char *msg)
{
    gMotive->messageCallback(id, msg);
}


Motive::Motive(QWidget *parent)
    : QWidget(parent)
    , mConnected(false)
    , mPlaying(false)
    , udpPort(0)
    , store(false)
{
    natNetClient = NULL;

    button = new QPushButton("Connect", this);
    buttonRb = new QPushButton("RB", this);
    layout = new QHBoxLayout(this);
    label  = new QLabel(this);
    label2 = new QLabel(this);

    layout->addWidget(button);
    layout->addWidget(buttonRb);

    layout->addWidget(label);
    layout->addWidget(label2);
    setLayout(layout);

    setEnabled(false);

    connect(button,   SIGNAL(pressed()), this, SLOT(connect_button()));
    connect(buttonRb, SIGNAL(pressed()), this, SLOT(testRb()));
    connect(this,    SIGNAL(outFrame(int,QString)), this, SLOT(frameInfo(int, QString)));

    udpSocket = new QUdpSocket(this);

    fpsTimer = new QTimer(this);

    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(fpsEvent()));
    frameCount = 0;
    fpsTimer->setSingleShot(false);
    fpsTimer->start(1000);
}

Motive::~Motive()
{
    if(natNetClient == NULL) return;
    if(mConnected) natNetClient->Uninitialize();
    delete natNetClient;
}

void Motive::fpsEvent()
{
    emit outFps(frameCount);
    frameCount = 0;
}

void Motive::frameInfo(int frame, QString tcs)
{
    QStringList sl( descriptions.values() );
    label2->setText(sl.join(','));
    label->setText(QString("%1 %2").arg(frame).arg(tcs));
}

void Motive::initialize( bool multicast,
                    QString local,
                   QString remote,
                   int commandPort,
                   int dataPort)
{
    if(natNetClient != NULL) delete natNetClient;

    natNetClient = new NatNetClient(multicast ? 0 : 1);  // 0 = multicast, 1 = unicast

    mLocal       = local;
    mRemote      = remote;
    mCommandPort = commandPort;
    mDataPort    = dataPort;
}

void Motive::setUdp(QHostAddress host, int port)
{
    udpHost = host;
    udpPort = port;
}


int Motive::doConnect()
{
    if(natNetClient == NULL) return -1;

    if(mConnected)
    {
        natNetClient->Uninitialize();
        mConnected = false;
    }

    char local_[1024];
    char remote_[1024];

    std::string slocal = mLocal.toStdString();
    std::string sremote = mRemote.toStdString();

    const char * clocal  = slocal.c_str();
    const char * cremote =  sremote.c_str();
    strcpy(local_, clocal );
    strcpy(remote_, cremote);

    logMessage("Connecting to Motive");
    button->setText("Connecting...");

    int ret = natNetClient->Initialize(local_, remote_, mCommandPort, mDataPort);
    if( ret == 0 )
    {
        mConnected = true;
        button->setText("Disconnect");
        button->setStyleSheet("background-color: #33ff33; color: #000000");

        sDataDescriptions *desc;
        sMarkerSetDescription* md;
        sRigidBodyDescription *rbd;
        sSkeletonDescription *sd;

        int ret = natNetClient->GetDataDescriptions(&desc);

        for(size_t i=0; i < ret; i++)
        {
            switch( desc->arrDataDescriptions[i].type )
            {
                case Descriptor_MarkerSet:
                    md = desc->arrDataDescriptions[i].Data.MarkerSetDescription;
                    break;
                case Descriptor_RigidBody:
                    rbd = desc->arrDataDescriptions[i].Data.RigidBodyDescription;
                    descriptions[rbd->ID] = QString(rbd->szName);
                    break;
                case Descriptor_Skeleton:
                    sd = desc->arrDataDescriptions[i].Data.SkeletonDescription;
                    break;
            }
        }

        natNetClient->SetDataCallback( &::dataCallback, (void*)this );
        gMotive = this;  // yuk.
        natNetClient->SetMessageCallback( &::messageCallback );
        logMessage("Motive Connected");
        //setEnabled(true);

    }
    else
    {
        mConnected = false;
        logMessage("Motive failed to connect");
        doDisconnect();
        //setEnabled(false);


    }
    return ret;
}

void Motive::doDisconnect()
{
    if(natNetClient == NULL) return;
    button->setText("Connect");
    button->setStyleSheet("background-color: #882222; color: #ffffff");
    if(!mConnected) return;
    logMessage("Disconnecting from motive");
    int ret = natNetClient->Uninitialize();
    mConnected = false;
    delete natNetClient;
    natNetClient = NULL;
    return;
}

void Motive::connect_button()
{
    if(mConnected)
    {
        doDisconnect();
    }
    else
    {
        doConnect();
    }
}

void Motive::dataCallback( sFrameOfMocapData *data )
{
    int          frame = data->iFrame;
    unsigned int tc    = data->Timecode;
    unsigned int subf  = data->TimecodeSubframe;

    frameCount++;

    int hour, min, sec, tframe, subframe;
    char buf[64];

    natNetClient->DecodeTimecode(tc, subf, &hour, &min, &sec, &tframe, &subframe);
    natNetClient->TimecodeStringify(tc, subf, buf, 64);

    QStringList slist;


    std::vector<Item> items;

    if(!udpHost.isNull() > 0 && udpPort > 0 && data->nRigidBodies > 0)
    {

        for(size_t i=0; i < data->nRigidBodies; i++)
        {
            const sRigidBodyData &rbd = data->RigidBodies[i];

            QByteArray namedata = descriptions[rbd.ID].toLocal8Bit();
            slist.append( descriptions[rbd.ID] );

            items.push_back( Item(namedata.data(), rbd.x, rbd.y, rbd.z,
                                  rbd.qx, rbd.qy, rbd.qz, rbd.qw ));

        }

        char databuf[1024];
        size_t len = serializeItems( items, databuf, 1024);

        udpSocket->writeDatagram( databuf, len,  udpHost, udpPort );
    }



    if(store)
    {
        dataMutex.lock();
        rbData.clear();
        for(size_t i=0; i < data->nRigidBodies; i++)
        {
            const sRigidBodyData &rbd = data->RigidBodies[i];
            rbData.insert(rbd.ID, RigidBody( rbd ));
        }
        dataMutex.unlock();
    }



    QString tcs(buf);
    int x = tcs.lastIndexOf('.');
    if(x>0) tcs = tcs.left(x);
    if(currentTC != tcs)
    {
        label->setText(QString("%1 %2 %3").arg(frame).arg(tcs).arg(slist.join(':')));
        emit outFrame(frame, tcs);
        currentTC = tcs;
    }
}

void Motive::messageCallback(int id, char *msg)
{
    logMessage(QString("%1 (%2)").arg(msg).arg(id));
}

bool Motive::setTake(QString take)
{
    QString msg = QString("SetRecordTakeName,") + take;
    if(sendMessage(msg.toUtf8().data()) != 0) return false;
    return true;
}

bool Motive::record()
{
    if(mPlaying) stop();
    if(sendMessage("StartRecording") != 0) return false;
    return true;
}

bool Motive::stop()
{
    bool ret;
    if(mPlaying)
    {
        mPlaying = false;
        ret  = sendMessage("TimelineStop") == 0;
        ret &= sendMessage("LiveMode") == 0;
        return ret;
    }
    else
    {
        return sendMessage("StopRecording") == 0;
    }
}

bool Motive::play()
{
    bool ret;
    ret =  sendMessage("EditMode") == 0;
    ret &= sendMessage("TimelinePlay") == 0;
    if(ret) mPlaying = true;
    return ret;
}





int Motive::sendMessage(QString message)
{
    int ret;

    if(!isEnabled()) return -1;

    if(!mConnected)
    {
        ret = doConnect();
        if(ret != 0)
        {
            logMessage("Could not connect");
            return ret;
        }
    }

    int nBytes=2048;
    char response[2048];
    ret = natNetClient->SendMessageAndWait(message.toUtf8().data(), 1, 20, (void**)&response, &nBytes);
#ifdef _DEBUG
    outLog( QString( "Sending Motive: %1 - Code: %2").arg(message).arg(ret) );
#endif
    return ret;
}

void Motive::logMessage(QString msg)
{
    emit outLog(msg);
}

QList<int> Motive::getRbIds()
{
    QList<int> ret;
    dataMutex.lock();
    ret = rbData.keys();
    dataMutex.unlock();
    return ret;
}


QString Motive::getRbName(int id)
{
    QString name;
    dataMutex.lock();
    name = descriptions[id];
    dataMutex.unlock();
    return name;
}

RigidBody Motive::getRbData(int id)
{
    RigidBody ret;
    dataMutex.lock();
    ret = rbData[id];
    dataMutex.unlock();
    return ret;
}


void Motive::testRb()
{
    sRigidBodyData rbd;
    rbd.ID = 0;
    rbd.qx = 1.5;
    rbd.qy = 3;
    rbd.qz = 4.7;
    rbd.qw = 0.2;
    rbData.insert(0, RigidBody( rbd ));
    descriptions[0] = "TEST";
    emit(outFrame(0, "00:00:00:00"));
}


