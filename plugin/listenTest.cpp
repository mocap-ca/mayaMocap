#include <stdio.h>
#include <vector>
#include <signal.h>

#include "item.h"
#include "udpSocket.h"

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

    UdpServer server;


    if(! server.bind( port) ) return 2;

    printf("Listening on port: %d\n", port);
    
    std::vector< Item > items;
    char buf[1024];
    int ret;

    int c = 0;

    while(run)
    {

        ret = server.receive();
        if( ret == -1 ) break;
        if( ret == 0 ) continue;

            printf(".");
            fflush(stdout);

        ret = parseItems( server.readBuffer, ret,  items );

        if( c++ > 24 )
        {
        

            printf("Parse returns: %d  Items: %d \n", ret, items.size());

            for(size_t n = 0; n < items.size(); n++)
            {
                Item &i = items[n];
                printf("-- %s  %f %f %f - %f %f %f %f\n",  i.name, i.tx, i.ty, i.tz, 
                            i.rx, i.ry, i.rz, i.rw );
            }
            c=0;
        }
    }

    
    /*

    items.push_back( Item( "test1", 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 1.0 ) );
    items.push_back( Item( "test2", 1.0, 2.0, 5.0, 0.0, 0.0, 0.0, 1.0 ) );

    size_t ret = serializeItems( items, buf, 1024 );

    for(size_t i = 0; i < ret; i++)
    {
        printf("%02x %02c   ", buf[i], buf[i] );
        if(i%8==7) printf("\n");
    }

    dumpData( buf );


    printf("----\n\n");

    std::vector< Item > rebuild;
    }*/

    


}
