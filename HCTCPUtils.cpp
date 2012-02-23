#include "HCTCPUtils.h"

void sendHTTPResponse(EthernetClient& client, const char* message, int error)
{
    if (client.connected())
    {
        if (error)
            write_P(client, PSTR("HTTP/1.0 500 Error\r\n"));
        else
            write_P(client, PSTR("HTTP/1.0 200 OK\r\n"));

        write_P(client, PSTR("Content-Type: text/plain\r\n\r\n"));

        if (message)
            client.println(message);
    }
}

void sendHTTPResponse_P(EthernetClient& client, const char* message, int error)
{
    sendHTTPResponse(client, NULL, error);

    if(client.connected() && message)
    {
        write_P(client, message);
        write_P(client, PSTR("\n"));
    }
}

void sendHTTPResponseOK(EthernetClient &client)
{
    sendHTTPResponse_P(client, PSTR("OK"));
}

bool readLine(EthernetClient& client, char* buffer, int buffer_size)
{
    int pos = 0;
    while (client.connected())
    {
        if (pos >= buffer_size)
            return false;

        if (client.available())
        {
            buffer[pos] = client.read();
            if (pos >= 1 && buffer[pos-1] == '\r' && buffer[pos] == '\n')
            {
                buffer[pos-1] = 0;
                return true;
            }
            pos += 1;
        }
    }
    return false;
}

bool skipLine(EthernetClient& client, bool& empty)
{
    int pos = 0;
    char last_char = 0;
    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            if (last_char == '\r' && c == '\n')
            {
                empty = (pos == 1);
                return true;
            }
            last_char = c;
            pos += 1;
        }
    }
    return false;
}

bool readUntilEOH(EthernetClient& client)
{
    bool empty = false;
    while (true)
    {
        if (!skipLine(client, empty))
            return false;
        if (empty)
            return true;
    }
}

void write_P(Stream& s, const char *str)
{
    uint8_t val;
    while(true)
    {
        val = pgm_read_byte(str);
        if (!val) break;
        s.write(val);
        str ++;
    }
}

void writeln_P(Stream& s, const char *str)
{
    write_P(s, str);
    write_P(s, PSTR("\n"));
}

void write_P(Print& s, const char *str)
{
    uint8_t val;
    while(true)
    {
        val = pgm_read_byte(str);
        if (!val) break;
        s.write(val);
        str ++;
    }
}

void writeln_P(Print& s, const char *str)
{
    write_P(s, str);
    write_P(s, PSTR("\n"));
}
