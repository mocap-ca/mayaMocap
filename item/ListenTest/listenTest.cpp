#include <stdio.h>
#include <vector>
#include <signal.h>

#include "item.h"
#include "udpSocket.h"

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

bool run = true;

void intHandler( int x )
{
    run = false;
}


int main(int argc, char*argv[])
{
#ifdef _WIN32
	WSADATA wsaData;
	DWORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#endif

    if( argc != 2 ) 
    {
        fprintf(stderr, "Usage: %s listenPort\n", argv[0]);
        return 1;
    }

    int port = atoi( argv[1] );

    signal(SIGINT, intHandler );

    UdpServer server;

    if(! server.bind( port) ) return 2;

    printf("Listening on port: %d\n", port);
    
    std::vector< Item* > items;
    char buf[1024];
    int ret;

    int c = 0;

    while(run)
    {
        ret = server.receive();
        if( ret == -1 ) break;
        if( ret == 0 ) continue;

        printf("%d ", ret);
        fflush(stdout);

        ret = parseItems( server.readBuffer, ret, &items );

		// Only print every one in 24 entries.
        if( c++ > 24 )
        {
            printf("Parse returns: %d  Items: %zu \n", ret, items.size());
			peel::dumpItems(items);
            c=0;
        }
    }
#ifdef _WIN32
	WSACleanup();
#endif
}
