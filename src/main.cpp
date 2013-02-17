#define MAC_ADDRESS         0x90, 0xA2, 0xDA, 0x00, 0xF0, 0x5F
//#define IP_ADDRESS          192, 168, 1, 123
#define COMMAND_SERVER_PORT 80
#define EVENT_SERVER_PORT   8080

#include <SPI.h>
#include <Ethernet.h>
#include "memory.h"
#include "server.h"

byte mac[] = { MAC_ADDRESS };

#ifdef IP_ADDRESS
#if defined(ARDUINO) && ARDUINO >= 100
IPAddress ip(IP_ADDRESS);
#else
byte ip[] = { IP_ADDRESS };
#endif
#endif

HomeControlServer hcs;

void setup()
{
    hcs.enableIRIn();
    hcs.enableIROut();
    hcs.enableIRStatus();

    hcs.enableRFOut();
    hcs.enableRFIn();
    hcs.enableRFStatus();

    #ifdef IP_ADDRESS
    Ethernet.begin(mac, ip);
    #else
    Ethernet.begin(mac);
    #endif

    hcs.startCommandServer(COMMAND_SERVER_PORT);
    hcs.startEventServer(EVENT_SERVER_PORT);
    hcs.enableStatus(8);
}

void loop()
{
    hcs.handleRequests();
    hcs.handleEvents();
}
