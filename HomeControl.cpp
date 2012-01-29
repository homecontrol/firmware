#include "HomeControl.h"
#include "HCTCPUtils.h"
#include "HCRFTransmit.h"

#define MAX_REQUEST_SIZE  128

// TODO: This should be configurable
#define RF_PULSE_LENGTH   325
#define RF_REPEAT          10

HomeControlServer::HomeControlServer()
: command_server(NULL), event_server(NULL),
  irsend(NULL), irrecv(NULL), rf_out_pin(-1)
{}

HomeControlServer::~HomeControlServer()
{
    delete irrecv;
    delete irsend;
    delete event_server;
    delete command_server;
}

void HomeControlServer::startCommandServer(int port)
{
    delete command_server;
    command_server = new EthernetServer(port);
    command_server->begin();
}

void HomeControlServer::startEventServer(int port)
{
    delete event_server;
    event_server = new EthernetServer(port);
    event_server->begin();
}

void HomeControlServer::enableIRIn(int pin)
{
    delete irrecv;
    irrecv = new IRrecv(pin);
    irrecv->enableIRIn();
}

void HomeControlServer::enableIROut()
{
    delete irsend;
    irsend = new IRsend();
}

void HomeControlServer::enableRFOut(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    rf_out_pin = pin;
}

void HomeControlServer::enableDigitalOut(int pin)
{
    // TODO: Maybe we should remember the output pins and check the commands
    // against the list
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void HomeControlServer::enableDigitalIn(int pin)
{
    pinMode(pin, INPUT);
}

void HomeControlServer::enableAnalogIn(int pin)
{
    // TODO
}

bool HomeControlServer::handleIRNECRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if (req.path[1] && req.path[2] && !req.path[3])
    {
        long bitstring = atol(req.path[1]);
        int nbits = atoi(req.path[2]);
        irsend->sendNEC(bitstring, nbits);
        if (irrecv)
            irrecv->enableIRIn();
        sendHTTPResponse(client, "OK", true);
        return true;
    }

    sendHTTPResponse(client, "Usage: /ir-nec/<bistring>/<nbits>", true);
    return false;
}

bool HomeControlServer::handleDigitalOutRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if (req.path[1] && req.path[2] && req.path[3] && !req.path[4])
    {
        int pin = atoi(req.path[1]);
        if (strcmp(req.path[2], "pulse-high") == 0)
        {
            int length = atoi(req.path[3]);
            digitalWrite(pin, HIGH);
            delay(length);
            digitalWrite(pin, LOW);
            sendHTTPResponse(client, "OK");
        }
        else
        {
            sendHTTPResponse(client, "Unknown mode", true);
        }

        return true;
    }

    sendHTTPResponse(client, "Usage: /digital-out/<pin>/<type>/<parameter>", true);
    return false;
}

bool HomeControlServer::handleDigitalInRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if (req.path[1] && !req.path[2])
    {
        int pin = atoi(req.path[1]);
        if (digitalRead(pin) == HIGH)
            sendHTTPResponse(client, "HIGH");
        else
            sendHTTPResponse(client, "LOW");
        return true;
    }

    sendHTTPResponse(client, "Usage: /digital-in/<pin>", true);
    return false;
}

bool HomeControlServer::handleAnalogInRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if (req.path[1] && !req.path[2])
    {
        int pin = atoi(req.path[1]);
        int result = analogRead(pin);
        sendHTTPResponse(client);
        client.print(pin);
        client.print(": ");
        client.println(result);
        return true;
    }

    sendHTTPResponse(client, "Usage: /analog-in/<pin>", true);
    return false;
}


bool HomeControlServer::handleRFBinaryRequest(EthernetClient& client, HCHTTPRequest& req)
{
    // TODO: Handle the case when RF is deactivated
    if (req.path[1] && !req.path[2])
    {
        bool invalid_chars = false;
        char* c = req.path[1];
        while (*c)
        {
            if (*c != '1' && *c != '0' && *c != 'f' && *c != 'F')
            {
                invalid_chars = true;
                break;
            }
            c += 1;
        }

        if (!invalid_chars)
        {
            noInterrupts();
            for (int i=0; i<RF_REPEAT; ++i)
            {
                c = req.path[1];
                while (*c)
                {
                    if (*c == '0')
                        send_0(rf_out_pin, RF_PULSE_LENGTH);
                    else if (*c == '1')
                        send_1(rf_out_pin, RF_PULSE_LENGTH);
                    else
                        send_F(rf_out_pin, RF_PULSE_LENGTH);
                    c += 1;
                }
                sync(rf_out_pin, RF_PULSE_LENGTH);
            }
            interrupts();

            sendHTTPResponse(client, "OK");
            return true;
        }
        else
        {
            sendHTTPResponse(client, "Invalid character: Only '0', '1', and 'F' allowed", true);
            return false;
        }
    }

    sendHTTPResponse(client, "Usage: /rf-binary/<bistring>", true);
    return false;
}


bool HomeControlServer::handleHTTPRequest(EthernetClient& client)
{
    char buffer[MAX_REQUEST_SIZE];

    if (readLine(client, buffer, MAX_REQUEST_SIZE-1) && readUntilEOH(client))
    {
        HCHTTPRequest req;
        req.parse(buffer);

        if (req.path[0])
        {
            if (strcmp(req.path[0], "ir-nec") == 0)
                return handleIRNECRequest(client, req);
            if (strcmp(req.path[0], "rf-binary") == 0)
                return handleRFBinaryRequest(client, req);
            if (strcmp(req.path[0], "digital-out") == 0)
                return handleDigitalOutRequest(client, req);
            if (strcmp(req.path[0], "digital-in") == 0)
                return handleDigitalInRequest(client, req);
            if (strcmp(req.path[0], "analog-in") == 0)
                return handleAnalogInRequest(client, req);
            return true;
        }

    }

    sendHTTPResponse(client, "Invalid request", true);
    return false;
}

void HomeControlServer::handleRequests()
{
    if (command_server)
    {
        EthernetClient client = command_server->available();
        while (client)
        {
            handleHTTPRequest(client);
            client.stop();
            client = command_server->available();
        }
    }
}

void HomeControlServer::handleEvents()
{
    if (event_server)
    {
        if (irrecv)
        {
            decode_results results;
            if (irrecv->decode(&results))
            {
                do
                {
                    if (results.decode_type == NEC)
                        event_server->print("ir-nec");
                    else if (results.decode_type == SONY)
                        event_server->print("ir-sony");
                    else if (results.decode_type == RC5)
                        event_server->print("ir-rc5");
                    else if (results.decode_type == RC6)
                        event_server->print("ir-rc6");
                    else
                        event_server->print("ir-raw");

                    // TODO: Print raw values properly
                    event_server->print(" ");
                    event_server->print(results.value, HEX);
                    event_server->print(" ");
                    event_server->println(results.bits, DEC);

                    irrecv->resume(); // Receive the next value
                }
                while (irrecv->decode(&results));
            }
            else
            {
                // This is needed to properly close
                // connections where the client has disconnected
                // Should probably be done less often...
                event_server->print("");
            }
        }
    }
}

// Implement new/delete ourselves since arduino does not provide them
void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete(void * ptr)
{
    free(ptr);
}
