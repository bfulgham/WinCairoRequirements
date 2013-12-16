/*
 * Copyright (c) 2009 Brent Fulgham.  All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the BSD-style license (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 */
/*****
 ** NOTE: You must have the Bonjour SDK installed to build/run this example.
 *****/
/*
    File:  DNSServiceMetaQuery.c
    
    Contains:  Sample code which shows how to discover all Bonjour service types
    being advertised on the local network.
    
    Copyright:  (c) Copyright 2004-2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
      
    Change History (most recent first):
        1.2   May 13, 2005
        1.1   December 8, 2004
        1.0   May 28, 2004
*/

#if defined(WIN32)
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <dns_sd.h>
#include <Iprtrmib.h>
#include <Iphlpapi.h>

static char* if_indextoname (DWORD ifIndex, char* nameBuff);

#define usleep(X)   Sleep(((X)+999)/1000)
#define ns_t_ptr    12
#define ns_c_in 1
#else
#include <dns_sd.h>
#include <arpa/nameser.h>
#include <sys/socket.h>
#include <net/if.h>
#include <assert.h>
#include <unistd.h>
#endif

#include <CoreFoundation/CoreFoundation.h>

#define MAX_DOMAIN_LABEL 63
#define MAX_DOMAIN_NAME 255
#define kServiceMetaQueryName  "_services._dns-sd._udp.local."

typedef struct { unsigned char c[ 64]; } domainlabel;      // One label: length byte and up to 63 characters.
typedef struct { unsigned char c[256]; } domainname;       // Up to 255 bytes of length-prefixed domainlabels.

typedef struct MyDNSServiceState {
    DNSServiceRef       service;
    CFRunLoopSourceRef  source;
    CFSocketRef         socket;
} MyDNSServiceState;


/* MyConvertDomainLabelToCString() converts a DNS label into a C string.
A DNS label string is formatted like "\003com".  The converted string
would look like "com." */

static char*
MyConvertDomainLabelToCString(const domainlabel *const label, char *ptr)
{
    const unsigned char *      src = label->c;      // Domain label we're reading.
    const unsigned char        len = *src++;        // Read length of this (non-null) label.
    const unsigned char *const end = src + len;     // Work out where the label ends.
    
    assert(label != NULL);
    assert(ptr   != NULL);
    
    if (len > MAX_DOMAIN_LABEL) return(NULL);       // If illegal label, abort.
    while (src < end) {                             // While we have characters in the label.
        unsigned char c = *src++;
        if (c == '.' || c == '\\')                  // If character is a dot or the escape character
            *ptr++ = '\\';                          // Output escape character.
        else if (c <= ' ') {                        // If non-printing ascii, output decimal escape sequence.
            *ptr++ = '\\';
            *ptr++ = (char)  ('0' + (c / 100)     );
            *ptr++ = (char)  ('0' + (c /  10) % 10);
            c      = (unsigned char)('0' + (c      ) % 10);
        }
        *ptr++ = (char)c;                           // Copy the character.
    }
    *ptr = 0;                                       // Null-terminate the string
    return(ptr);                                    // and return.
}


/* MyConvertDomainNameToCString() converts a DNS name string into a C string.
A DNS name string is formated like "\003www\005apple\003com\0".  The converted
string would look like "www.apple.com".  If the DNS name contains a period "." or
a backslash "\", then those characters will be escaped with backslash characters,
as in "\." and "\\".  Note: To guarantee that there will be no possible overrun,
"ptr" must be at least kDNSServiceMaxDomainName (1005 bytes) */

static char*
MyConvertDomainNameToCString(const domainname *const name, char *ptr)
{
    const unsigned char *src         = name->c;                     // Domain name we're reading.
    const unsigned char *const max   = name->c + MAX_DOMAIN_NAME;   // Maximum that's valid.

    assert(name != NULL);
    assert(ptr  != NULL);

    if (*src == 0) *ptr++ = '.';                                    // Special case: For root, just write a dot.

    while (*src) {                                                  // While more characters in the domain name.
        if (src + 1 + *src >= max) return(NULL);
        ptr = MyConvertDomainLabelToCString((const domainlabel *)src, ptr);
        if (!ptr) return(NULL);
        src += 1 + *src;
        *ptr++ = '.';                                               // Write the dot after the label.
    }

    *ptr++ = 0;                                                     // Null-terminate the string
    return(ptr);                                                    // and return.
}



/* The DNSServiceQueryRecord callback returns a DNS PTR record with rdata formatted like:

\005_http\004_tcp\005local\0
    
MyGetTypeAndDomain() takes the rdata and splits it up into two C strings which correspond to the
"type" and "domain" of a service.  These strings could potentially be passed to a function
like DNSServiceBrowse().  Assuming the example rdata above, this function would return "_http._tcp."
as the "type", and "local." for the "domain". */

