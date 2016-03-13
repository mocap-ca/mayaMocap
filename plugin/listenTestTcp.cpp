#include <stdio.h>
#include <vector>
#include <signal.h>
#include <stdlib.h>

#include "tcpSocket.h"

bool run = true;

void intHandler( int x )
{
    run = false;
}


int main(int argc, char*argv[])
{

    if( argc != 2 ) 
    {
        fprintf(stderr, "Usage: %s listenPort\n", argv[0]);
        return 1;
    }

    int port = atoi( argv[1] );

    signal(SIGINT, intHandler );

    TcpSocket server;

    if(! server.bind( port) ) return 2;

    if(! server.listen() )
    {
        fprintf(stderr, "Could not listen\n");
        return 2;
    }

    printf("Listening on port: %d\n", port);
    
    int c = 0;

    std::vector<TcpSocket *> connections;

    while(run)
    {

        TcpSocket *con = server.accept(0);

        bool action = false;

        if(con != NULL)
        {
            connections.push_back(con);
            action = true;
        }

        for( std::vector<TcpSocket*>::iterator i = connections.begin(); i != connections.end(); i++)
        {
            int ret = (*i)->receive(0);
            if( ret == -1 )
            {
                // Connection closed - remove it from the list
                printf("Connection closed\n");
                delete *i;
                i = connections.erase(i);
                
            }
            if( ret > 0 )
            {
                action = true;
                printf("%.*s\n", (*i)->buflen, (*i)->buffer );
            }
        }

#ifdef _WIN32
		if(!action) { Sleep( 500 ) ; }
#else
        if(!action) { usleep( 500000) ; }
#endif
    }
}
