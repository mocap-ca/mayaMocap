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
#include <QMutex>
#include <QTimer>
#include "rigidBody.h"

#include <QUdpSocket>


class Motive : public QWidget
{
    Q_OBJECT
public:
    explicit Motive(QWidget *parent = 0);
    ~Motive();
    void initialize(bool multicast, QString local, QString remote, int commandPort, int dataPort);
    void setUdp( QHostAddress, int);
    int doConnect();
    void doDisconnect();

    QPushButton* button;
    QPushButton* buttonRb;
    QHBoxLayout* layout;
    QLabel*      label;
    QLabel*      label2;
    QString      currentTC;
    QUdpSocket*  udpSocket;
    QTimer*      fpsTimer;

    int  frameCount;

    bool mConnected;
    bool mPlaying;

    bool record();
    bool setTake(QString);
    bool stop();
    bool play();

    bool store;

    void dataCallback( sFrameOfMocapData* );
    void messageCallback( int, char * );

    QList <int> getRbIds();
    RigidBody getRbData(int id);
    QString   getRbName(int id);

    QHostAddress udpHost;
    int          udpPort;


private:
    int sendMessage(QString message);

    NatNetClient *natNetClient;
    QString mLocal;
    QString mRemote;
    int     mCommandPort;
    int     mDataPort;

    QMap <int, QString> descriptions;
    QMap <int, RigidBody> rbData;

    QMutex dataMutex;


private slots:
    void logMessage(QString);
    void connect_button();
    void testRb();
    void frameInfo(int, QString);
    void fpsEvent();

signals:
    void outLog(QString);
    void outFrame(int, QString);
    void outFps(int);
};


void dataCallback(sFrameOfMocapData *, void *  );
void messageCallback(int, char*);



#endif // MOTIVE_H
