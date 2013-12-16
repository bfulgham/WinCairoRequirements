// webserve.m -- a very simple web server using fork() to handle requests

/* compile with
cc -g -Wall -o webserve webserve.c
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define SOCK_CONST_DATA const void*
#else
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#define SOCK_CONST_DATA const char*
#include <windows.h>
#include <winsock2.h>
#include <Iprtrmib.h>
#include <Iphlpapi.h>
#include <io.h>
#define sleep Sleep
static DWORD getpid() {
   return GetCurrentProcessId();
}
char *strsep(char **stringp, const char *delim);
#endif

#include <CoreFoundation/CoreFoundation.h>

#define PORT_NUMBER 8080    // set to 80 to listen on the HTTP port
#define LINEBUFFER_SIZE 8192

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct clientConn
{
   CFWriteStreamRef writeStream;
   CFSocketNativeHandle listenSocket;
};

// HTTP request handling
// these are some of the common HTTP response codes
#define HTTP_OK         200
#define HTTP_NOT_FOUND  404
#define HTTP_ERROR      500


// return a string to the browser
#define returnString(httpResult, string, channel) \
   returnBuffer((httpResult), (string), (strlen(string)), (channel))

// return a character buffer (not necessarily zero-terminated) to the
// browser (runs in the child)
void returnBuffer (int httpResult, const char *content, 
                   int contentLength, CFWriteStreamRef commChannel)
{  
    static const CFStringRef headerFmt = CFSTR("HTTP/1.0 %d blah\r\n");
    static const CFStringRef contentType = CFSTR("Content-Type: text/html\r\n");
    static const CFStringRef contentLenFmt = CFSTR("Content-Length: %d\r\n");
    static const CFStringRef endHeader = CFSTR("\r\n");

    CFStringRef header = 0, contentLengthStr = 0;

    header = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, headerFmt, httpResult);
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(header,CFStringGetFastestEncoding(header)),CFStringGetLength(header));
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(contentType,CFStringGetFastestEncoding(contentType)),CFStringGetLength(contentType));

    contentLengthStr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, contentLenFmt, contentLength);
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(contentLengthStr,CFStringGetFastestEncoding(contentLengthStr)),CFStringGetLength(contentLengthStr));
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(endHeader,CFStringGetFastestEncoding(endHeader)),CFStringGetLength(endHeader));
   
    CFWriteStreamWrite(commChannel, (const UInt8*)content, contentLength);
} // returnBuffer


// stream back to the browser numbers being counted, with a pause
// between them.  The user should see the numbers appear every couple
// of seconds (runs in the child)
void returnNumbers (int number, CFWriteStreamRef commChannel)
{
    static const CFStringRef headerFmt = CFSTR("HTTP/1.0 %d blah\r\n");
    static const CFStringRef contentType = CFSTR("Content-Type: text/html\r\n");
    static const CFStringRef endHeader = CFSTR("\r\n");
    static const CFStringRef dataHeaderFmt = CFSTR("<html><head><title>Numbers</title></head><body><h2>The numbers from %d to %d</h2>\n");
    static const CFStringRef numberFmt = CFSTR("%d\n");
    static const CFStringRef done = CFSTR("<hr>Done\n</body></html>\r\n");

    int min = MIN (number, 1);
    int max = MAX (number, 1);

    CFStringRef header = 0, dataHeader = 0;

    header = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, headerFmt, HTTP_OK);
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(header,CFStringGetFastestEncoding(header)),CFStringGetLength(header));
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(contentType,CFStringGetFastestEncoding(contentType)),CFStringGetLength(contentType));
    // no content length, dynamic
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(endHeader,CFStringGetFastestEncoding(endHeader)),CFStringGetLength(endHeader));

    dataHeader = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, dataHeaderFmt, min, max);
    CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(dataHeader,CFStringGetFastestEncoding(dataHeader)),CFStringGetLength(dataHeader));

    for (int i = min; i <= max; i++) {
        sleep (2);
        CFStringRef numberStr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, numberFmt, i);
        CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(numberStr,CFStringGetFastestEncoding(numberStr)),CFStringGetLength(numberStr));
    }

   CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(done,CFStringGetFastestEncoding(done)),CFStringGetLength(done));
} // returnNumbers


// return a file from the file system, relative to where the webserve
// is running.  Note that this doesn't look for any nasty characters
// like '..', so this function is a pretty big security hole
// (runs in the child)
void returnFile (const char *filename, CFWriteStreamRef commChannel)
{
    static const CFStringRef headerFmt = CFSTR("HTTP/1.0 %d blah\r\n");
    char lineBuffer[LINEBUFFER_SIZE];
    const char *mimetype = NULL;

    // try to guess the mime type.  IE assumes all non-graphic files
    // are HTML
    if (strstr(filename, ".m") != NULL) {
        mimetype = "text/plain";
    } else if (strstr(filename, ".h") != NULL) {
        mimetype = "text/plain";
    } else if (strstr(filename, ".txt") != NULL) {
        mimetype = "text/plain";
    } else if (strstr(filename, ".tgz") != NULL) {
        mimetype = "application/x-compressed";
    } else if (strstr(filename, ".html") != NULL) {
        mimetype = "text/html";
    } else if (strstr(filename, ".htm") != NULL) {
        mimetype = "text/html";
    } else if (strstr(filename, ".mp3") != NULL) {
        mimetype = "audio/mpeg";
    }

    FILE *file;
    file = fopen (filename, "r");

    if (file == NULL) {
        returnString (HTTP_NOT_FOUND, 
                      "could not find your file.  Sorry\n.", 
                      commChannel);
    } else {
        CFStringRef header = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, headerFmt, HTTP_OK);
        CFWriteStreamWrite(commChannel,(const UInt8*)CFStringGetCStringPtr(header,CFStringGetFastestEncoding(header)),CFStringGetLength(header));
        if (mimetype != NULL) {
            sprintf (lineBuffer, "Content-Type: %s\r\n", mimetype);
            CFWriteStreamWrite(commChannel, (const UInt8*)lineBuffer, strlen(lineBuffer));
       }
       CFWriteStreamWrite(commChannel, (const UInt8*)"\r\n", 2); // no content length, dynamic

#define BUFFER_SIZE (8 * 1024)
        char *buffer[BUFFER_SIZE];
        int result;

        while ((result = fread (buffer, 1, BUFFER_SIZE, file)) > 0) {
            CFWriteStreamWrite (commChannel, (const  UInt8*)buffer, result);
        }
#undef BUFFER_SIZE
    }
}


// using the method and the request (the path part of the url),
// generate the data for the user and send it back. (runs in the
// child)
void handleRequest (const char *method, 
                    const char *originalRequest, CFWriteStreamRef commChannel)
{
    char* request = strdup (originalRequest);

    // we'll use strsep to split this
    if (strcmp(method, "GET") != 0) {
        returnString (HTTP_ERROR, 
                      "only GETs are supported", commChannel);
        goto bailout;
    }
    
    char *chunk, *nextString;
    nextString = request;

    chunk = strsep (&nextString, "/");
    // urls start with slashes, so chunk is ""

    chunk = strsep (&nextString, "/");  // the leading part of the url

    if (strcmp(chunk, "numbers") == 0) {
        int number;

        // url of the form /numbers/5 to print numbers from 1 to 5
        chunk = strsep (&nextString, "/");
        number = atoi(chunk);
        returnNumbers (number, commChannel);

    } else if (strcmp(chunk, "file") == 0) {
        chunk = strsep (&nextString, ""); // get the rest of the string
        returnFile (chunk, commChannel);
    } else {
        returnString (HTTP_NOT_FOUND, 
                      "could not handle your request.  Sorry\n.",
                      commChannel);
    }

bailout:
    fprintf (stderr, "child %ld handled request '%s'\n", 
             (long)getpid(), originalRequest);

    free (request);
}


// read the request from the browser, pull apart the elements of the
// request, and then dispatch it.  (runs in the child)
void dispatchRequest (CFReadStreamRef readStream, CFStreamEventType eventType, void* clientCallBackInfo)
{
    char lineBuffer[LINEBUFFER_SIZE];
   
    struct clientConn* pClientConn = ((struct clientConn*)clientCallBackInfo);
   
    CFWriteStreamRef writeStream = pClientConn->writeStream;
    CFSocketNativeHandle listenSocket = pClientConn->listenSocket;

    CFIndex bytesRead = CFReadStreamRead(readStream, (UInt8*)lineBuffer, LINEBUFFER_SIZE);
   
    // this is pretty lame in that it only reads the first line and 
    // assumes that's the request, subsequently ignoring any headers
    // that might be sent.

   if (bytesRead > 0) {
        // ok, now figure out what they wanted
        char *requestElements[3], *nextString, *chunk;
        int i = 0;
        nextString = lineBuffer;
        while (i < 3 && (chunk = strsep (&nextString, " "))) {
            requestElements[i] = chunk;
            i++;
        }
        if (i != 3) {
            returnString (HTTP_ERROR, "malformed request", writeStream);
            goto bailout;
        }
        
        handleRequest (requestElements[0], requestElements[1], writeStream);
    } else
        fprintf (stderr, "read an empty request.  exiting\n");

bailout:
    fflush (stderr);
    if (writeStream) {
        CFWriteStreamClose(writeStream);
        //CFRelease(writeStream);
    }
    if (readStream) {
        CFReadStreamUnscheduleFromRunLoop(readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFReadStreamSetClient(readStream, kCFStreamEventNone, NULL, NULL);
        CFReadStreamClose(readStream);
        CFRelease(readStream);
    }
   close(listenSocket);
   free(pClientConn);
} // dispatchRequest



// sit blocking on accept until either it breaks out with a signal
// (like SIGCHLD) or a new connection comes in.  If it's a new
// connection, fork off a child to process the request
static void acceptRequest(CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
    struct sockaddr_in clientAddr;
    int port = 0;
    const char* addr = 0;
    Boolean didSet = 0;
   
    CFSocketNativeHandle listenSocket = *(CFSocketNativeHandle *)data;
   
    if (kCFSocketAcceptCallBack != type) {
        fprintf (stderr, "accept called with incorrect type (%d).  error: %d/%s\n",
                 type, errno, strerror(errno));
        return;
    }
   
    memcpy (&clientAddr, (const void*)address, sizeof(clientAddr));
   
    port = clientAddr.sin_port;
    addr = inet_ntoa(clientAddr.sin_addr);
    
    fprintf (stderr, "Accepted connection from %s:%d\n", addr, port);

    // child sends output to stderr, so make sure it's drained before moving on
    fflush (stderr); 
    
    CFReadStreamRef readStream = 0;
    CFWriteStreamRef writeStream = 0;
    CFStreamCreatePairWithSocket(kCFAllocatorDefault, listenSocket, &readStream, &writeStream);
    if (!readStream || !writeStream) {
         fprintf (stderr, "Failed to build I/O streams.  Error: %d/%s\n",
                  errno, strerror(errno));
         close(listenSocket);
         return;
    }
   
    CFStreamClientContext clientContext;
    memset(&clientContext, 0x00, sizeof(clientContext));

    clientContext.info = malloc(sizeof(clientConn));
    ((struct clientConn*)clientContext.info)->writeStream = writeStream;
    ((struct clientConn*)clientContext.info)->listenSocket = listenSocket;
   
    didSet = CFReadStreamSetClient(readStream, kCFStreamEventOpenCompleted|kCFStreamEventHasBytesAvailable|kCFStreamEventEndEncountered, (CFReadStreamClientCallBack)&dispatchRequest, &clientContext);
       
    if (didSet)
        CFReadStreamScheduleWithRunLoop(readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
   
    CFReadStreamOpen(readStream);
    CFWriteStreamOpen(writeStream);
}



// this is 100% stolen from chatterserver.m
// start listening on our server port (runs in parent)
CFSocketRef startListening ()
{
    int result = 0;
    int yes = 1;

    CFDataRef address4 = 0;
   
    CFSocketContext socketContext = {0, 0, NULL, NULL, NULL};
    CFSocketRef listenOn = CFSocketCreate (kCFAllocatorDefault, PF_INET, SOCK_STREAM, IPPROTO_TCP, kCFSocketAcceptCallBack, (CFSocketCallBack)&acceptRequest, &socketContext);   
    if (!listenOn) {
        fprintf (stderr, "could not make a socket.  error: %d / %s\n",
                 errno, strerror(errno));
        return 0;
    }

    result = setsockopt (CFSocketGetNative (listenOn), SOL_SOCKET, SO_REUSEADDR, (SOCK_CONST_DATA)&yes, sizeof(yes));
    if (result == -1) {
        fprintf (stderr, 
                 "could not setsockopt to reuse address. %d / %s\n",
                 errno, strerror(errno));
        if (listenOn) CFRelease(listenOn);
        listenOn = 0;
        return 0;
    }

    // bind to an address and port
    struct sockaddr_in address;
#if !defined(__WIN32__)
    address.sin_len = sizeof (struct sockaddr_in);
#endif
    address.sin_family = AF_INET;
    address.sin_port = htons (PORT_NUMBER);
    address.sin_addr.s_addr = htonl (INADDR_ANY);
    memset (address.sin_zero, 0, sizeof(address.sin_zero));
   
    address4 = CFDataCreateWithBytesNoCopy (kCFAllocatorDefault, (const UInt8*)&address, sizeof(address), kCFAllocatorDefault);
   
    if (kCFSocketSuccess != CFSocketSetAddress (listenOn, address4)) {
        fprintf (stderr, "could not bind socket.  error: %d / %s\n",
                 errno, strerror(errno));
        if (listenOn) CFRelease(listenOn);
        listenOn = 0;
        return 0;
    }
    
    return listenOn;
}


int main (int argc, char *argv[])
{
    CFSocketRef listenSocket = 0;
    CFRunLoopSourceRef source = 0;

    listenSocket = startListening ();
    if (!listenSocket)
        return -1;
   
    source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, listenSocket, 0);
   
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CFRelease(source);
   
    CFRunLoopRun();

    return (EXIT_SUCCESS);
}

#if defined(__WIN32__)
/*
 * From the BSD sources.  Why isn't this in MSVCRTL?
 */
char *strsep(char **stringp, const char *delim) {
        char *origin, *p;
        const char *pp;

        if (stringp || !*stringp || !delim || !*delim) {
                return NULL;
        }

        for(origin = p = *stringp; *p; p++) {
                for(pp = delim; *pp; pp++) {
                        if (*pp == *p) {
                                *p = 0;
                                *stringp = *++p ? p : NULL;
                        }
                }
        }

        return origin;
}
#endif
