#include "HCHTTPRequest.h"
#include <string.h>

void HCHTTPRequest::parse(char* request)
{
    char* url = 0;
    char* query_params = 0;

    method = http_version = 0;
    for (int i=0; i<MAX_PATH_ELEMENTS; ++i)
        path[i] = 0;
    for (int i=0; i<MAX_PARAMS; ++i)
        param_keys[i] = param_vals[i] = 0;

    int len = strlen(request);

    // Parse method
    int pos = 0;
    method = request;
    while (pos < len && request[pos] != ' ')
        pos += 1;
    request[pos] = 0;

    while (pos < len && (request[pos] == ' ' || request[pos] == 0))
        pos += 1;

    // Parse url
    url = request + pos;
    while (pos < len && request[pos] != ' ')
        pos += 1;
    request[pos] = 0;

    while (pos < len && (request[pos] == ' ' || request[pos] == 0))
        pos += 1;

    // Parse http version
    http_version = request + pos;

    if (url)
    {
        int len = strlen(url);
        int pos = 0;

        if (url[pos] == '/')
            pos += 1;

        for (int i=0; i<MAX_PATH_ELEMENTS-1; ++i)
        {
            path[i] = url + pos;
            while (pos < len && url[pos] != '/' && url[pos] != '?')
                pos += 1;
            char last = url[pos];
            url[pos] = 0;

            if (last == '?')
            {
                query_params = url + pos + 1;
                break;
            }
            else if (pos >= len)
            {
                query_params = url + pos;
                break;
            }
            else
            {
                pos += 1;
            }
        }
    }

    if (query_params)
    {
        int len = strlen(query_params);
        int pos = 0;

        for (int i=0; i<MAX_PARAMS-1; ++i)
        {
            if (pos >= len)
                break;

            param_keys[i] = query_params + pos;
            while (pos < len && query_params[pos] != '&' && query_params[pos] != '=')
                pos += 1;
            char last = query_params[pos];
            query_params[pos] = 0;
            pos += 1;

            if (last == '=')
            {
                param_vals[i] = query_params + pos;
                while (pos < len && query_params[pos] != '&')
                    pos += 1;
                query_params[pos] = 0;
                pos += 1;
            }
        }
    }
}

