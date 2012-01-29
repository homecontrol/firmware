#ifndef HCTCPREQUEST_H_
#define HCTCPREQUEST_H_

#define MAX_PATH_ELEMENTS 16
#define MAX_PARAMS        16

class HCHTTPRequest
{
    public:
        void parse(char* request);

        char* method;
        char* http_version;
        char* path[MAX_PATH_ELEMENTS];
        char* param_keys[MAX_PARAMS];
        char* param_vals[MAX_PARAMS];
};

#endif
