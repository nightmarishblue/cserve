#include "http.h"
#include "main.h"
#include "opts.h"
#include "sock.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>

#include <unistd.h>
#include <sys/socket.h> // for a send or 2
#include <fcntl.h>
#include <errno.h>

const char* mthdstrs[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
const size_t mthdc = size(mthdstrs);
enum method methodfromstr(const char* mthdstr)
{
    for (int i = 0; i < mthdc; i++)
    {
        if (strncmp(mthdstr, mthdstrs[i], 8) == 0) // biggest is CONNECT
            return (enum method) i;
    }
    return -1;
}

const char* strfrommethod(enum method mthd)
{
    if (mthd < (enum method) 0 || mthd > PATCH) return NULL;
    return mthdstrs[mthd];
}

const struct status statuses[] = {
    {"OK", OK},
    {"Bad Request", BAD_REQUEST},
    {"Forbidden", FORBIDDEN},
    {"Not Found", NOT_FOUND},
    {"Length Required", LENGTH_REQUIRED},
    // things for our implementation limits
    {"Payload Too Large", PAYLOAD_TOO_LARGE},
    {"URI Too Long", URI_TOO_LONG},
    {"I'm a teapot", IM_A_TEAPOT}, // things we don't support
    {"Internal Server Error", INTERNAL_SERVER_ERROR},
    {"Not Implemented", NOT_IMPLEMENTED},
};
const size_t statusc = size(statuses);
const struct status* statusfromcode(enum code code)
{
    for (int i = 0; i < statusc; i++)
    {
        const struct status* curr = &statuses[i];
        if (curr->code == code)
            return curr;
    }
    return NULL;
}


const char* verstrs[] = { "1.0", "1.1" };
const size_t verc = size(verstrs);
// from a string like HTTP/X.X, deduce version
enum version versionfromstr(const char verstr[MAX_VERSION_LEN])
{
    if (strncmp(verstr, "HTTP/", 5) != 0)
        return -1;
    verstr += 5; // shift up to the number
    for (int i = 0; i < verc; i++)
    {
        if (strncmp(verstr, verstrs[i], 4) == 0) // version string can be no larger than 4 chars
            return (enum version) i;
    }
    return -1;
}

const char* strfromversion(enum version ver)
{
    if (ver < (enum version) 0 || ver > V1_1) return NULL;
    return verstrs[ver];
}

bool sendstatus(fd sock, enum version version, enum code code)
{
    const struct status* status = statusfromcode(code);
    const char* verstr = strfromversion(version);
    bool success = status && verstr;
    if (!verstr)
        verstr = strfromversion(DEFAULT_HTTP_VERSION);
    if (!status)
        status = statusfromcode(INTERNAL_SERVER_ERROR);
    if ((success = status && verstr))
        success = sockprintf(sock, "HTTP/%s %d %s\r\n", verstr, (int) status->code, status->desc) > 0;
    return success; // this is a lot of error checking I know
}

#define MAX_REQUEST_LEN 4096 // TODO

// read the first line out of a socket and figure out if it's a valid request
// populate the request object with its version and file identifier
// return the code to respond with (will be 4xx if bad)
// FIXME out parameters bad
enum code parsereq(SBUFF* sock, struct request* request)
{
    // read first few bytes - ensure they are GET /
    char mthdstr[MAX_METHOD_LEN];
    // reject invalid input - smallest method name is GET, 3 chars
    if (readuntilchar(sock, MAX_METHOD_LEN, mthdstr, ' ') < 3 || (request->method = methodfromstr(mthdstr)) == -1)
        return BAD_REQUEST;
    else if (request->method > HEAD)
    {
        fprintf(stderr, "oops... we don't have '%s' @u@\n", strfrommethod(request->method));
        return IM_A_TEAPOT;
    }

    // if the next character isn't a /, KILL
    if ((request->identifier[0] = sbuffgetc(sock)) != '/') // consume this so we don't have to read it again
        return BAD_REQUEST;

    // extract the path
    // read up to ' ', up to limit
    // you would think we should subtract 1 from this, but due to null-term nonsense, no need
    size_t pathlen = readuntilchar(sock, MAX_REQ_PATH_LEN, &request->identifier[1], ' ');
    if (pathlen == MAX_REQ_PATH_LEN) // returns len on failure
        return URI_TOO_LONG;
    
    // add index.html to the identifier if there's no file
    // BUG: /folder and /folder/ are both valid request targets I think
    // TODO also maybe consider dropping the leading / too if we're gonna hammer the data
    if (request->identifier[pathlen] == '/')
        strcpy(&request->identifier[pathlen + 1], INDEX);

    // extract the version string
    char verstr[MAX_VERSION_LEN];
    readuntilchar(sock, MAX_VERSION_LEN, verstr, '\r');
    enum version version = versionfromstr(verstr);
    if (version == -1)
        return BAD_REQUEST;
    request->version = version;
    
    // if the next character isn't a \n, KILL
    if (sbuffgetc(sock) != '\n')
        return BAD_REQUEST;

    // consume the remaining headers
    // TODO actually parse - at least the important ones like Content-Length
    const char* end = "\r\n\r\n";
    size_t j = 0;
    for (int i = 0; i < MAX_REQUEST_LEN; i++)
    {
        int c = sbuffgetc(sock);
        if (c == -1) {
            eprintf("Error reading from socket");
            return INTERNAL_SERVER_ERROR;
        }
        if (c == end[j])
        {
            if (end[++j] == '\0') break; // FIXME does not consume any request body
        }
        else
            j = 0;
    }

    return OK;
}

enum code errorcode(int errorno) {
    switch (errno)
    {
        // file errors
        case ENOENT:
            return NOT_FOUND;
        case EACCES:
            return FORBIDDEN;
        case ENAMETOOLONG:
            return URI_TOO_LONG;

        default:
            return INTERNAL_SERVER_ERROR;
    }
}

// open the file identified by a request, and store the resultant handle and response code in res
// return the size of the file in bytes, or -1 if an error occurred
off_t getfile(struct request* req, struct response* res)
{
    const char* filepath = req->identifier;
    res->file = openunder(options->srvdir, filepath, O_RDONLY);
    // the filepath is technically unneeded here
    if (res->file == -1)
    {
        eprintf("could not open file '%s'", filepath);
        res->code = errorcode(errno);
        return -1;
    }

    off_t size = filesize(res->file);
    if (size == -1)
        res->code = INTERNAL_SERVER_ERROR;
    return size; // FIXME i think it would be cleaner if this returned a response struct :/
}

#define EMPTY "Content-Length: 0\r\n\r\n" // GET responses are assumed to have a body by default

bool serve(SBUFF* sock)
{
    struct request req = { .method = GET, .version = DEFAULT_HTTP_VERSION, .identifier = "" };
    struct response res;
    res.code = parsereq(sock, &req);

    // TODO parse headers somewhere about here
    printf("%s %s HTTP/%s\n", strfrommethod(req.method), req.identifier, strfromversion(req.version));
    if (res.code != OK)
    {
        const struct status* stat = statusfromcode(res.code);
        if (stat)
            fprintf(stderr, "erroneous request: %s\n", stat->desc);
        sendstatus(sock->desc, req.version, res.code);
        return false; // do we really need to break the connection here?
    }

    // TODO probably throw errors if the file isn't "regular"
    switch (req.method)
    {
        case GET: {
            // open the file and send it
            off_t fsize = getfile(&req, &res);
            sendstatus(sock->desc, req.version, res.code);

            if (fsize != -1) // no need to check file, fsize tells us if it's open
            {
                sockprintf(sock->desc, "Content-Length: %ld\r\n\r\n", fsize);
                transmitfile(sock->desc, res.file, fsize); // TODO check return value and break connection if bad
            }
            else
            {
                send(sock->desc, EMPTY, sizeof(EMPTY) - 1, 0);
            }

            if (res.file != -1)
                close(res.file);
            
            break;
        };
        case HEAD: {
            // stat the file - don't open it
            // check if we can access the file
            if (!canread(options->srvdir, req.identifier + 1)) {
                eprintf("cannot access file at relative path %s", req.identifier + 1);
                sendstatus(sock->desc, req.version, errorcode(errno));
                send(sock->desc, EMPTY, sizeof(EMPTY) - 1, 0);
                break;
            }

            struct stat stats;
            if (!statfile(options->srvdir, req.identifier + 1, &stats)) {
                eprintf("could not stat relative path %s", req.identifier + 1);
                sendstatus(sock->desc, req.version, errorcode(errno)); // figure out why
                send(sock->desc, EMPTY, sizeof(EMPTY) - 1, 0);
                break; // not a fatal error
            }

            sendstatus(sock->desc, req.version, OK);
            sockprintf(sock->desc, "Content-Length: %ld\r\n\r\n", stats.st_size);

            break;
        };
        default: {
            fprintf(stderr, "Illegal state: unsupported method '%s'\n", strfrommethod(req.method));
            return false;
        };
    }

    return true;
}