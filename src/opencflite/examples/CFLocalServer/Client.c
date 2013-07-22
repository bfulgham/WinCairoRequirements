/*
 *    Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>
 *    Copyright (c) 2009 Brent Fulgham.  All rights reserved.
 *    All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the BSD-style license (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 */

/*
    File:       Client.c

    Contains:   Client showing integration of CFSockets and UNIX domain sockets.

    Written by: DTS

    Copyright:  Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

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

$Log: Client.c,v $
Revision 1.2  2005/05/18 13:36:28         
Fixed various documentation/comment changes.

Revision 1.1  2005/05/17 12:19:13         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <CoreFoundation/CoreFoundation.h>

#include <stdlib.h>
#include <assert.h>

#if defined(WIN32)
#include <errno.h>
#define ECANCELED 15

#include <stdio.h>
#define snprintf _snprintf

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#define PRId32 "d"
#define PRIu32 "u"

#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#endif

// Project interfaces

#include "Protocol.h"
#include "Common.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Connection Abstraction

// A ConnectionRef represents a connection from the client to the server. 
// The internals of this are opaque to external callers.  All operations on 
// a connection are done via the routines in this section.

enum {
    kConnectionStateMagic = 'LCCM'           // Local Client Connection Magic
};

typedef struct ConnectionState *  ConnectionRef;
    // Pseudo-opaque reference to the connection.

typedef Boolean (*ConnectionCallbackProcPtr)(
    ConnectionRef           conn, 
    const PacketHeader *    packet, 
    void *                  refCon
);
    // When the client enables listening on a connection, it supplies a 
    // function of this type as a callback.  We call this function in 
    // the context of the runloop specified by the client when they enable 
    // listening.
    //
    // conn is a reference to the connection.  It will not be NULL.
    //
    // packet is a pointer to the packet that arrived, or NULL if we've 
    // detected that the connection to the server is broken.
    //
    // refCon is a value that the client specified when it registered this 
    // callback.
    //
    // If the server sends you a bad packet, you can return false to 
    // tell the connection management system to shut down the connection.

// ConnectionState is the structure used to track a single connection to 
// the server.  All fields after fSockFD are only relevant if the client 
// has enabled listening.

struct ConnectionState {
    OSType                      fMagic;             // kConnectionStateMagic
    int                         fSockFD;            // UNIX domain socket to server
    CFSocketRef                 fSockCF;            // CFSocket wrapper for the above
    CFRunLoopSourceRef          fRunLoopSource;     // runloop source for the above
    CFMutableDataRef            fBufferedPackets;   // buffer for incomplete packet data
    ConnectionCallbackProcPtr   fCallback;          // client's packet callback
    void *                      fCallbackRefCon;    // refCon for the above.
};

// Forward declarations.  See the comments associated with the function definition.

static void ConnectionShutdown(ConnectionRef conn);
static void ConnectionCloseInternal(ConnectionRef conn, Boolean sayGoodbye);

static int ConnectionOpen(ConnectionRef *connPtr)
    // Opens a connection to the server.
    //
    // On entry, connPtr must not be NULL
    // On entry, *connPtr must be NULL
    // Returns an errno-style error code
    // On success, *connPtr will not be NULL
    // On error, *connPtr will be NULL
{
    int                 err;
    ConnectionRef       conn;
    Boolean             sayGoodbye;
#if !defined(_WIN32)
    int sockType = AF_UNIX;
#else
    int sockType = AF_INET;
#endif
    
    assert( connPtr != NULL);
    assert(*connPtr == NULL);
    
    sayGoodbye = false;
    
    // Allocate a ConnectionState structure and fill out some basic fields.
    
    err = 0;
    conn = (ConnectionRef) calloc(1, sizeof(*conn));
    if (conn == NULL) {
        err = ENOMEM;
    }
    if (err == 0) {
        conn->fMagic  = kConnectionStateMagic;
        
        // For clean up to work properly, we must make sure that, if 
        // the connection record is allocated successfully, we always 
        // set fSockFD to -1.  So, while the following line is redundant 
        // in the current code, it's present to press home this point.

        conn->fSockFD = -1;
    }
    
    // Create a UNIX domain socket and connect to the server. 
    
    if (err == 0) {
        conn->fSockFD = socket(sockType, SOCK_STREAM, 0);
        err = MoreUNIXErrno(conn->fSockFD);
    }
    if (err == 0) {
        struct sockaddr_un connReq;

        connReq.sun_family = AF_UNIX;
        strcpy(connReq.sun_path, kServerSocketPath);

        err = connect(conn->fSockFD, (struct sockaddr*) &connReq, SUN_LEN(&connReq));
        err = MoreUNIXErrno(err);
        
        sayGoodbye = (err == 0);
    }
    
    // Clean up.
    
    if (err != 0) {
        ConnectionCloseInternal(conn, sayGoodbye);
        conn = NULL;
    }
    *connPtr = conn;
    
    assert( (err == 0) == (*connPtr != NULL) );
    
    return err;
}

static int ConnectionSend(ConnectionRef conn, const PacketHeader *packet)
    // Send a packet to the server.  Use this when you're not expecting a 
    // reply.
    //
    // conn must be a valid connection
    // packet must be a valid, ready-to-send, packet
    // Returns an errno-style error code
{
    int     err;
    
    assert(conn != NULL);
    assert(conn->fSockFD != -1);            // connection must not be shut down
    // conn->fSockCF may or may not be NULL; it's OK to send a packet when listening 
    // because there's no reply; OTOH, you can't do an RPC while listening because 
    // an unsolicited packet might get mixed up with the RPC reply.
    
    assert(packet != NULL);
    assert(packet->fMagic == kPacketMagic);
    assert(packet->fSize >= sizeof(PacketHeader));
    
    // Simply send the packet down the socket.
    
    err = MoreUNIXWrite(conn->fSockFD, packet, packet->fSize, NULL);
    
    return err;
}

static int ConnectionRPC(
    ConnectionRef           conn, 
    const PacketHeader *    request, 
    PacketHeader *          reply, 
    size_t                  replySize
)
    // Perform an RPC (Remote Procedure Call) with the server.  That is, send 
    // the server a packet and wait for a reply.  You can only use this on 
    // connections that are not in listening mode.
    //
    // conn must be a valid connection
    //
    // packet must be a valid, ready-to-send, packet
    //
    // reply and replySize specify a buffer where the reply packet is placed;
    // reply size must not be NULL; replySize must not be less that the 
    // packet header size (sizeof(PacketHeader)); if the reply packet is bigger 
    // than replySize, the data that won't fit is discarded; you can detect this 
    // by looking at reply->fSize
    //
    // Returns an errno-style error code
    // On success, the buffer specified by reply and replySize will contain the 
    // reply packet; on error, the contents of that buffer is invalid; also, 
    // if this routine errors the connection is no longer useful (conn is still 
    // valid, but you can't use it to transmit any more data)
{
    int     err;
    
    assert(conn != NULL);
    assert(conn->fSockFD != -1);            // connection must not be shut down
    assert(conn->fSockCF == NULL);          // RPC and listening are mutually exclusive
                                            // because unsolicited packet might get mixed up 
                                            // with the reply

    assert(request != NULL);
    assert(request->fMagic == kPacketMagic);
    assert(request->fSize >= sizeof(PacketHeader));

    assert(reply != NULL);
    assert(replySize >= sizeof(PacketHeader));
    
    // Send the request.
    
    err = ConnectionSend(conn, request);
    
    // Read and validate the reply header.
    
    if (err == 0) {
        err = MoreUNIXRead(conn->fSockFD, reply, sizeof(PacketHeader), NULL);
    }
    if ( (err == 0) && (reply->fMagic != kPacketMagic) ) {
        fprintf(stderr, "ConnectionRPC: Bad magic (%.4s).\n", (char *) &reply->fMagic);
        err = EINVAL;
    }
    if ( (err == 0) && (reply->fType != kPacketTypeReply) ) {
        fprintf(stderr, "ConnectionRPC: Type wrong (%.4s).\n", (char *) &reply->fType);
        err = EINVAL;
    }
    if ( (err == 0) && (reply->fID != request->fID) ) {
        fprintf(stderr, "ConnectionRPC: ID mismatch (%" PRId32 ").\n", reply->fID);
        err = EINVAL;
    }
    if ( (err == 0) && ( (reply->fSize < sizeof(PacketHeader)) || (reply->fSize > kPacketMaximumSize) ) ) {
        fprintf(stderr, "ConnectionRPC: Bogus packet size (%" PRIu32 ").\n", reply->fSize);
        err = EINVAL;
    }

    // Read the packet payload that will fit in the reply buffer.
    
    if ( (err == 0) && (reply->fSize > sizeof(PacketHeader)) ) {
        uint32_t  payloadToRead;
        
        if (reply->fSize > replySize) {
            payloadToRead = replySize;
        } else {
            payloadToRead = reply->fSize;
        }
        payloadToRead -= sizeof(PacketHeader);
        
        err = MoreUNIXRead(conn->fSockFD, ((char *) reply) + sizeof(PacketHeader), payloadToRead, NULL);
    }

    // Discard any remaining packet payload that will fit in the reply buffer.
    // The addition check in the next line is necessary to avoid the undefined behaviour 
    // of malloc(0) in the dependent block.

    if ( (err == 0) && (reply->fSize > replySize) ) {
        uint32_t    payloadToJunk;
        void *      junkBuf;
        
        payloadToJunk = reply->fSize - replySize;
        
        junkBuf = malloc(payloadToJunk);
        if (junkBuf == NULL) {
            err = ENOMEM;
        }
        
        if (err == 0) { 
            err = MoreUNIXRead(conn->fSockFD, junkBuf, payloadToJunk, NULL);
        }
        
        free(junkBuf);
    }

    // Any errors cause us to immediately shut down our connection because we 
    // we're no longer sure of the state of the channel (that is, did we leave 
    // half a packet stuck in the pipe).
    
    if (err != 0) {
        ConnectionShutdown(conn);
    }
    
    return err;
}

static void ConnectionGotData(
    CFSocketRef             s, 
    CFSocketCallBackType    type, 
    CFDataRef               address, 
    const void *            data, 
    void *                  info
)
    // This is a a CFSocket callback indicating that data has arrived on the 
    // socket.  It's only called if the user has registered the associated 
    // connection for listening.  The parameter are as per the CFSocket 
    // documentation.  As this is a callback of type kCFSocketDataCallBack, 
    // data contains newly arrived data that CFSocket has already read for us.
{
    CFDataRef       newData;
    ConnectionRef   conn;

	(void)address;
    assert(s != NULL);
    assert(type == kCFSocketDataCallBack);

    // Cast data to a CFDataRef, newData.
    
    newData = (CFDataRef) data;
    assert(newData != NULL);
    assert( CFGetTypeID(newData) == CFDataGetTypeID() );
    
    // Cast info to a ConnectionRef.
    
    conn = (ConnectionRef) info;
    assert(conn->fMagic == kConnectionStateMagic);
    
    if ( CFDataGetLength(newData) == 0 ) {
        // End of data stream; the server is dead.
        
        fprintf(stderr, "ConnectionGotData: Server died unexpectedly.\n");

        // Tell the client.
        
        (void) conn->fCallback(conn, NULL, conn->fCallbackRefCon);
        
        // Shut 'er down Clancy, she's pumping mud!
        
        ConnectionShutdown(conn);
    } else {
        // We have new data from the server.  Appending to our buffer.
        
        CFDataAppendBytes(conn->fBufferedPackets, CFDataGetBytePtr(newData), CFDataGetLength(newData));
        
        // Now see if there are any complete packets in the buffer; and, 
        // if so, deliver them to the client.
        
        do {
            PacketHeader *  thisPacket;
            Boolean         success;
            
            if ( CFDataGetLength(conn->fBufferedPackets) < sizeof(PacketHeader) ) {
                // Not enough data for the packet header; we're done.
                break;
            }
            
            thisPacket = (PacketHeader *) CFDataGetBytePtr(conn->fBufferedPackets);
            
            if ( thisPacket->fMagic != kPacketMagic ) {
                fprintf(stderr, "ConnectionGotData: Server sent us a packet with bad magic (%.4s).\n", (char *) &thisPacket->fMagic);
                
                ConnectionShutdown(conn);
                break;
            }
            
            if (thisPacket->fSize > kPacketMaximumSize) {
                fprintf(stderr, "ConnectionGotData: Server sent us a packet that's just too big (%" PRIu32 ").\n", thisPacket->fSize);
                
                ConnectionShutdown(conn);
                break;
            }
            
            if ( CFDataGetLength(conn->fBufferedPackets) < thisPacket->fSize ) {
                // Not enough data for the packet body; we're done.
                break;
            }
            
            // Tell the client about the packet.

            success = conn->fCallback(conn, thisPacket, conn->fCallbackRefCon);
            if ( ! success ) {
                ConnectionShutdown(conn);
                break;
            }
            
            // Delete this packet from the front of our packet buffer.  I horror at 
            // the inefficiency of this, but it is sample code after all.
            
            CFDataDeleteBytes(conn->fBufferedPackets, CFRangeMake(0, thisPacket->fSize));
            
        } while (true);
    }
}

static int ConnectionRegisterListener(
    ConnectionRef               conn, 
    CFRunLoopRef                runLoop,
    CFStringRef                 runLoopMode, 
    ConnectionCallbackProcPtr   callback, 
    void *                      refCon
)
    // Register a listener to be called when packets arrive.  Once you've done 
    // this, you can no longer use conn for RPCs.
    //
    // conn must be a valid connection
    //
    // runLoop and runLoopMode specify the context in which the callback will 
    // be called; in most cases you specify CFRunLoopGetCurrent() and 
    // kCFRunLoopDefaultMode
    //
    // callback is the function you want to be called when packets arrive; it 
    // must not be NULL
    //
    // refCon is passed to callback
    //
    // Returns an errno-style error code
    // On success, the connection has been converted to a listener and your 
    // callback will be called from the context of the specific runloop when 
    // a packet arrives; on error, the connection is no longer useful (conn is 
    // still valid, but you can't use it to transmit any more data)
{
    int             err;

    assert(conn != NULL);
    assert(runLoop != NULL);
    assert(runLoopMode != NULL);
    assert(callback != NULL);

    assert(conn->fSockFD != -1);            // connection must not be shut down
    assert(conn->fSockCF == NULL);          // can't register twice
    
    // Create the packet buffer.
    
    err = 0;
    conn->fBufferedPackets = CFDataCreateMutable(NULL, 0);
    if (conn->fBufferedPackets == NULL) {
        err = ENOMEM;
    }
    
    // Add the source to the runloop.
    
    if (err == 0) {
        CFSocketContext context;

        memset(&context, 0, sizeof(context));
        context.info = conn;

        conn->fSockCF = CFSocketCreateWithNative(
            NULL, 
            (CFSocketNativeHandle) conn->fSockFD, 
            kCFSocketDataCallBack, 
            ConnectionGotData, 
            &context
        );
        if (conn->fSockCF == NULL) {
            err = EINVAL;
        }
    }
    if (err == 0) {
        conn->fRunLoopSource = CFSocketCreateRunLoopSource(NULL, conn->fSockCF, 0);
        if (conn->fRunLoopSource == NULL) {
            err = EINVAL;
        }
    }
    if (err == 0) {
        conn->fCallback = callback;
        conn->fCallbackRefCon = refCon;
        
        CFRunLoopAddSource( runLoop, conn->fRunLoopSource, runLoopMode);
    }

    // Any failure means the entire connection is dead; again, this is the 
    // draconian approach to error handling.  But hey, connections are 
    // (relatively) cheap.
    
    if (err != 0) {
        ConnectionShutdown(conn);
    }
    
    return err;
}

static void ConnectionShutdown(ConnectionRef conn)
    // This routine shuts down down the connection to the server 
    // without saying goodbye; it leaves conn valid.  This routine 
    // is primarily used internally to the connection abstraction 
    // where we notice that the connection has failed for some reason. 
    // It's also called by the client after a successful quit RPC 
    // because we know that the server has closed its end of the 
    // connection.
    //
    // It's important to nil out the fields as we close them because 
    // this routine is called if any messaging routine fails.  If it 
    // doesn't nil out the fields, two bad things might happen:
    //
    // o When the connection is eventually closed, ConnectionCloseInternal 
    //   will try to send a Goodbye, which fails triggering an assert.
    //
    // o If ConnectionShutdown is called twice on a particular connection 
    //   (which happens a lot; this is a belts and braces implementation 
    //   [that's "belts and suspenders" for the Americans reading this; 
    //   ever wonder why Monty Python's lumberjacks sing about "suspenders 
    //   and a bra"?; well look up "suspenders" in a non-American dictionary 
    //   for a quiet chuckle :-] )
{
    int     junk;
    Boolean hadSockCF;

    assert(conn != NULL);
    
    conn->fCallback       = NULL;
    conn->fCallbackRefCon = NULL;

    if (conn->fRunLoopSource != NULL) {
        CFRunLoopSourceInvalidate(conn->fRunLoopSource);
        
        CFRelease(conn->fRunLoopSource);
        
        conn->fRunLoopSource = NULL;
    }
    
    // CFSocket will close conn->fSockFD when we invalidate conn->fSockCF, 
    // so we remember whether we did this so that, later on, we know 
    // whether to close the file descriptor ourselves.  We need an extra 
    // variable because we NULL out fSockCF as we release it, for the reason 
    // described above.
    
    hadSockCF = (conn->fSockCF != NULL);
    if (conn->fSockCF != NULL) {
        CFSocketInvalidate(conn->fSockCF);
        
        CFRelease(conn->fSockCF);
        
        conn->fSockCF = NULL;
    }

    if (conn->fBufferedPackets != NULL) {
        CFRelease(conn->fBufferedPackets);
        conn->fBufferedPackets = NULL;
    }

    if ( (conn->fSockFD != -1) && ! hadSockCF ) {
#if !defined(_WIN32)
        junk = close(conn->fSockFD);
#else
        junk = closesocket(conn->fSockFD);
#endif
        assert(junk == 0);
    }
    // We always set fSockFD to -1 because either we've closed it or 
    // CFSocket has.
    conn->fSockFD = -1;
}

static void ConnectionCloseInternal(ConnectionRef conn, Boolean sayGoodbye)
    // The core of ConnectionClose.  It's called by ConnectionClose 
    // and by ConnectionOpen, if it fails for some reason.  This exists 
    // as a separate routine so that we can add the sayGoodbye parameter, 
    // which controls whether we send a goodbye packet to the server.  We 
    // need this because we should always try to say goodbye if we're called 
    // from ConnectionClose, but if we're called from ConnectionOpen we 
    // should only try to say goodbye if we successfully connected the 
    // socket.
    //
    // Regardless, the bulk of the work of this routine is done by 
    // ConnectionShutdown.  This routine exists to a) say goodbye, if 
    // necessary, and b) free the memory associated with the connection.
{
    int     junk;
    
    if (conn != NULL) {
        assert(conn->fMagic == kConnectionStateMagic);

        if ( (conn->fSockFD != -1) && sayGoodbye ) {
            PacketGoodbye   goodbye;

            InitPacketHeader(&goodbye.fHeader, kPacketTypeGoodbye, sizeof(goodbye), false);
            snprintf(goodbye.fMessage, sizeof(goodbye.fMessage), "Process %ld signing off", (long) getpid());
            
            junk = ConnectionSend(conn, &goodbye.fHeader);
            assert(junk == 0);
        }
        ConnectionShutdown(conn);
        
        free(conn);
    }
}

static void ConnectionClose(ConnectionRef conn)
    // Closes the connection.  It's legal to pass conn as NULL, in which 
    // case this does nothing (kinda like free'ing NULL).
{
    ConnectionCloseInternal(conn, true);
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Command Line Tool

// The following routines use the connection abstraction defined above 
// to implement a simple command line tool that exercises the server.

enum {
    kResultColumnWidth = 10
};

static void PrintResult(const char *command, int errNum, const char *arg)
    // Prints the result of a command.  command is the name of the 
    // command, errNum is the errno-style error number, and arg 
    // (if not NULL) is the command argument.
{
    if (errNum == 0) {
        if (arg == NULL) {
            fprintf(stderr, "%*s\n", kResultColumnWidth, command);
        } else {
            fprintf(stderr, "%*s \"%s\"\n", kResultColumnWidth, command, arg);
        }
    } else {
        fprintf(stderr, "%*s failed with error %d\n", kResultColumnWidth, command, errNum);
    }
}

static void DoNOP(ConnectionRef conn)
    // Implements the "nop" command by doing a NOP RPC with the server.
{
    int         err;
    PacketNOP   request;
    PacketReply reply;
    
    InitPacketHeader(&request.fHeader, kPacketTypeNOP, sizeof(request), true);
    
    err = ConnectionRPC(conn, &request.fHeader, &reply.fHeader, sizeof(reply));
    if (err == 0) {
        err = reply.fErr;
    }
    PrintResult("nop", err, NULL);
}

static void DoWhisper(ConnectionRef conn, const char *message)
    // Implements the "whisper" command by doing a whisper RPC with the server.
    //
    // The server responds to this RPC by printing the message.
{
    int             err;
    PacketWhisper   request;
    PacketReply     reply;
    
    InitPacketHeader(&request.fHeader, kPacketTypeWhisper, sizeof(request), true);
    snprintf(request.fMessage, sizeof(request.fMessage), "%s", message);
    
    err = ConnectionRPC(conn, &request.fHeader, &reply.fHeader, sizeof(reply));
    if (err == 0) {
        err = reply.fErr;
    }
    PrintResult("whisper", err, message);
}

static void DoShout(ConnectionRef conn, const char *message)
    // Implements the "shout" command by sending a shout packet to the server. 
    // Note that this is /not/ an RPC.
    //
    // The server responds to this packet by echoing it to each registered 
    // listener.
{
    int         err;
    PacketShout request;
    
    InitPacketHeader(&request.fHeader, kPacketTypeShout, sizeof(request), false);
    snprintf(request.fMessage, sizeof(request.fMessage), "%s", message);
    
    err = ConnectionSend(conn, &request.fHeader);
    PrintResult("shout", err, message);
}

static Boolean GotPacket(ConnectionRef conn, const PacketHeader *packet, void *refCon)
    // DoListen registers this routine with the connection abstraction layer 
    // so that it is called when a packet arrives.  For a description of the 
    // parameters, see the comments next to ConnectionCallbackProcPtr.
{
    Boolean         result;
    CFRunLoopRef    runLoop;
    
	(void)conn;

    // When we register this callback, we pass a reference to the runloop 
    // as the refCon.  Extract that reference here.
    
    runLoop = (CFRunLoopRef) refCon;
    assert(runLoop != NULL);
    assert( CFGetTypeID(runLoop) == CFRunLoopGetTypeID() );
    
    result = true;
    if (packet == NULL) {
        // Server connection has gone away.  No need to return false because 
        // the connection is torn anyway.
        
        CFRunLoopStop(runLoop);
    } else {
        // We got a packet from the server.  Tell the user about it.
        
        switch (packet->fType) {
            case kPacketTypeShout:
                if (packet->fSize != sizeof(PacketShout)) {
                    fprintf(stderr, "GotPacket: Server sent us a Shout with the wrong size (%" PRIu32 ").\n", packet->fSize);
                    result = false;
                }
                
                if (result && (packet->fID != kPacketIDNone) ) {
                    fprintf(stderr, "GotPacket: Server sent us a Shout with the wrong size (%" PRId32 ").\n", packet->fID);
                    result = false;
                }

                if (result) {
                    PacketShout * shoutPacket;
                    
                    shoutPacket = (PacketShout *) packet;
                    fprintf(stderr, "%*s heard \"%.*s\"\n", kResultColumnWidth, "", (int) sizeof(shoutPacket->fMessage), shoutPacket->fMessage);                
                }
                break;
            default:   
                fprintf(stderr, "GotPacket: Server sent us a packet with an unexpected type (%.4s).\n", (char *) &packet->fType);
                result = false;
                break;
        }
    }
    
    return result;
}

static void DoListen(ConnectionRef conn)
    // Implements the "listen" command.  First this does a listen RPC 
    // to tell the server that this connection is now a listener.  Next it 
    // calls ConnectionRegisterListener to register a packet listener callback 
    // (GotPacket, above) on the runloop.  It then runs the runloop until its 
    // stopped (by a SIGINT, via SIGINTRunLoopCallback, below).
{
    int             err;
    PacketListen    request;
    PacketReply     reply;
    
    InitPacketHeader(&request.fHeader, kPacketTypeListen, sizeof(request), true);
    
    err = ConnectionRPC(conn, &request.fHeader, &reply.fHeader, sizeof(reply));
    if (err == 0) {
        err = reply.fErr;
    }
    if (err == 0) {
        err = ConnectionRegisterListener(
            conn, 
            CFRunLoopGetCurrent(), 
            kCFRunLoopDefaultMode, 
            GotPacket, 
            CFRunLoopGetCurrent()
        );
    }
    if (err != 0) {
        PrintResult("listen", err, NULL);
    } else {
        fprintf(stderr, "%*s Press ^C to quit.\n", kResultColumnWidth, "listen");

        CFRunLoopRun();
    }
}

static void DoQuit(ConnectionRef conn)
    // Implements the "quit" command by doing a quit RPC with the server. 
    // The server responds to this RPC by quitting.  Cleverly, it sends us 
    // the RPC reply right before quitting.
{
    int         err;
    PacketQuit  request;
    PacketReply reply;
    
    InitPacketHeader(&request.fHeader, kPacketTypeQuit, sizeof(request), true);
    
    err = ConnectionRPC(conn, &request.fHeader, &reply.fHeader, sizeof(reply));
    if (err == 0) {
        err = reply.fErr;
    }
    if (err == 0) {
        // If the quit is successful, we shut down our end of the connection 
        // because we know that the server has shut down its end.
        ConnectionShutdown(conn);
    }
    PrintResult("quit", err, NULL);
}

#if !defined(_WIN32)
static void SIGINTRunLoopCallback(const siginfo_t *sigInfo, void *refCon)
    // This routine is called in response to a SIGINT signal. 
    // It is not, however, a signal handler.  Rather, we 
    // orchestrate to have it called from the runloop (via 
    // the magic of InstallSignalToSocket).  It's purpose 
    // is to stop the runloop when the user types ^C.
{
    (void)sigInfo;
	(void)refCon;
    
    // Stop the runloop.  Note that we can get a reference to the runloop by 
	// calling CFRunLoopGetCurrent because this is called from the runloop.
    
    CFRunLoopStop( CFRunLoopGetCurrent() );
    
    // Print a bonus newline to ensure that the next command prompt isn't 
    // printed on the same line as the echoed ^C.
    
    fprintf(stderr, "\n");
}
#endif

static void PrintUsage(const char *argv0)
    // Print the program's usage.
{
    const char *command;
    
    command = strrchr(argv0, '/');
    if (command == NULL) {
        command = argv0;
    } else {
        command += 1;
    }
    fprintf(stderr, "usage: %s command...\n", command);
    fprintf(stderr, "       commands: nop\n");
    fprintf(stderr, "                 whisper <message>\n");
    fprintf(stderr, "                 shout   <message>\n");
    fprintf(stderr, "                 listen  (must be last)\n");
    fprintf(stderr, "                 quit\n");
}

int main (int argc, const char * argv[])
    // The primary entry point.
{
    int             err;
    ConnectionRef   conn;
    
    conn = NULL;
    
    // If we get no arguments, just print the usage and fail.
    
    err = 0;
    if (argc == 1) {
        PrintUsage(argv[0]);
        err = ECANCELED;
    }
    
#if !defined(_WIN32)
    // SIGPIPE is evil, so tell the system not to send it to us.
    
    if (err == 0) {
        err = MoreUNIXIgnoreSIGPIPE();
    }

    // Organise to have SIGINT delivered to a runloop callback.
    
    if (err == 0) {
        sigset_t    justSIGINT;
        
        (void) sigemptyset(&justSIGINT);
        (void) sigaddset(&justSIGINT, SIGINT);
        
        err = InstallSignalToSocket(
            &justSIGINT,
            CFRunLoopGetCurrent(),
            kCFRunLoopDefaultMode,
            SIGINTRunLoopCallback,
            NULL
        );
    }
#else
    {
       WORD versionRequested = MAKEWORD(2, 0);
       WSADATA wsaData;
       err = WSAStartup(versionRequested, &wsaData);
       if (err != 0 || LOBYTE(wsaData.wVersion) != LOBYTE(versionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(versionRequested)) {
           WSACleanup();
           CFLog(0, CFSTR("*** Could not initialize WinSock subsystem!!!"));
       }
    }
#endif
    
    // Connect to the server.
    
    if (err == 0) {
        err = ConnectionOpen(&conn);
    }
    
    // Process the command line arguments.  Basically the arguments are a 
    // sequence of commands, which we process in order.  The logic is 
    // a little convoluted because some commands have arguments and because 
    // the "listen" command must come last.
    
    if (err == 0) {
        Boolean printTheUsage;
        int     argIndex;
        
		printTheUsage = false;
		
        argIndex = 1;
        while ( (err == 0) && (argIndex < argc) ) {
            if ( strcmp(argv[argIndex], "nop") == 0 ) {
                DoNOP(conn);
            } else if ( strcmp(argv[argIndex], "whisper") == 0 ) {
                argIndex += 1;
                if (argIndex < argc) {
                    DoWhisper(conn, argv[argIndex]);
                } else {
                    printTheUsage = true;
                    err = ECANCELED;
                }
            } else if ( strcmp(argv[argIndex], "shout") == 0 ) {
                argIndex += 1;
                if (argIndex < argc) {
                    DoShout(conn, argv[argIndex]);
                } else {
                    printTheUsage = true;
                    err = ECANCELED;
                }
            } else if ( strcmp(argv[argIndex], "listen") == 0 ) {
                if ( (argIndex + 1) == argc ) {         // if listen is the last argument
                    DoListen(conn);
                } else {
                    printTheUsage = true;
                    err = ECANCELED;
                }
            } else if ( strcmp(argv[argIndex], "quit") == 0 ) {
                DoQuit(conn);
            } else {
                printTheUsage = true;
                err = ECANCELED;
            }
            argIndex += 1;
        }
        
        if (printTheUsage) {
            PrintUsage(argv[0]);
        }
    }
    
    // Clean up.
    
    ConnectionClose(conn);

    if ( (err != 0) && (err != ECANCELED) ) {
        fprintf(stderr, "SimpleClientCF: Failed with error %d.\n", err);
    }
    
    return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
