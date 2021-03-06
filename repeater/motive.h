#ifndef MOTIVE_H
#define MOTIVE_H

#include "NatNetTypes.h"
#include "NatNetClient.h"

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QCheckBox>
#include "rigidBody.h"

#include <QUdpSocket>

#define DATA_PACKET_SIZE 8194


class Motive : public QWidget
{
    Q_OBJECT
public:
    explicit Motive(QWidget *parent = 0);
    ~Motive();
    void initialize(bool multicast, QString local, QString remote, int commandPort, int dataPort, QString udpTarget, int udpPort);
    void getDescriptions();
    int doConnect();
    void doDisconnect();

    QCheckBox*   checkboxOnline;
    QPushButton* buttonRb;
    QHBoxLayout* layout;
    QLabel*      label;
    QLabel*      label2;
    QString      currentTC;
    QUdpSocket*  udpSocket;
    QTimer*      fpsTimer;

    QCheckBox*   sendMarkers;
    QCheckBox*   sendRigidbodies;
    QCheckBox*   sendSegments;

    int  frameCount;

    bool mConnected;
    bool mPlaying;

    bool record();
    bool setTake(QString);
    bool stop();
    bool play();

    void dataCallback( sFrameOfMocapData* );
    void messageCallback( int, char * );

    QList <int> getRbIds();
    RigidBody getRbData(int id);
    QString   getRbName(int id);


private:
    int sendMessage(QString message);

    bool    mMulticast;
    NatNetClient *natNetClient;
    QString mLocal;
    QString mRemote;
    int     mCommandPort;
    int     mDataPort;

    QList <QString> mkrDesc;

    QMap <int, QString> rbDesc;
    QMap <int, RigidBody> rbData;
    QMap <int, QString>  skelNames;
    QMap <int, QString > skelDesc;

    QMutex dataMutex;

    QString mUdpServer;
    int     mUdpPort;
    bool    store;


private slots:
    void logMessage(QString);
    void online(bool);
    void testRb();
    void fpsEvent();

signals:
    void outLog(QString);
    void outFrame(int, QString);
    void outFps(int);
    void outRBList(QStringList)
    void outSkelList(QStringList)
    void outMarkerList(QStringList)
};


void dataCallback(sFrameOfMocapData *, void *  );
void messageCallback(int, char*);



#endif // MOTIVE_H