static void
MyGetTypeAndDomain(const void * rdata, uint16_t rdlen, char * type, char * domain)
{
    unsigned char *cursor;
    unsigned char *start;
    unsigned char *end;

    assert(rdata  != NULL);
    assert(rdlen  != 0);
    assert(type   != NULL);
    assert(domain != NULL);

    start = (unsigned char*)malloc(rdlen);
    assert(start != NULL);
    memcpy(start, rdata, rdlen);

    end = start + rdlen;
    cursor = start;
    if ((*cursor == 0) || (*cursor >= 64)) goto exitWithError;
    cursor += 1 + *cursor;                                       // Move to the start of the second DNS label.
    if (cursor >= end) goto exitWithError;
    if ((*cursor == 0) || (*cursor >= 64)) goto exitWithError;
    cursor += 1 + *cursor;                                       // Move to the start of the thrid DNS label.
    if (cursor >= end) goto exitWithError;
    
    /* Take everything from start of third DNS label until end of DNS name and call that the "domain". */
    if (MyConvertDomainNameToCString((const domainname *)cursor, domain) == NULL) goto exitWithError;
    *cursor = 0;                                                 // Set the length byte of the third label to zero.

    /* Take the first two DNS labels and call that the "type". */
    if (MyConvertDomainNameToCString((const domainname *)start, type) == NULL) goto exitWithError;
    free(start);
    return;

exitWithError:
    fprintf(stderr, "Invalid DNS name string\n");
    free(start);
}



static void
MyDNSServiceCleanUp(MyDNSServiceState * query)
{
    /* Remove the CFRunLoopSource from the current run loop. */
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), query->source, kCFRunLoopCommonModes);
    CFRelease(query->source);

    /* Invalidate the CFSocket. */
    CFSocketInvalidate(query->socket);
    CFRelease(query->socket);

    /* Workaround that gives time to CFSocket's select thread so it can remove the socket from its FD set
    before we close the socket by calling DNSServiceRefDeallocate. <rdar://problem/3585273> */
    usleep(1000);

    /* Terminate the connection with the mDNSResponder daemon, which cancels the query. */
    DNSServiceRefDeallocate(query->service);
}



/* MySocketReadCallback() gets called when data is available for reading from the Unix domain socket
connected to the mDNSResponder daemon.  This happens when the mDNSResponder delivers us a response to our query. */

static void
MySocketReadCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void * data, void * info)
{
#if defined(_APPLE_)
    #pragma unused(s)
    #pragma unused(type)
    #pragma unused(address)
    #pragma unused(data)
#endif

    DNSServiceErrorType err;
 
    MyDNSServiceState* query = (MyDNSServiceState*)info;  // context passed in to CFSocketCreateWithNative().
    assert(query != NULL);

    /* Read a reply from the mDNSResponder, which will end up calling MyMetaQueryCallback(). */
    err= DNSServiceProcessResult(query->service);
    if (err != kDNSServiceErr_NoError)
    {
        fprintf(stderr, "DNSServiceProcessResult returned %d\n", err);
        
        /* Terminate the query operation and release the CFRunLoopSource and CFSocket. */
        MyDNSServiceCleanUp(query);
        CFRunLoopStop(CFRunLoopGetCurrent());
    }
}



void
MyDNSServiceAddServiceToRunLoop(MyDNSServiceState* query)
{
    CFSocketNativeHandle sock;
    CFOptionFlags        sockFlags;
    CFSocketContext      context = { 0, query, NULL, NULL, NULL };  // Use MyDNSServiceState as context data.
    
    /* Access the underlying Unix domain socket to communicate with the mDNSResponder daemon. */
    sock = DNSServiceRefSockFD(query->service);
    assert(sock != -1);
    
    /* Create a CFSocket using the Unix domain socket. */
    query->socket = CFSocketCreateWithNative(NULL, sock, kCFSocketReadCallBack, MySocketReadCallback, &context);
    assert(query->socket != NULL);
    
    /* Prevent CFSocketInvalidate from closing DNSServiceRef's socket. */
    sockFlags = CFSocketGetSocketFlags(query->socket);
    CFSocketSetSocketFlags(query->socket, sockFlags & (~kCFSocketCloseOnInvalidate));
    
    /* Create a CFRunLoopSource from the CFSocket. */
    query->source = CFSocketCreateRunLoopSource(NULL, query->socket, 0);
    assert(query->source != NULL);

    /* Add the CFRunLoopSource to the current run loop. */
    CFRunLoopAddSource(CFRunLoopGetCurrent(), query->source, kCFRunLoopCommonModes);
}


static void
MyConvertInterfaceIndexToName(uint32_t interface, char * interfaceName)
{
    assert(interfaceName != NULL);
    
    if      (interface == 0)          strcpy(interfaceName,   "all");   // All active network interfaces.
    else if (interface == 0xFFFFFFFF) strcpy(interfaceName, "local");   // Only available locally on this machine.
    else if_indextoname(interface, interfaceName);                      // Converts interface index to interface name.
}


