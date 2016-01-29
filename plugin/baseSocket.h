#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>



class BaseSocket
{
public:
    int   mSocket;
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
        if(mSocket > 0) close(mSocket) ;
    }

    bool isConnected()
    {
        return mSocket > 0;
    }

    virtual bool create() = 0;

    bool bind( int port )
    {

        // Close the socket if open, and create a new one
        if(mSocket != -1) close(mSocket);
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
 

};




#endif // BASE_SOCKET_H
