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

    if( argc != 2 )
    {
        fprintf(stderr, "Usage: %s localListenPort\n", argv[0]);
        return 1;
    }

    const char* host = "192.168.1.91"; // local network broadcast
    int   port = 9499;

    char msg[1024];
    sprintf(msg, "maya 192.168.1.2 %s", argv[1] );

    signal(SIGINT, intHandler );

    client = new UdpClient();

    if(!client->create())
    {
        printf("Could not create udp socket\n");
        return 1;
    }
    printf(" %s\n", msg);

    if(! client->setBroadcast() )
    {
	printf("Could not set broadcast\n");
	delete client;
	return 2;
    }
    
    if(! client->sendDatagram( host, port, msg, strlen(msg)) )
    {
	printf("Could nto send datagram\n");
    }

    printf("Closing socket\n");
    delete client;

}
