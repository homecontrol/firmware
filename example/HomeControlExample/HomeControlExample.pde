#define MAC_ADDRESS         0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define IP_ADDRESS          192, 168, 1, 123
#define COMMAND_SERVER_PORT 80
#define EVENT_SERVER_PORT   8080

#define IR_SEND_PIN         3
#define IR_RECV_PIN         5
#define RF_SEND_PIN         7
#define RF_RECV_PIN         2

//#define DEBUG

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

#ifdef DEBUG
unsigned long time;
#endif

// Initialization
void setup()
{
	#ifdef DEBUG
	Serial.begin(9600);
	time = millis();
	#endif

    hcs.enableIRIn(IR_RECV_PIN);
    hcs.enableIROut();
    hcs.enableRFOut(RF_SEND_PIN, 9);
    hcs.enableRFIn();

//    hcs.enableDigitalOut(6);
//    hcs.enableDigitalOut(9);
//    hcs.enableDigitalIn(8);

    Ethernet.begin(mac, ip);
    hcs.startCommandServer(COMMAND_SERVER_PORT);
    hcs.startEventServer(EVENT_SERVER_PORT);
}


void loop()
{
	#ifdef DEBUG
	// Print free memory
	if(millis() - time > 1000)
	{
		time = millis();
		Serial.print("memory: ");
		Serial.print(freeMemory(true));
		Serial.println(" bytes left.");
	}
	#endif

    hcs.handleRequests();
    hcs.handleEvents();
}

// vim: ft=cpp
