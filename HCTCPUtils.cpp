#include "HCTCPUtils.h"

void sendHTTPResponse(EthernetClient& client, const char* message, int error)
{
    if (client.connected())
    {
        if (error)
            client.println("HTTP/1.0 500 Error");
        else
            client.println("HTTP/1.0 200 OK");

        client.println("Content-Type: text/plain");
        client.println();
        if (message)
            client.println(message);
    }
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
