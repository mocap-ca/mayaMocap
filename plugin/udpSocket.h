#ifndef CSOCKET_H
#define CSOCKET_H

#include "baseSocket.h"

#include "item.h"

#define BUFSIZE 1024

// Wrap the unix socket funcions in to a simple class
class UdpSocket : public BaseSocket
{
public:
    double   stubCount;
    bool     isUdp;

    bool create()
    {
        printf("Creating udp socket\n");
        mSocket = socket( AF_INET, SOCK_DGRAM, 0);
        return mSocket > 0;
    }

};

class UdpServer : public UdpSocket
{
public:

    char     readBuffer[BUFSIZE];
    int      buflen;

    UdpServer() : buflen(0) {}

    // returns -1 on error, 0 for no data  >0 for data.
    int receive()
    {
        if(mSocket == -1)
        {
            fprintf(stderr, "Receive called on null socket error\n");
            return -1;
        }

        struct sockaddr_in clientAddress;
        socklen_t addrLen = sizeof(struct sockaddr);

        int ret = this->wait();
        if(ret < 1) return ret;

        // Copy the data in to a local buffer
        buflen = ::recvfrom(mSocket, readBuffer, 1024, 0, (struct sockaddr *)&clientAddress, &addrLen );

        return buflen;
    }
};


class UdpClient : public UdpSocket
{
public:
    bool sendDatagram(const char *host, int port, char *data, size_t len)
    {
        if( mSocket == -1) return false;

        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        if (inet_aton(host, &addr.sin_addr) == -1 )
        {
            fprintf(stderr, "Invalid host\n");
            return false;
        }

        sendto( mSocket, (void*)data, len, 0, (struct sockaddr*) &addr, sizeof( struct sockaddr ) );
    }
        


};


#endif // CSOCKET_H