static void
DNSSD_API MyMetaQueryCallback(DNSServiceRef service, DNSServiceFlags flags, uint32_t interfaceID, DNSServiceErrorType error,
    const char * fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void * rdata, uint32_t ttl, void * context)
{    
#if defined(_APPLE_)
    #pragma unused(service)
    #pragma unused(rrclass)
    #pragma unused(ttl)
    #pragma unused(context)
#endif

    assert(strcmp(fullname, kServiceMetaQueryName) == 0);
                    
    if (error == kDNSServiceErr_NoError) {
    
#if defined(_GNUC_)
        char interfaceName[IF_NAMESIZE] = "";
        char domain[MAX_DOMAIN_NAME]    = "";
        char type[MAX_DOMAIN_NAME]      = "";
#else
        char interfaceName[MAX_DOMAIN_NAME];
        char domain[MAX_DOMAIN_NAME];
        char type[MAX_DOMAIN_NAME];

        memset(interfaceName, 0x00, MAX_DOMAIN_NAME);
        memset(domain, 0x00, MAX_DOMAIN_NAME);
        memset(type, 0x00, MAX_DOMAIN_NAME);
#endif
        /* Get the type and domain from the discovered PTR record. */
        MyGetTypeAndDomain(rdata, rdlen, type, domain);        

        /* Convert an interface index into a BSD-style interface name. */
        MyConvertInterfaceIndexToName(interfaceID, interfaceName);
        
        if (flags & kDNSServiceFlagsAdd) {
            fprintf(stderr, "ADD      %-28s  %-14s %s\n", type, domain, interfaceName);
        } else {
            /* REMOVE is only called when a network interface is disabled or if the record
            expires from the cache.  For network efficiency reasons, clients do not send
            goodbye packets for meta-query PTR records when deregistering a service.  */
            fprintf(stderr, "REMOVE   %-28s  %-14s %s\n", type, domain, interfaceName);
        }
    } else {
        fprintf(stderr, "MyQueryRecordCallback returned %d\n", error);
    }
}



static DNSServiceErrorType
MyDNSServiceMetaQuery(MyDNSServiceState * query, DNSServiceQueryRecordReply callback)
{
    DNSServiceErrorType error;

    assert(query    != NULL);
    assert(callback != NULL);

    /* Issue a Multicast DNS query for the service type meta-query PTR record. */
    error = DNSServiceQueryRecord(&query->service,
                                                0,  // no flags
                                                0,  // all network interfaces
                            kServiceMetaQueryName,  // meta-query record name
                                         ns_t_ptr,  // DNS PTR Record
                                          ns_c_in,  // Internet Class
                                         callback,  // callback function ptr
                                            NULL);  // no context

    if (error == kDNSServiceErr_NoError) {

        /* Create a CFRunLoopSource and add it to the run loop to enable asynchronous callbacks. */
        MyDNSServiceAddServiceToRunLoop(query);
        
        fprintf(stderr, "Event       Service Type               Domain      Interface\n");
        fprintf(stderr, "------------------------------------------------------------\n");
    }

    return error;
}



int
main (int argc, const char * argv[])
{
#if defined(_APPLE_)
    #pragma unused(argc)
    #pragma unused(argv)
#endif
    
    MyDNSServiceState query;
    DNSServiceErrorType error;
    
    /* Start the DNS-SD services meta-query, create the CFRunLoopSource, add it to the run loop. */
    error = MyDNSServiceMetaQuery(&query, (DNSServiceQueryRecordReply)MyMetaQueryCallback);
    if (error == kDNSServiceErr_NoError) {
    
        /* Start the run loop to receive asynchronous callbacks via MyMetaQueryCallback. */
        CFRunLoopRun();
        
        /* Terminate the query operation and release the CFRunLoopSource and CFSocket. */
        MyDNSServiceCleanUp(&query);
        
    } else {
        fprintf(stderr, "MyDNSServiceMetaQuery returned %d\n", error);
    }

    return 0;
}

#if defined(WIN32)
/*
 * The following implementation is from Apple's Bonjour implementation.
 * It's so annoying that this is not provided on user-level Windows OS's
 * (you have to get server 2008 to have this implemented in the OS.)
 *
* Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
static char*
if_indextoname( DWORD ifIndex, char * nameBuff)
{
        PIP_ADAPTER_INFO        pAdapterInfo = NULL;
        PIP_ADAPTER_INFO        pAdapter = NULL;
        DWORD                           dwRetVal = 0;
        char                    *       ifName = NULL;
        ULONG                           ulOutBufLen = 0;

        if (GetAdaptersInfo( NULL, &ulOutBufLen) != ERROR_BUFFER_OVERFLOW)
        {
                goto exit;
        }

        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);

        if (pAdapterInfo == NULL)
        {
                goto exit;
        }

        dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );

        if (dwRetVal != NO_ERROR)
        {
                goto exit;
        }

        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
                if (pAdapter->Index == ifIndex)
                {
                        // It would be better if we passed in the length of nameBuff to this
                        // function, so we would have absolute certainty that no buffer
                        // overflows would occur.  Buffer overflows *shouldn't* occur because
                        // nameBuff is of size MAX_ADAPTER_NAME_LENGTH.
                        strcpy( nameBuff, pAdapter->AdapterName );
                        ifName = nameBuff;
                        break;
                }

                pAdapter = pAdapter->Next;
        }

exit:

        if (pAdapterInfo != NULL)
        {
                free( pAdapterInfo );
                pAdapterInfo = NULL;
        }

        return ifName;
}
#endif
