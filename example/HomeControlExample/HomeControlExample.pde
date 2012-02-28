#define MAC_ADDRESS         0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define IP_ADDRESS          192, 168, 1, 123
#define COMMAND_SERVER_PORT 80
#define EVENT_SERVER_PORT   8080

#define IR_SEND_PIN         3
#define IR_RECV_PIN         5
#define RF_SEND_PIN         7
#define RF_RECV_PIN         2

// Includes
#include <SPI.h>
#include <Ethernet.h>
#include <HomeControl.h>
#include <IRremote.h>
#include <HCRadio.h>
#include <MemoryFree.h>

// Global objects
byte mac[] = { MAC_ADDRESS };
#if defined(ARDUINO) && ARDUINO >= 100
IPAddress ip(IP_ADDRESS);
#else
byte ip[] = { IP_ADDRESS };
#endif

HomeControlServer hcs;

// Initialization
void setup()
{
    hcs.enableIRIn(IR_RECV_PIN);
    hcs.enableIROut();
    hcs.enableIRStatus(8);

    hcs.enableRFOut(RF_SEND_PIN);
    hcs.enableRFIn();
    hcs.enableRFStatus(6);

//    hcs.enableDigitalOut(6);
//    hcs.enableDigitalOut(9);
//    hcs.enableDigitalIn(8);

    Ethernet.begin(mac, ip);
    hcs.startCommandServer(COMMAND_SERVER_PORT);
    hcs.startEventServer(EVENT_SERVER_PORT);
    hcs.enableStatus(9);
}

void loop()
{
    hcs.handleRequests();
    hcs.handleEvents();
}

// vim: ft=cpp
