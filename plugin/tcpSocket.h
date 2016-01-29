#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "baseSocket.h"

#include <sstream>
#include <string>

class TcpSocket : public BaseSocket
{
public:

    int buflen;
    char buffer[1024];

    TcpSocket() : buflen(0)
    { }

    bool create()
    {
        printf("Creating tcp socket...");
        mSocket = socket( AF_INET, SOCK_STREAM, 0);
        return mSocket > 0;
    }
    


    bool listen()
    {
        return ::listen( mSocket, 1 ) == 0;
    }

    TcpSocket* accept(int timeout)
    {

        int ret = this->wait(timeout);
        if( ret < 1 ) return NULL;

        TcpSocket *out = new TcpSocket();

        socklen_t len = sizeof( struct sockaddr );
        out->mSocket = ::accept( mSocket, ( struct sockaddr*)  &out->mAddr, &len );
        if( out->mSocket == -1 ) 
        {
            delete out;
            return NULL;
        }

        return out;
    }


    /************
     * Client   
     */


    // Returns -1 on error, 0 for no data, >0 for data
    // Times out after 0.5 secs of waiting for data
    int receive(int timeout)
    {
        if(mSocket == -1)
        {
            fprintf(stderr, "Receive called on null socket error\n");
            return -1;
        }


        // Check to see if there is any data on the line.  Retuns 0 if there is not.
        int ret = this->wait(timeout);
        if(ret < 1) return 0;

        if(buflen > 1020) buflen = 0;

        buflen = recv( mSocket, buffer + buflen, 1024-buflen, 0 );
        //printf("%d %d\n", ret, buflen);
        if (buflen == 0) return -1;
        return buflen;
    }

    bool get( std::string &item )
    {
        if(buflen == 0) return false;
        std::istringstream parser( std::string( buffer, buflen ) );

        std::getline( parser, item, '\n');

        if( parser.eof() )
        {
            // we reached the end of the file before the delimeter
            return false;
        }

        // put whatever is left back in the buffer
        parser.read( buffer, buflen );
        buflen = parser.gcount();

        return true;
    }


    bool connect(const char *host, unsigned short port )
    {
        if(mSocket == -1)
        {
            fprintf(stderr, "Connecting to invalid socket error\n");
            return false;
        }

        hostent *hp = gethostbyname( host );

        mAddr.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
        mAddr.sin_port = htons( port );

        if( ::connect( mSocket, (sockaddr*) &mAddr, sizeof( sockaddr ) ) == -1 )
        {
            mSocket = -1;
            fprintf(stderr, "Could not connect to: %s:%d", host, port);
            return false;
        }
        return true;
    }
};

    





#endif
