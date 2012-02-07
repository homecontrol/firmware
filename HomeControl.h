#ifndef HOMECONTROL_H_
#define HOMECONTROL_H_

#include <Ethernet.h>
// TODO: This is somwhat ugly...
#include "../IRremote/IRremote.h"
#include "../HCRadio/HCRadio.h"
#include "HCHTTPRequest.h"

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

        void enableRFOut(int send_pin, int status_pin = -1);
        void enableRFIn();

        void enableDigitalOut(int pin);
        void enableDigitalIn(int pin);
        void enableAnalogIn(int pin);

        void handleRequests();
        void handleEvents();

    private:
        bool handleIRRawRequest     (EthernetClient& client, HCHTTPRequest& request);
        bool handleIRNECRequest     (EthernetClient& client, HCHTTPRequest& request);
        bool handleRFBinaryRequest  (EthernetClient& client, HCHTTPRequest& request);
        bool handleDigitalOutRequest(EthernetClient& client, HCHTTPRequest& request);
        bool handleDigitalInRequest (EthernetClient& client, HCHTTPRequest& request);
        bool handleAnalogInRequest  (EthernetClient& client, HCHTTPRequest& request);

        bool handleHTTPRequest(EthernetClient& client);

        EthernetServer* command_server;
        EthernetServer* event_server;
        IRsend*         irsend;
        IRrecv*         irrecv;
        HCRadio*		radio;
        int             rf_out_pin;
};

void * operator new(size_t size);
void operator delete(void * ptr);

#endif
