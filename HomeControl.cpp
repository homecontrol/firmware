#include "HomeControl.h"
#include "HCTCPUtils.h"

HomeControlServer::HomeControlServer()
: command_server(NULL), event_server(NULL),
  irsend(NULL), irrecv(NULL), radio(NULL)
{}

HomeControlServer::~HomeControlServer()
{
    delete irrecv;
    delete irsend;
    delete event_server;
    delete command_server;
    delete radio;
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

void HomeControlServer::enableRFOut(int send_pin, int status_pin)
{
    if(!radio)
        radio = new HCRadio();

    radio->enable_send(send_pin, RF_DEFAULT_SEND_REPEAT, RF_DEFAULT_PULSE_WIDTH);

    if(status_pin != -1)
        radio->enable_status(status_pin);
}

void HomeControlServer::enableRFIn()
{
    if(!radio)
        radio = new HCRadio();

    // Interrupt 0 is attached to PIN 2,
    // see http://arduino.cc/it/Reference/AttachInterrupt
    radio->enable_receive(RF_RECEIVER_IRQ);
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

bool HomeControlServer::handleMemoryRequest(EthernetClient& client, HCHTTPRequest& req)
{
    sendHTTPResponse(client, NULL, true);

    client.print(freeMemory());
    write_P(client, PSTR(" bytes free\n"));

    return false;
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
        sendHTTPResponseOK(client);
        return true;
    }

    sendHTTPResponse_P(client, PSTR("Usage: /ir-nec/<bistring>/<nbits>"), true);
    return false;
}

bool HomeControlServer::handleIRRawRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!irsend)
    {
        sendHTTPResponse_P(client, PSTR("IR is disabled"));
        return false;
    }

    if(req.path[2]) irsend->enableIROut(atoi(req.path[2]));
    else irsend->enableIROut(IR_DEFAULT_KHZ);

    if (req.path[1])
    {
        char* c = req.path[1];
        int t = 0, i = 0;

        cli();
        noInterrupts();

        while(true)
        {
            if(*c == '.' || *c == 0)
            {
                if((i % 2) == 0) irsend->mark(t);
                else irsend->space(t);

                if(*c == 0)
                    break;

                i ++;
                t = 0;
            }
            else
            {
                if(*c < '0' || *c > '9')
                {
                    t = -1;
                    break;
                }

                t = 10 * t + (*c - '0');
            }

            c ++;
        }
        irsend->space(0); // Make sure IR LED is off.

        sei();
        interrupts();

        if (irrecv)
            irrecv->enableIRIn();

        if(t < 0)
        {
            sendHTTPResponse_P(client, PSTR("Invalid character"), true);
            return false;
        }

        sendHTTPResponseOK(client);
        return true;
    }

    sendHTTPResponse_P(client, PSTR("Usage: /ir-raw/<rawcode>[/<khz>]\n"
            "Example: /ir-raw/400.500.200, starting with IR mark"), true);

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
            sendHTTPResponseOK(client);
        }
        else
        {
            sendHTTPResponse_P(client, PSTR("Unknown mode"), true);
        }

        return true;
    }

    sendHTTPResponse_P(client,
            PSTR("Usage: /digital-out/<pin>/<type>/<parameter>"), true);
    return false;
}

bool HomeControlServer::handleDigitalInRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if (req.path[1] && !req.path[2])
    {
        int pin = atoi(req.path[1]);
        if (digitalRead(pin) == HIGH)
            sendHTTPResponse_P(client, PSTR("HIGH"));
        else
            sendHTTPResponse_P(client, PSTR("LOW"));
        return true;
    }

    sendHTTPResponse_P(client, PSTR("Usage: /digital-in/<pin>"));
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

    sendHTTPResponse_P(client, PSTR("Usage: /analog-in/<pin>"), true);
    return false;
}

bool HomeControlServer::handleRFTristateRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!radio)
    {
        sendHTTPResponse_P(client, PSTR("RF is disabled"), true);
        return false;
    }

    // Set pulse length if given
    if(req.path[2]) radio->set_pulse_length(atoi(req.path[2]));
    else radio->set_pulse_length(RF_DEFAULT_PULSE_WIDTH);

    // Set number of repeats if given
    if(req.path[3]) radio->set_send_repeat(atoi(req.path[3]));
    else radio->set_send_repeat(RF_DEFAULT_SEND_REPEAT);

    if (req.path[1])
    {
        if(radio->send_tristate(req.path[1]))
        {
            sendHTTPResponseOK(client);
            return true;
        }
        else
        {
            sendHTTPResponse_P(client,
                    PSTR("Invalid character: Only '0', '1', and 'F' allowed"), true);
            return false;
        }
    }

    sendHTTPResponse_P(client,
            PSTR("Usage: /rf-tristate/<bistring>[/<pulse length>][/<repeat>]"), true);
    return false;
}

