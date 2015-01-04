#ifndef HOMECONTROL_H_
#define HOMECONTROL_H_

#include <Ethernet.h>
#include <infrared/IRremote.h>
//#include <radio/HCRadio.h>
#include <rcswitch/RCSwitch.h>
#include "memory.h"
#include "http_request.h"

#define IR_DEFAULT_KHZ 38
#define IR_RECV_PIN 1
#define IR_STAT_PIN 4

#define RF_DEFAULT_PULSE_WIDTH 433
#define RF_RECV_IRQ 0 // PIN 2, see http://arduino.cc/it/Reference/AttachInterrupt
#define RF_SEND_PIN 6
#define RF_STAT_PIN 3
#define RF_DEFAULT_SEND_REPEAT 10

#define MAX_REQUEST_SIZE 612

#if !defined(ARDUINO) || ARDUINO < 100
#define EthernetClient Client
#define EthernetServer Server
#endif

class HomeControlServer
{
    public:
        HomeControlServer();
        ~HomeControlServer();

        void startCommandServer(int port);
        void startEventServer(int port);


        void enableIRIn(int pin = IR_RECV_PIN);
        // IR sender must be attached to the PWM pin which depends
        // on the board, see TIMER_PWM_PIN in ext/infrared/IRremoteInt.h.
        void enableIROut();
        void enableIRStatus(int pin = IR_STAT_PIN);

        void enableRFIn(int irq = RF_RECV_IRQ);
        void enableRFOut(int pin = RF_SEND_PIN);
        void enableRFStatus(int pin = RF_STAT_PIN);

        void enableDigitalOut(int pin);
        void enableDigitalIn(int pin);
        void enableAnalogIn(int pin);

        void handleRequests();
        void handleEvents();
        void enableStatus(int pin);

    private:

        bool handleMemoryRequest    (EthernetClient& client, HCHTTPRequest& request);
        bool handleIRRawRequest     (EthernetClient& client, HCHTTPRequest& request);
        bool handleIRNECRequest     (EthernetClient& client, HCHTTPRequest& request);
        bool handleRFTristateRequest(EthernetClient& client, HCHTTPRequest& request);
        bool handleRFRawRequest     (EthernetClient& client, HCHTTPRequest& request);
        bool handleDigitalOutRequest(EthernetClient& client, HCHTTPRequest& request);
        bool handleDigitalInRequest (EthernetClient& client, HCHTTPRequest& request);
        bool handleAnalogInRequest  (EthernetClient& client, HCHTTPRequest& request);

        bool handleHTTPRequest(EthernetClient& client);

        EthernetServer* command_server;
        EthernetServer* event_server;
        IRsend*         irsend;
        IRrecv*         irrecv;
        RCSwitch*       radio;

        int             status_pin;
        int             ir_status_pin;
        int             rf_status_pin;
        int             rf_send_pin;

};

#if !defined(ARDUINO) || ARDUINO < 100

void * operator new(size_t size);
void operator delete(void * ptr);

#endif
#endif
