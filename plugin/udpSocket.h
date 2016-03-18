#ifndef CSOCKET_H
#define CSOCKET_H

#include "baseSocket.h"

#include "item.h"

#define BUFSIZE 9200

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
		return isConnected();		
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
        buflen = ::recvfrom(mSocket, readBuffer, BUFSIZE, 0, (struct sockaddr *)&clientAddress, &addrLen );

        return buflen;
    }
};


class UdpClient : public UdpSocket
{
public:
    bool setBroadcast()
    {
        int x = 1;
        return setsockopt( mSocket, SOL_SOCKET, SO_BROADCAST, (const char*) &x, sizeof(x) ) != -1;
    }

    bool sendDatagram(const char *host, uint16_t port, const char *data, size_t len)
    {
        if( mSocket == -1) return false;

        struct sockaddr_in addr;

		if(!getHostAddr( host, port, addr ))
		{
            fprintf(stderr, "Invalid host\n");
            return false;
        }

#ifdef _WIN32
        return sendto( mSocket, (const char*)data, len, 0, (struct sockaddr*) &addr, sizeof( struct sockaddr ) ) -1;
#else 
        return sendto( mSocket, (void*)data, len, 0, (struct sockaddr*) &addr, sizeof( struct sockaddr ) ) -1;
#endif
	}
        


};


#endif // CSOCKET_H
