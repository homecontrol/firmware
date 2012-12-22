#ifndef HCTCPUTILS_H_
#define HCTCPUTILS_H_

#include <Ethernet.h>
#if !defined(ARDUINO) || ARDUINO < 100
#define EthernetClient Client
#endif

#include <avr/pgmspace.h>

void sendHTTPResponse(EthernetClient& client, const char* message = 0, int error = false);
void sendHTTPResponse_P(EthernetClient& client, const char* message = 0, int error = false);
void sendHTTPResponseOK(EthernetClient& client);
bool readLine(EthernetClient& client, char* buffer, int buffer_size);
bool skipLine(EthernetClient& client, bool& empty);
bool readUntilEOH(EthernetClient& client);
void write_P(Stream& s, const char *str);
void writeln_P(Stream& s, const char *str);
void write_P(Print& s, const char *str);
void writeln_P(Print& s, const char *str);

#endif
