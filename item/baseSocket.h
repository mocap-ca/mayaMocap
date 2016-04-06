#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <sys/types.h>

#ifdef _WIN32
#pragma comment(lib,"Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>   // Needed for _wtoi
#include <stdint.h>

#define CLOSESOCKET closesocket
#else

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

#define CLOSESOCKET close
#define SOCKET int

#endif

#include <stdio.h>
#include <string.h>



class BaseSocket
{
public:
    SOCKET mSocket;
    struct sockaddr_in mAddr; // init by the constructor, excluding the port


    BaseSocket()
    {
        mSocket = -1;
        mAddr.sin_family      = AF_INET;
        mAddr.sin_port        = 0;
        mAddr.sin_addr.s_addr = INADDR_ANY;
    }

    ~BaseSocket()
    {
        if(mSocket > 0) ::CLOSESOCKET(mSocket) ;
    }

    void close()
    {
        if(mSocket > 0) ::CLOSESOCKET(mSocket);
		mSocket = -1;
    }

    bool isConnected()
    {
#ifdef _WIN32
		return mSocket != INVALID_SOCKET;
#else
        return mSocket > 0;
#endif
    }

    virtual bool create() = 0;

    bool bind( uint16_t port )
    {

        // Close the socket if open, and create a new one
        if(mSocket != -1) ::CLOSESOCKET(mSocket);
        if(!create()) return false;

        mAddr.sin_port = htons(port);
        printf("binding to %d\n", port);
        if( ::bind(mSocket, (struct sockaddr*)&mAddr, sizeof(struct sockaddr)) == -1)
        {
            fprintf( stderr, "Cannot bind\n");
            return false;
        }

        return true;
    }


    int wait( int timeout = 500000)
    {
        // check the socket for data
        fd_set read_set;
        FD_ZERO( &read_set );
        FD_SET( mSocket, &read_set );
        if( timeout == -1)
        {
            if( select( mSocket+1, &read_set, NULL, NULL, NULL ) == -1 ) return -1;
        }
        else
        {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = timeout; // 1/2 sec
            if( select( mSocket+1, &read_set, NULL, NULL, &tv ) == -1 ) return -1;
        }
        if(! FD_ISSET( mSocket, &read_set ) ) return 0;
        return 1;
    }

	bool getHostAddr(const char *host, uint16_t port, struct sockaddr_in &addr)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

#ifdef _WIN32
		return inet_pton(AF_INET, host, &addr.sin_addr) != -1;
#else
		return inet_aton(host, &addr.sin_addr) != -1;
#endif
	}
 

};




#endif // BASE_SOCKET_H
