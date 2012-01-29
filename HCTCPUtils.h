#ifndef HCTCPUTILS_H_
#define HCTCPUTILS_H_

#include <Ethernet.h>
#if !defined(ARDUINO) || ARDUINO < 100
#define EthernetClient Client
#endif

void sendHTTPResponse(EthernetClient& client, const char* message = 0, int error = false);
bool readLine(EthernetClient& client, char* buffer, int buffer_size);
bool skipLine(EthernetClient& client, bool& empty);
bool readUntilEOH(EthernetClient& client);

#endif