bool HomeControlServer::handleRFRawRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!radio)
    {
        sendHTTPResponse_P(client, PSTR("RF is disabled"), true);
        return false;
    }

    // Set number of repeats if given
    if(req.path[2]) radio->set_send_repeat(atoi(req.path[2]));
    else radio->set_send_repeat(RF_DEFAULT_SEND_REPEAT);

    if (req.path[1])
    {
        if(!radio->send_raw(req.path[1]))
        {
            sendHTTPResponse_P(client, PSTR("Invalid character"), true);
            return false;
        }

        sendHTTPResponseOK(client);
        return true;
    }

    sendHTTPResponse_P(client, PSTR("Usage: /rf-raw/<rawcode>[/<repeat>]\n"
            "Example: /rf-raw/400.500.200, starting with high pulse"), true);
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
            if (strcmp(req.path[0], "mem") == 0)
                return handleMemoryRequest(client, req);
            if (strcmp(req.path[0], "ir-raw") == 0)
                return handleIRRawRequest(client, req);
            if (strcmp(req.path[0], "ir-nec") == 0)
                return handleIRNECRequest(client, req);
            if (strcmp(req.path[0], "rf-tristate") == 0)
                return handleRFTristateRequest(client, req);
            if (strcmp(req.path[0], "rf-raw") == 0)
                return handleRFRawRequest(client, req);
            if (strcmp(req.path[0], "digital-out") == 0)
                return handleDigitalOutRequest(client, req);
            if (strcmp(req.path[0], "digital-in") == 0)
                return handleDigitalInRequest(client, req);
            if (strcmp(req.path[0], "analog-in") == 0)
                return handleAnalogInRequest(client, req);
            return true;
        }

    }

    sendHTTPResponse_P(client, PSTR("Invalid request"), true);
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
            decode_results result;
            if (irrecv->decode(&result))
            {
                do
                {
                    // TODO: Introduce PSTR to free some more memory.

                    write_P(*event_server, PSTR("{\"type\": \"ir\", "));
                    write_P(*event_server, PSTR("\"decoding\": "));
                    if (result.decode_type == NEC)
                        write_P(*event_server, PSTR("\"nec\", "));
                    else if (result.decode_type == SONY)
                        write_P(*event_server, PSTR("\"sony\", "));
                    else if (result.decode_type == RC5)
                        write_P(*event_server, PSTR("\"rc5\", "));
                    else if (result.decode_type == RC6)
                        write_P(*event_server, PSTR("\"rc6\", "));
                    else
                        write_P(*event_server, PSTR("\"raw\", "));

                    write_P(*event_server, PSTR("\"hex\": \""));
                    event_server->print(result.value, HEX);
                    write_P(*event_server, PSTR("\", "));
                    write_P(*event_server, PSTR("\"length\": \""));
                    event_server->print(result.bits, DEC);
                    write_P(*event_server, PSTR("\", "));

                    // Note that the first value is a space!
                    if(result.rawlen > 1)
                    {
                        write_P(*event_server, PSTR("\"timings\": [\""));
                        event_server->print(result.rawbuf[0] * USECPERTICK - MARK_EXCESS, DEC); // Mark.
                        write_P(*event_server, PSTR("\""));
                        for(int i = 1; i < result.rawlen; i ++)
                        {
                            unsigned int duration = 0;
                            if((i % 2) == 1) duration = result.rawbuf[i] * USECPERTICK - MARK_EXCESS; // Mark
                            else duration = result.rawbuf[i] * USECPERTICK + MARK_EXCESS; // Space

                            write_P(*event_server, PSTR(", \""));
                            event_server->print(duration, DEC);
                            write_P(*event_server, PSTR("\""));
                        }
                        write_P(*event_server, PSTR("]"));
                    }

                    writeln_P(*event_server, PSTR("}"));

                    irrecv->resume(); // Receive the next value
                }
                while (irrecv->decode(&result));
            }
            else
            {
                // This is needed to properly close
                // connections where the client has disconnected
                // Should probably be done less often...
                write_P(*event_server, PSTR(""));
            }
        }

        if(radio)
        {
            HCRadioResult result;
            if(radio->decode(&result))
            {
                // Send json description of the result
                write_P(*event_server, PSTR("{\"type\": \"rf\", "));
                if(result.len_timings == 0)
                    write_P(*event_server, PSTR("\"error\": \"unkown_decoding\""));
                else
                {
                    write_P(*event_server, PSTR("\"pulse_length\": \""));
                    event_server->print(result.pulse_length);
                    write_P(*event_server, PSTR("\", "));
                    write_P(*event_server, PSTR("\"len_timings\": \""));
                    event_server->print(result.len_timings);
                    write_P(*event_server, PSTR("\", "));
                    write_P(*event_server, PSTR("\"timings\": [\""));

                    if(result.len_timings > 0)
                    {
                        event_server->print(result.timings[0], DEC);
                        write_P(*event_server, PSTR("\""));

                        for(unsigned int i = 1; i < result.len_timings; i ++)
                        {
                            write_P(*event_server, PSTR(", \""));
                            event_server->print(result.timings[i], DEC);
                            write_P(*event_server, PSTR("\""));
                        }
                    }

                    write_P(*event_server, PSTR("]"));
                }

                writeln_P(*event_server, PSTR("}"));
            }
            else
            {
                // This is needed to properly close
                // connections where the client has disconnected
                // Should probably be done less often...
                write_P(*event_server, PSTR(""));
            }
        }
    }
}

#if !defined(ARDUINO) || ARDUINO < 100

// Implement new/delete ourselves since arduino does not provide them
void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete(void * ptr)
{
    free(ptr);
}

#endif
