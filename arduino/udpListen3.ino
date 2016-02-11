/*  Ardiuno WIFI Controller - Alastair Macleod 2016
 *   
 *  Send a udp broacast packet to BROADCAST_PORT with the data:  cmd dottedIp port
 *    - where cmd can be anything right now (reserved for future use)
 *    - dotted ip and port is the remote ip and port that the arduino should send data to
 *   
 *  
 */  

#define BROADCAST_PORT 9499   // The port that we listen for UDP packets on

// Uncomment this to send data using tcp, othersie udp will be used
//#define USETCP




// My defines


#define LEDPIN 6              // Status LED pin
#define TRIGPIN 5             // Joystick trigger

#define WLAN_SSID       "RTMOCAP"        // cannot be longer than 32 characters
#define WLAN_PASS       ""
#define WLAN_SECURITY   WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2#define WLAN_SSID       "Aquarium"        // cannot be longer than 32 characters

//#define WLAN_SSID "RLJ8J"
//#define WLAN_PASS ""
//#define WLAN_SECURITY WLAN_SEC_WPA2

#define XPIN A0
#define YPIN A1

// Control global variables

int       XValue = -1;
int       YValue = -1;
int       trigger = 0;


float XTotal = 0;
float YTotal = 0;


// Wifi Headers

#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>
#include <utility/socket.h>

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>


#define ADAFRUIT_CC3000_IRQ   2
#define ADAFRUIT_CC3000_VBAT  A3
#define ADAFRUIT_CC3000_CS    8

#define NO_CLIENT -1 
// Create CC3000 instance
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);


#ifdef USETCP
Adafruit_CC3000_Client tcp_client  = NO_CLIENT;
#else
INT32 sendSocket = -1;
#endif

uint32_t host = 0;
int      port = 0;

// Standard headers
#include <string.h>
#include <stdlib.h>

// Encoder
//#include "Encoder.h"
//Encoder myEnc(2, 3);
//long oldPosition  = -999;


void processInput()
{
     // Joystick
   int newX = analogRead(XPIN);
   int newY = analogRead(YPIN);

   String s;


   float diff = abs(newX - XValue + newY - YValue);
   float zero = abs(newX - 514 + newY - 503);
   
  
   if(diff > 2 || zero > 5 )
   {
      XTotal += newX - 514;
      YTotal += newY - 503;

      //String msg;
      //msg += newX + "  " + newY + "  " + XTotal + "   " + YTotal
      //Serial.println(msg);
      
      s += XTotal / 1000.0;
      s += ' ';
      s += YTotal / 1000.0;
      s += ' ';
      s += " 0  0 0 0 1";
      send_request("J", s);
    
      XValue = newX;
      YValue = newY;

   }

   // Trigger
  
   int t = digitalRead(TRIGPIN);
   if(t != trigger)
   {
      Serial.println("Reset");
      XTotal = 0;
      YTotal = 0;
      trigger = t;

      s = "0 0 0  0 0 0 1";
      send_request("J", s);
   }
}


void setDHCP()
{
  // Check DHCP
  Serial.println(F("Connected. Requesting an IP..."));
  while (!cc3000.checkDHCP())
  {
    Serial.print('.');
    delay(100);
  }
  Serial.println(F("DHCP Done."));
}

void setStatic()
{
  uint32_t setIpAddress = cc3000.IP2U32(192, 168, 1, 91 );
  uint32_t setNetmask   = cc3000.IP2U32(255, 255, 255, 0 );
  uint32_t setGateway   = cc3000.IP2U32(192, 168, 1, 254 );
  uint32_t setDns       = cc3000.IP2U32(192, 168, 1, 254 );

  Serial.print( F("IP: "));           cc3000.printIPdotsRev(setIpAddress);
  Serial.print( F("\n"));
  Serial.print( F("\nNetmask: ") );   cc3000.printIPdotsRev( setNetmask );
  Serial.print( F("\nGateway: ") );   cc3000.printIPdotsRev( setGateway );
  Serial.print( F("\nNameserver: ")); cc3000.printIPdotsRev( setDns );
  Serial.print(F("\n"));

  if( !cc3000.setStaticIPAddress(setIpAddress, setNetmask, setGateway, setDns))
  {
       while(1) errorDelay(5); 
  }
}

void showConnection()
{
  Serial.println( F("IP: "));
  uint32_t ip, netmask, gateway, dhcpserv, nsserv;
  while( !cc3000.getIPAddress( &ip, &netmask, &gateway, &dhcpserv, &nsserv))
  {
      Serial.println("XXX");
      errorDelay(1); 
  }
  Serial.println( F("GOT: "));
  cc3000.printIPdotsRev(ip);
  Serial.print( F("\nNetmask: ") );    cc3000.printIPdotsRev( netmask );
  Serial.print( F("\nGateway: ") );    cc3000.printIPdotsRev( gateway );
  Serial.print( F("\nDHCP#:   ") );    cc3000.printIPdotsRev( dhcpserv );
  Serial.print( F("\nNameserver: "));  cc3000.printIPdotsRev( nsserv );
  Serial.print(F("\n"));
}



void connect_wifi() 
{

  blinkDelay(1, "Starting Wifi");
  if (!cc3000.begin()) errorDelay(1); 


  //setStatic()
  
  blinkDelay(2, "Connecting to wifi");
  while(!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY))  errorDelay(1); 

  blinkDelay(3, "Setting DHCP");
  setDHCP();

  showConnection();
  
}

