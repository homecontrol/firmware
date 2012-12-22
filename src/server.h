#ifndef HOMECONTROL_H_
#define HOMECONTROL_H_

#include <Ethernet.h>
#include <infrared/IRremote.h>
#include <radio/HCRadio.h>
#include "memory.h"
#include "http_request.h"

#define IR_DEFAULT_KHZ 38

#define RF_DEFAULT_PULSE_WIDTH 433
#define RF_RECEIVER_IRQ 0
#define RF_DEFAULT_SEND_REPEAT 10

#define MAX_REQUEST_SIZE  612

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

        void enableIRIn(int pin);
        void enableIROut(/* pin == 3 */);
        void enableIRStatus(int pin);

        void enableRFIn(/* pin == 2 */);
        void enableRFOut(int pin);
        void enableRFStatus(int pin);

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
        HCRadio*        radio;

        int             status_pin;
        int             ir_status_pin;

};

#if !defined(ARDUINO) || ARDUINO < 100

void * operator new(size_t size);
void operator delete(void * ptr);

#endif
#endif
