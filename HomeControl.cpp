#include "HomeControl.h"
#include "HCTCPUtils.h"

#define MAX_REQUEST_SIZE  128

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

	radio->enable_send(send_pin, RF_SEND_REPEAT, RF_DEFAULT_PULSE_LENGTH);

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

unsigned int HomeControlServer::explode(char* data,
                                        unsigned int* timings,
                                        unsigned int max_len,
                                        char delimiter)
{
    unsigned int j = 0;
    timings[0] = 0;

    for(unsigned int i = 0; i < max_len; i ++)
    {
        if(data[i] == '\0')
            break;

        if(data[i] == delimiter)
        {
            j ++;

            if(j == max_len)
                return j;

            timings[j] = 0;
        }

        char c = data[i];
        timings[j] = 10 * timings[j] + atoi(&c);
    }

    return j + 1;
}

bool HomeControlServer::handleIRRawRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!irsend)
    {
        sendHTTPResponse(client, "IR is disabled");
        return false;
    }


    if(req.path[2]) irsend->enableIROut(atoi(req.path[2]));
    else irsend->enableIROut(IR_DEFAULT_KHZ);

    if (req.path[1])
    {
        unsigned int timings[RAWBUF];
        unsigned int len = explode(req.path[1], timings, RAWBUF);

        cli();
        for(unsigned int i = 0; i < len; i ++)
        {
            if((i % 2) == 0) irsend->mark(timings[i]);
            else irsend->space(timings[i]);
        }

        irsend->space(0); // Make sure IR LED is off.
        sei();

        sendHTTPResponse(client, "OK");
        return true;
    }

    sendHTTPResponse(client, "Usage: /ir-raw/<rawcode>[/<khz>]\n"
    						 "Example: /ir-raw/400.500.200, starting with IR mark", true);
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

bool HomeControlServer::handleRFTristateRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!radio)
    {
        sendHTTPResponse(client, "RF is disabled");
        return false;
    }

	// Set pulse length if given
	if(req.path[2]) radio->set_pulse_length(atoi(req.path[2]));
    else radio->set_pulse_length(RF_DEFAULT_PULSE_LENGTH);

    // Set number of repeats if given
    if(req.path[3]) radio->set_send_repeat(atoi(req.path[3]));
    else radio->set_send_repeat(RF_DEFAULT_SEND_REPEAT);

    if (req.path[1])
    {
    	if(radio->send_tristate(req.path[1]))
    	{
            sendHTTPResponse(client, "OK");
            return true;
        }
        else
        {
            sendHTTPResponse(client, "Invalid character: Only '0', '1', and 'F' allowed", true);
            return false;
        }
    }

    sendHTTPResponse(client, "Usage: /rf-tristate/<bistring>[/<pulse length>][/<repeat>]", true);
    return false;
}

bool HomeControlServer::handleRFRawRequest(EthernetClient& client, HCHTTPRequest& req)
{
    if(!radio)
    {
        sendHTTPResponse(client, "RF is disabled");
        return false;
    }

    // Set number of repeats if given
    if(req.path[2]) radio->set_send_repeat(atoi(req.path[2]));
    else radio->set_send_repeat(RF_DEFAULT_SEND_REPEAT);

    if (req.path[1])
    {
        unsigned long timings[HCRADIO_MAX_TIMINGS];
        unsigned int len_timings = explode(req.path[1], timings, HCRADIO_MAX_TIMINGS);

        radio->send_raw(timings, len_timings);
        sendHTTPResponse(client, "OK");
        return true;
    }

    sendHTTPResponse(client, "Usage: /rf-raw/<rawcode>[/<repeat>]\n"
                             "Example: /fr-raw/400.500.200, starting with high pulse", true);
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
        	if (strcmp(req.path[0], "ir-raw") == 0)
        		return handleIRRawRequest(client, req);
            if (strcmp(req.path[0], "ir-nec") == 0)
                return handleIRNECRequest(client, req);
            if (strcmp(req.path[0], "rf-tristate") == 0)
                return handleRFRawRequest(client, req);
            if (strcmp(req.path[0], "rf-raw") == 0)
                return handleRFTristateRequest(client, req);                
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
                    event_server->print("{\"type\": \"ir\", ");
                    event_server->print("\"decoding\": ");
                    if (results.decode_type == NEC)
                        event_server->print("\"nec\", ");
                    else if (results.decode_type == SONY)
                        event_server->print("\"sony\", ");
                    else if (results.decode_type == RC5)
                        event_server->print("\"rc5\", ");
                    else if (results.decode_type == RC6)
                        event_server->print("\"rc6\", ");
                    else
                        event_server->print("\"raw\", ");

                    event_server->print("\"hex\": \"");
                    event_server->print(results.value, HEX);
                    event_server->print("\", ");
                    event_server->print("\"dec\": \"");
                    event_server->print(results.bits, DEC);
                    event_server->print("\", ");

                    // Drop the first value, since this is the gap to the previous signal!
                    if(result->rawlen > 1)
                    {
                        event_server->print("\"timings\": [\"");
                        event_server->print(result.rawbuf[1] * USECPERTICK - MARK_EXCESS, DEC); // Mark.
                        event_server->print("\"");
                        for(unsigned int i = 2; i < result->rawlen; i ++)
                        {
                            unsigned int duration = 0;
                            if((i % 2) == 1) duration = results->rawbuf[i] * USECPERTICK - MARK_EXCESS; // Mark
                            else duration = results->rawbuf[i] * USECPERTICK + MARK_EXCESS; // Space

                            event_server->print(", \"");
                            event_server->print(duration, DEC);
                            event_server->print("\"");
                        }
                        event_server->print("]");
                    }

                    event_server->println("}");

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

        if(radio)
        {
        	HCRadioResult result;
        	if(radio->decode(&result))
        	{
        		// Send json description of the result
        		event_server->print("{\"type\": \"rf\", ");
        		if(result.len_timings == 0)
        			event_server->print("\"error\": \"unkown_decoding\"");
        		else
        		{
        			event_server->print("\"pulse_length\": \"" + String(result.pulse_length) + "\", ");
        			event_server->print("\"len_timings\": \"" + String(result.len_timings) + "\", ");
        			event_server->print("\"timings\": [\"");
        			event_server->print(result.timings[0], DEC);
        			event_server->print("\"");

        			for(unsigned int i = 0; i < result.len_timings; i ++)
        			{
        				event_server->print(", \"");
        				event_server->print(result.timings[i], DEC);
        				event_server->print("\"");
        			}

        			event_server->print("]");
        		}

        		event_server->println("}");
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
