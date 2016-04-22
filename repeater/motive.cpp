#include "motive.h"

#include "item.h"
#include <vector>
#include <QFile>
#include <QTimer>

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
    , mUdpPort(0)
    , store(false)  
    , frameCount(0)
{
    natNetClient = NULL;

    checkboxOnline = new QCheckBox(this);
    buttonRb = new QPushButton("Update", this);
    layout = new QHBoxLayout(this);
    label  = new QLabel(this);
    label2 = new QLabel(this);

    sendMarkers = new QCheckBox("MKR", this);
    sendMarkers->setChecked(true);
    sendRigidbodies = new QCheckBox("RB", this);
    sendRigidbodies->setChecked(true);
    sendSegments = new QCheckBox("SEG", this);
    sendSegments->setChecked(true);

    layout->addWidget(checkboxOnline, 0);
    layout->addWidget(buttonRb, 0);

    layout->addWidget(label, 0);
    layout->addWidget(label2, 1);
    layout->addWidget(sendMarkers);
    layout->addWidget(sendRigidbodies);
    layout->addWidget(sendSegments);
    setLayout(layout);


    connect(checkboxOnline,   SIGNAL(clicked(bool)), this, SLOT(online(bool)));
    connect(buttonRb,         SIGNAL(pressed()), this, SLOT(testRb()));

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

    if(frameCount == 0)
    {
        this->setStyleSheet("color : #f00");
    }
    else
    {
        this->setStyleSheet("");
    }

    frameCount = 0;

    QStringList rbs( rbDesc.values() );
    int skels = skelDesc.count();

    label2->setText(QString( "RB: %1  SK: %2" ).arg( rbs.join(',')).arg( skels));
    infoMutex.lock();
    label->setText(infoMessage);
    infoMutex.unlock();
}

void Motive::initialize(bool multicast,
                   QString local,
                   QString remote,
                   int commandPort,
                   int dataPort,
                   QString udpTarget,
                   int     udpPort )
{
    mMulticast   = multicast;
    mLocal       = local;
    mRemote      = remote;
    mCommandPort = commandPort;
    mDataPort    = dataPort;

    mUdpServer   = udpTarget;
    mUdpPort     = udpPort;

}



int Motive::doConnect()
{

    if(natNetClient == NULL)
    {
        natNetClient = new NatNetClient(mMulticast ? 0 : 1);
    }

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
    checkboxOnline->setCheckState(Qt::PartiallyChecked);

    int ret = natNetClient->Initialize(local_, remote_, mCommandPort, mDataPort);
    if( ret == 0 )
    {
        mConnected = true;
        
        checkboxOnline->setCheckState(Qt::Checked);

        natNetClient->SetMessageCallback( &::messageCallback );

        getDescriptions();

        natNetClient->SetDataCallback( &::dataCallback, (void*)this );
        gMotive = this;  // yuk.

        logMessage("Motive Connected");

    }
    else
    {
        mConnected = false;
        logMessage("Motive failed to connect");
        doDisconnect();


    }
    return ret;
}

void Motive::getDescriptions()
{
    sDataDescriptions *desc;

    int ret = natNetClient->GetDataDescriptions(&desc);

#ifdef BUG
    QFile ff(QString("C:\\Users\\amacleod\\Desktop\\desc.txt"));
    ff.open( QFile::WriteOnly );
#endif

    for(size_t i=0; i < ret; i++)
    {
        if( desc->arrDataDescriptions[i].type == Descriptor_MarkerSet)
        {
            sMarkerSetDescription* md;

            md = desc->arrDataDescriptions[i].Data.MarkerSetDescription;

#ifdef BUG
            if(ff.isOpen()) ff.write( QString("Markerset: %1\n").arg(md->szName).toUtf8() );
#endif

            char*ptr = *md->szMarkerNames;
            for( size_t  j=0; j < md->nMarkers; j++)
            {
                QString name("%1_%2");
                size_t len = strlen( ptr );
                mkrDesc.append( name.arg(QString(md->szName)).arg( QString(ptr) ) );
#ifdef BUG
                if(ff.isOpen()) ff.write( QString("   Marker: %1 - %2\n").arg(j).arg(ptr).toUtf8() );
#endif
                ptr += len + 1;

            }
        }

        if( desc->arrDataDescriptions[i].type == Descriptor_RigidBody )
        {
            sRigidBodyDescription *rbd;
            rbd = desc->arrDataDescriptions[i].Data.RigidBodyDescription;
            rbDesc[rbd->ID] = QString(rbd->szName);
#ifdef BUG
            if(ff.isOpen()) ff.write( QString("RB: %1 parent: %2  - %3\n").arg( rbd->ID ).arg( rbd->parentID ).arg( rbd->szName).toUtf8());
#endif

        }

        if ( desc->arrDataDescriptions[i].type == Descriptor_Skeleton )
        {
            sSkeletonDescription *sd;
            sd = desc->arrDataDescriptions[i].Data.SkeletonDescription;
            int id = sd->skeletonID;

#ifdef BUG
            if(ff.isOpen()) ff.write( QString("Skel: %1  %2\n").arg( id ).arg( sd->szName ).toUtf8());
#endif
            skelNames[id] = sd->szName;

            for ( size_t  j = 0; j < sd->nRigidBodies; j++)
            {
                int rbid = sd->RigidBodies[j].ID | (sd->skeletonID << 16);
                skelDesc[rbid] = QString ( sd->RigidBodies[j].szName );
#ifdef BUG
                if(ff.isOpen()) ff.write( QString("   RB: %1   %2\n").arg(rbid).arg( sd->RigidBodies[j].szName ).toUtf8());
#endif
            }
        }
    }
#ifdef BUG
    if(ff.isOpen()) ff.close();
#endif
}

