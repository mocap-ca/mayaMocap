#include "udpSocket.h"
#include "item.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <vector>

UdpClient *client = NULL;

bool run = true;

void intHandler( int x )
{
    run = false;
}


int main(int argc, char *argv[])
{

    if( argc != 3 )
    {
        fprintf(stderr, "Usage: %s host port\n", argv[0]);
        return 1;
    }

    char* host = argv[1];
    int   port = atoi(argv[2]);

    if(port < 1024)
    {
        fprintf(stderr, "Invalid port\n");
        return 2;
    }

    signal(SIGINT, intHandler );

    client = new UdpClient();

    if(!client->create()) return 1;
    printf("\n");

    char buffer[1024];

    std::vector< Item > items;
    items.push_back( Item( "test1", 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 1.0 ) );
    items.push_back( Item( "test2", 1.0, 2.0, 5.0, 0.0, 0.0, 0.0, 1.0 ) );


    float testval = 0.0f;


    int c = 0;

    while(run)
    {

        items[0].ty = testval * 2;
        items[1].tx = testval * 3;
        testval += 0.1f;
        if(testval > 10.0f) testval = 0.0f;

        size_t sz = serializeItems( items, buffer, 1024);


        if (c++ == 24)
        {
            printf("%f ", testval);
            fflush( stdout );
            c = 0;
        }
        client->sendDatagram( host, port, buffer, sz);

#ifdef _WIN32 
                Sleep(1000/24);
#else
				usleep( 1000000 / 24 );		
#endif
    }
    

    printf("Closing socket\n");
    delete client;

    
    

}
