// http protocol parsing and stuff
#ifndef _HTTP_H
#define _HTTP_H

#include "inet.h"

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

#define MAX_VERSION_LEN 9 // array size needed for "HTTP/X.X"
enum version
{
    // _0_9, // no one will get mad if we don't support 0.9
    V1_0,
    V1_1,
};

#define DEFAULT_HTTP_VERSION V1_1 // the default version to assume when we can't determine the client's

// codes for the statuses, by name
enum code
{
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    LENGTH_REQUIRED = 411, // the client didn't supply Content-Length
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    IM_A_TEAPOT = 418, // things we should comply with but don't
    INTERNAL_SERVER_ERROR = 500, // ¯\_(ツ)_/¯
    NOT_IMPLEMENTED = 501, // things we don't have to comply with (and don't)
};

// struct representing the response type, like 200 OK
struct status
{
    const char* desc;
    enum code code;
};

#define MAX_REQ_PATH 256
struct request
{
    enum method method; // what method this request is
    enum version version; // the request's version
    char identifier[256]; // the path to the file
};

struct response
{
    enum code code;
    fd file; // handle of the file to serve
};

// with a string of the form "HTTP/x.x" (9 chars), return its version
// or -1 on invalid
enum version versionfromstr(const char verstr[MAX_VERSION_LEN]);

// read from a given socket and answer 1 HTTP request
// return false if connection should be broken
bool serve(fd sock);
#endif