void blinkDelay(int n, const char *msg)
{
  for(int i=0; i< n; i++)
  {
    Serial.print('.'); 
    digitalWrite(LEDPIN, HIGH);
    delay(150);
    digitalWrite(LEDPIN, LOW);
    delay(150);
  }
  Serial.println(msg);
}

void errorDelay(int n)
{
    Serial.print("!");
    for( int i =0; i < n ; i++)
    { 
      digitalWrite(LEDPIN, HIGH);
      delay(100);
      digitalWrite(LEDPIN, LOW);
      delay(100);
    }
    delay(1000); 
  
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Startup"));

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH); 

  pinMode(TRIGPIN, INPUT);

  connect_wifi();
}

void loop() 
{
   // Wait for a udp command to tell us where to connect to
   if( host == 0 || port == 0)
   {
      listenUDP();
      return;
   }

   // connect to the service - udp/tcp option

#ifdef USETCP
   if(tcp_client == NO_CLIENT || !tcp_client.connected())
   {
      // Wait for a command to tell us what to do.
      connectTcp();
      return;
   }
#else
  connectUdp();
#endif

   digitalWrite(LEDPIN, HIGH);

   processInput();


}



/********************
 *   UDP LISTEN     *
 ********************/

void listenUDP()
{
  // Wait for udp data to tell us what we need to do.  This can be broadcast.
  char buf[64];
  
  Serial.println(F("Waiting for server broadcast..."));

  // Create a UDP socket it listen for broadcasts
  sockaddr_in service;
  memset( &service, 0, sizeof(sockaddr_in) );
  service.sin_family = AF_INET;
  service.sin_addr.s_addr = 0;//INADDR_ANY;
  service.sin_port = htons( BROADCAST_PORT );
  INT32 broadcastSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  if(bind( broadcastSocket, (sockaddr*)&service, sizeof(service) ) == -1)
  {
    Serial.println(F("Could not bind!"));
    Serial.println(BROADCAST_PORT);
    while(1) errorDelay( 10 );
  }
  
  digitalWrite(LEDPIN, HIGH);
  Serial.print(F("Listening for UDP on port: "));
  Serial.print(BROADCAST_PORT);
  Serial.print(F("\n"));

  // Get a UDP packet
  sockaddr_in from;
  socklen_t fromLen;
  int ret = recvfrom( broadcastSocket, buf, 64, 0L, (sockaddr*)&from, &fromLen);
  //disconnect(broadcastSocket);
  closesocket(broadcastSocket);

  // Check for a failure
  if( ret == -1 )
  {
    Serial.println(F("ERROR"));
    errorDelay(6);
    return;
  }

  // Empty packet
  if(ret == 0) return;

  // Received data.  woot!
  Serial.println(F("CONNECTED"));
  char cmd[32];
  int hA, hB, hC, hD;
  buf[ret] = 0;

  Serial.println(buf);

  // Parse the command
  if( sscanf(buf, "%s %d.%d.%d.%d %d", cmd, &hA, &hB, &hC, &hD, &port) == 6)
  {
    host = cc3000.IP2U32( hA, hB, hC, hD );
  }
  else
  {
    Serial.print(buf);
    Serial.println(F(" - parse Error"));    
  }
}





#ifdef USETCP


void connectTcp()
{
    Serial.print(F("Connecting to "));
    cc3000.printIPdotsRev(host);
    Serial.print(F(" port "));
    Serial.println( port );
     
    tcp_client = cc3000.connectTCP( host, port );
    digitalWrite(LEDPIN, HIGH);
    delay(500);
    digitalWrite(LEDPIN, LOW);
    delay(500);
}


// Function to send a TCP request and get the result as a string
void send_request(String message) {

  if(tcp_client == NO_CLIENT)
  {
    Serial.println(F("No client."));
    delay(300);
    return;
  }

  // Send request
  digitalWrite(LEDPIN, LOW);
  //Serial.println(request);


  int ret = tcp_client.write(data, 5);
  if( ret < 0 )
  {
    errorDelay(5);
    tcp_client.close();
    tcp_client = NO_CLIENT;
    return ret;
  }

  Serial.println( ret );

  tcp_client.write(node.c_str(), node.length());
  tcp_client.write(message.c_str(), message.length());
    
  delay(10);
  digitalWrite(LEDPIN, HIGH);
}

#else


void connectUdp()
{
    if(sendSocket == -1)
  {
    Serial.println("Creating send socket");
    sendSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  }
}


// Function to send a UDP message
void send_request(String node, String message) {



  sockaddr_in addr;
  memset( &addr, 0, sizeof(sockaddr_in) );
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(host);
  addr.sin_port = htons(port);

  char data[100];
  data[0] = 42;
  data[1] = 1;
  data[2] = 43;
  data[3] = node.length();
  data[4] = message.length();
  char *ptr = & data[5];
  memcpy( ptr, node.c_str(), node.length());
  ptr += node.length();
  memcpy( ptr, message.c_str(), message.length());
  ptr += message.length();
  

  // Send request
  digitalWrite(LEDPIN, LOW);
  /*cc3000.printIPdotsRev(host);
  Serial.print(":");
  Serial.print(port);
  Serial.print(" --- ");
  Serial.print(node);
  Serial.print("  ");
  Serial.print(message);
  Serial.print("  ");
 */ 
  INT16 ret = sendto(sendSocket, data, ptr-data, 0, (sockaddr*)&addr, sizeof(addr));
  //Serial.println(ret);

  if( ret == -1 )
  {
     errorDelay(5);
     closesocket(sendSocket);
     sendSocket = -1;
     connectUdp();
  }
  //delay(40);
  digitalWrite(LEDPIN, HIGH);

}


#endif


