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

unsigned long time;

// Initialization
void setup()
{
    Serial.begin(9600);
    memory();
    Serial.println("initializing ...");

    time = millis();

    Serial.println("enable IR ...");
    hcs.enableIRIn(IR_RECV_PIN);
    memory();
    hcs.enableIROut();
    memory();
    Serial.println("enable RF ...");
    hcs.enableRFOut(RF_SEND_PIN, 9);
    memory();
    hcs.enableRFIn();
    memory();

//    hcs.enableDigitalOut(6);
//    hcs.enableDigitalOut(9);
//    hcs.enableDigitalIn(8);

    Serial.println("enable ethernet ...");
    Ethernet.begin(mac, ip);
    memory();
    Serial.println("enable ethernet servers ...");
    hcs.startCommandServer(COMMAND_SERVER_PORT);
    memory();
    hcs.startEventServer(EVENT_SERVER_PORT);
    memory();
}

void memory()
{
    Serial.print("memory: ");
    Serial.print(freeMemory());
    Serial.println(" bytes left.");
}

void loop()
{
    hcs.handleRequests();
    hcs.handleEvents();
}

// vim: ft=cpp