void Motive::doDisconnect()
{
    if(natNetClient == NULL) return;
    checkboxOnline->setChecked(false);
    if(!mConnected) return;
    logMessage("Disconnecting from motive");
    natNetClient->Uninitialize();
    mConnected = false;
    delete natNetClient;
    natNetClient = NULL;    
    return;
}

void Motive::online(bool value)
{
    if(value)  doConnect();
    else       doDisconnect();
}

void Motive::dataCallback( sFrameOfMocapData *data )
{
    int          frame = data->iFrame;
    unsigned int tc    = data->Timecode;
    unsigned int subf  = data->TimecodeSubframe;

    int hour, min, sec, tframe, subframe;
    char tcbuf[64];

    frameCount++;



    if(store)
    {
        // Add the data to rbData member
        dataMutex.lock();
        rbData.clear();
        for(size_t i=0; i < data->nRigidBodies; i++)
        {
            const sRigidBodyData &rbd = data->RigidBodies[i];
            rbData.insert(rbd.ID, RigidBody( rbd ));
        }
        dataMutex.unlock();
    }

    if(mUdpServer.length() > 0 && mUdpPort > 0 )
    {
        std::vector<peel::Item*> items;

        if(sendRigidbodies->isChecked())
        {
            // Send each rigidbody as a datagram
            for(size_t i=0; i < data->nRigidBodies; i++)
            {
                const sRigidBodyData &rbd = data->RigidBodies[i];
                items.push_back( new peel::Segment(rbDesc[rbd.ID].toLocal8Bit().data(), rbd.x, rbd.y, rbd.z, rbd.qx, rbd.qy, rbd.qz, rbd.qw) );
            }
        }

        if(sendMarkers->isChecked())
        {
            for(size_t i=0; i < data->nLabeledMarkers; i++)
            {
                const sMarker &mkr = data->LabeledMarkers[i];
                if( i >= mkrDesc.length() ) getDescriptions();
                if ( i < mkrDesc.length() )
                    items.push_back( new peel::Marker( mkrDesc[i].toUtf8(), mkr.x, mkr.y, mkr.z ));
            }
        }

        if(sendSegments->isChecked())
        {
            for(size_t i=0; i < data->nSkeletons; i++)
            {
                const sSkeletonData &skel = data->Skeletons[i];
                for ( size_t j = 0; j < skel.nRigidBodies; j++)
                {
                    const sRigidBodyData &rbd = skel.RigidBodyData[j];
                    QString name = skelDesc[ rbd.ID ];
                    items.push_back( new peel::Segment(name.toLocal8Bit().data(), rbd.x, rbd.y, rbd.z, rbd.qx, rbd.qy, rbd.qz, rbd.qw) );
                }
            }
        }



        char buf[DATA_PACKET_SIZE];
        size_t len = serializeItems( items, buf, DATA_PACKET_SIZE );
        udpSocket->writeDatagram( buf, len,  QHostAddress( mUdpServer), mUdpPort );

        for(std::vector<peel::Item*>::iterator i = items.begin(); i != items.end(); i++)
        {
            delete (*i);
        }

    }

    natNetClient->DecodeTimecode(tc, subf, &hour, &min, &sec, &tframe, &subframe);
    natNetClient->TimecodeStringify(tc, subf, tcbuf, 64);

    QString tcs(tcbuf);
    int x = tcs.lastIndexOf('.');
    if(x>0) tcs = tcs.left(x);
    if(currentTC != tcs)
    {
        //label->setText(QString("%1 %2").arg(frame).arg(tcs));
        emit outFrame(frame, tcs);
        currentTC = tcs;
    }

    infoMutex.lock();
    infoMessage = QString("%1 %2").arg(frame).arg(tcs);
    infoMutex.unlock();

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
    name = rbDesc[id];
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
    getDescriptions();
}


