//#define MAC_ADDRESS         0x90, 0xA2, 0xDA, 0x00, 0xE7, 0x31 // 1
//#define MAC_ADDRESS         0x90, 0xA2, 0xDA, 0x0F, 0x6D, 0x67 // 2
#define MAC_ADDRESS         0x90, 0xA2, 0xDA, 0x00, 0xF0, 0x5F // 3

//#define IP_ADDRESS          192, 168, 1, 117
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
//    Serial.begin(9600);

    hcs.enableIRIn();
    hcs.enableIROut();
    hcs.enableIRStatus();

    hcs.enableRFIn();
    hcs.enableRFOut();
    hcs.enableRFStatus();

    #ifdef IP_ADDRESS
    Ethernet.begin(mac, ip);
    #else
    Ethernet.begin(mac);
    #endif

//    Serial.print("IP = ");
//    Serial.println(Ethernet.localIP());

    hcs.startCommandServer(COMMAND_SERVER_PORT);
    hcs.startEventServer(EVENT_SERVER_PORT);
    hcs.enableStatus(5); // Status PIN
}

void loop()
{
    hcs.handleRequests();
    hcs.handleEvents();
}


/** RCSwitch Send Example ********************************************************/
/*
#include <rcswitch/RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  mySwitch.setPulseLength(315);
  mySwitch.enableTransmit(6);  // Using Pin #10
}

void loop() {
  mySwitch.sendTriState("FFF00FFF00FF");
  delay(1000);
  mySwitch.sendTriState("FFF00FFF0000");
  delay(1000);
}
*/
/** RCSwitch Receive Example *****************************************************/
/*
#include <rcswitch/RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  //pinMode(2, INPUT);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  //mySwitch.enableReceive(1);  // Receiver on interrupt 1 => that is pin #3
  Serial.println("IRQ active");
}

void loop() {
  if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }

    mySwitch.resetAvailable();
  }
}
*/
