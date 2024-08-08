// http protocol parsing and stuff
#ifndef _HTTP_H
#define _HTTP_H

#include "strio.h"

#include <stdbool.h>

#define MAX_METHOD_LEN 8 // array size needed to represent the name of all methods
// request methods
enum method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
};

enum version
{
    // _0_9, // no one will get mad if we don't support 0.9
    _1_0,
    _1_1,
};

#define MAX_CODE_LEN 22 // array size needed to represent all code names
// codes for the statuses, by name
enum code
{
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    LENGTH_REQUIRED = 411,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    IM_A_TEAPOT = 418,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501
};

// struct representing the response type, like 200 OK
struct status
{
    char desc[24];
    enum code code;
};

struct request
{
    enum method method; // what method this request is
    enum version version; // the request's version
    char* identifier; // the path to the file
};

// with a string of the form "HTTP/x.x" (9 chars), return its version
// or -1 on invalid
enum version versionfromstr(const char verstr[9]);

bool stopafterstring(struct buffer* buf, void* _str);
//stop after finding \r\n
bool stopafterline(struct buffer* buf, void*_);
void serve(fd sock);
#endif