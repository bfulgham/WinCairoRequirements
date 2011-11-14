/*
 *    Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>
 *    All rights reserved.
 */

/*
    File:       Server.c

    Contains:   Server showing integration of CFSockets and UNIX domain sockets.

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

$Log: Server.c,v $
Revision 1.2  2005/05/18 13:36:39         
Fixed various documentation/comment changes.

Revision 1.1  2005/05/17 12:19:32         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <CoreFoundation/CoreFoundation.h>

#include <stdlib.h>
#include <assert.h>
#include <signal.h>

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

typedef uint16_t mode_t;

#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/signal.h>
#endif

// Project interfaces

#include "Protocol.h"
#include "Common.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** Client State Management

// The server maintains a ClientState structure to track the state of 
// each client.  This state divides neatly into three groups.
//
// o socket -- fSockFD, fSockCF, and fRunLoopSource all represent different 
//   aspects of the UNIX domain socket that we're using to talk to the client.
//
// o incoming -- fBufferedData is a buffer containing any incomplete packets that 
//   we've received from the client.
//
// o outgoing -- fPendingSends and fPendingSendOffset control the packets that 
//   are waiting to be sent to the client.  This list might get long if the 
//   client stops listening to us.  Packets will first back up in the UNIX 
//   domain socket's socket buffer.  Once that fills up we won't be able to 
//   write to the socket anymore.  We respond to that by buffering the 
//   packets on the fPendingSends list.  We also tell CFSocket to let us know 
//   (with a kCFSocketWriteCallBack event) if space becomes available.
//
//   At this point one of two things will happen.  Either we'll buffer too 
//   many packets for the client (kClientMaximumPendingSends) in which case 
//   we'll kill the client.  Or the client will start receiving packets again, 
//   which will start to empty the socket buffer.  CFSocket will tells about 
//   this by sending us a kCFSocketWriteCallBack event, and we'll start 
//   pulling packets off the fPendingSends list and writing them to the socket.

enum {
    kClientStateMagic = 'LSCM'               // for Local Server Client Magic
};

struct ClientState {
    OSType              fMagic;             // kClientStateMagic
    int                 fSockFD;            // UNIX domain socket to client
    CFSocketRef         fSockCF;            // CFSocket wrapper for the above
    CFRunLoopSourceRef  fRunLoopSource;     // runloop source for the above
    CFMutableDataRef    fBufferedData;      // buffers data for incomplete incoming packets
    CFMutableArrayRef   fPendingSends;      // list of packets waiting to be sent
    size_t              fPendingSendOffset; // offset of next byte to send in first packet on list
    Boolean             fListening;         // true if this client is a listener
};
typedef struct ClientState ClientState;

// To prevent a deaf client from sucking down all of our memory, we limit the 
// number of packets that we'll buffer for a given client.  If the length of 
// fPendingSends exceeds kClientMaximumPendingSends, we'll kill the client rather 
// than queue more data.

enum {
    kClientMaximumPendingSends = 100
};

// gClients is a set of all clients we know about.

static CFMutableSetRef gClients = NULL;		// of (ClientState *)

#pragma mark Misc

static Boolean ClientCheckPacketSize(ClientState *client, const PacketHeader *packet, size_t requiredSize)
    // Checks that a packet that has arrived from a client is of the 
    // appropriate size.  Returns false, and prints a message, if it isn't.
{
    Boolean result;
    
    assert(client != NULL);
    assert(packet != NULL);
    assert(requiredSize >= sizeof(PacketHeader));
    
    result = true;
    if (packet->fSize != requiredSize) {
        fprintf(
            stderr, 
            "ClientCheckPacketSize: Client %p sent us a '%.4s' of the wrong size (got %" PRIu32 ", wanted %zu).\n", 
            client, 
            (char *) &packet->fType, 
            packet->fSize, 
            requiredSize
        );
        result = false;
    }
    return result;
}

static Boolean ClientCheckPacketID(ClientState *client, const PacketHeader *packet, int32_t requiredID)
    // Checks that a packet that has arrived from a client has the 
    // correct ID.  Returns false, and prints a message, if it doesn't.
{
    Boolean result;

    assert(client != NULL);
    assert(packet != NULL);
    
    result = true;
    if (packet->fID != requiredID) {
        fprintf(
            stderr, 
            "ClientCheckPacketID: Client %p sent us a '%.4s' with the wrong ID (got %" PRId32 ", wanted %" PRId32 ").\n", 
            client, 
            (char *) &packet->fType, 
            packet->fID, 
            requiredID
        );
        result = false;
    }
    return result;
}

// Forward declarations

static void ClientGotSpace(ClientState *client);
static void ClientGotData(ClientState *client, const void *data);

static void ClientEvent(
    CFSocketRef             s, 
    CFSocketCallBackType    type, 
    CFDataRef               address, 
    const void *            data, 
    void *                  info
)
	// This is the CFSocket event callback for client sockets.  For a description 
	// of the parameters, see the CFSocket documentation.
	//
	// This routine responds to two events, kCFSocketDataCallBack and 
	// kCFSocketWriteCallBack, dispatching them to ClientGotData and 
	// ClientGotSpace, respectively.
{
	ClientState *	client;
	
	(void)address;
    assert(s != NULL);

    client = (ClientState *) info;
    assert(client != NULL);
    assert(client->fMagic == kClientStateMagic);
    
    switch (type) {
        case kCFSocketDataCallBack:
            ClientGotData(client, data);
            break;
        case kCFSocketWriteCallBack:
            ClientGotSpace(client);
            break;
        default:
            assert(false);
            break;
    }
}

#pragma mark Create/Destroy

static void ClientDestroy(ClientState *client);

static int ClientInitialise(void)
	// Initialises the client management layer, which simply involves 
	// creating an empty gClients set.
{
	int err;
	
	err = 0;
	gClients = CFSetCreateMutable(NULL, 0, NULL);
	if (gClients == NULL) {
		err = ENOMEM;
	}
	return err;
}

static void ClientTerminate(void)
	// Shuts down the client management layer.  This involves destroying 
	// any remaining clients and disposing of gClients.
{
	CFIndex             clientCount;
	CFIndex             clientIndex;
	ClientState **      allClients;

    if (gClients != NULL) {
		// Can't use CFSetApplyFunction because the ClientDestroy modifies 
		// the gClients set.
		
		clientCount = CFSetGetCount(gClients);
		
		allClients = calloc(clientCount, sizeof(ClientState *));
		if (allClients == NULL) {
			fprintf(stderr, "CFLocalServer: Could not clean up clients because we couldn't allocate memory.\n");
		} else {
			CFSetGetValues(gClients, (const void **) allClients);
			
			for (clientIndex = 0; clientIndex < clientCount; clientIndex++) {
				fprintf(stderr, "CFLocalServer: Client %p killed because we're quitting.\n", allClients[clientIndex]);

				ClientDestroy( allClients[clientIndex] );
			}
		}

		free(allClients);
		
		CFRelease(gClients);
		gClients = NULL;
    }
}

static int ClientCreate(int clientSockFD, ClientState **clientPtr)
    // Creates a new client that communicates over clientSockFD. 
    // If clientPtr is not NULL, it returns a pointer to the 
    // client state record in *clientPtr.
    //
    // clientSockFD must be a valid file descriptor referencing a 
    // socket that's connected to the client
    // On input, if clientPtr is not NULL, *clientPtr must be NULL
    // Returns an errno-style error code
    // On success, if clientPtr is not NULL, *clientPtr will not be NULL
    // On success, clientSockFD is owned by the new client; the caller 
    // need not close it
    // On error, if clientPtr is not NULL, *clientPtr will be NULL
    // On error, clientSockFD will have been closed.
    //
    // IMPORTANT:
    // Regardless of whether this routine succeeds or fails, it assumes 
    // responsibility for clientSockFD.  The caller is never required to 
    // close it.
{
    int             err;
    int             junk;
    ClientState *   client;
    
    assert( (clientPtr == NULL) || (*clientPtr == NULL) );
    
    assert(gClients != NULL);
    
    // Create the client state record.
    
    err = 0;
    client = (ClientState *) calloc(1, sizeof(*client));
    if (client == NULL) {
        err = ENOMEM;
    }

    // Fill in the easy fields.  This also prepares us for the clean up 
    // on failure.
    
    if (err == 0) {
        client->fMagic = kClientStateMagic;

        // For clean up to work properly, we must make sure that, if 
        // the connection record is allocated successfully, we always 
        // set fSockFD to the incoming clientSockFD.

        client->fSockFD = clientSockFD;
        
        client->fBufferedData = CFDataCreateMutable(NULL, 0);
        client->fPendingSends = CFArrayCreateMutable(NULL, 0, NULL);

        if ( (client->fBufferedData == NULL) || (client->fPendingSends == NULL) ) {
            err = ENOMEM;
        }
    }
    
    // Make the socket non-blocking.  We need to do this because 
    // otherwise ClientSendPending can get stuck in a write.
    
    if (err == 0) {
        err = MoreUNIXSetNonBlocking(client->fSockFD);
    }
    
    // Wrap the socket in a CFSocket, and create and install the run loop source.
    
    if (err == 0) {
        CFSocketContext context;
        
        memset(&context, 0, sizeof(context));
        context.info = client;
        
        client->fSockCF = CFSocketCreateWithNative(
            NULL, 
            (CFSocketNativeHandle) client->fSockFD, 
            kCFSocketDataCallBack + kCFSocketWriteCallBack, 
            ClientEvent, 
            &context
        );
        if (client->fSockCF == NULL) {
            err = EINVAL;
        }
    }

    if (err == 0) {
        client->fRunLoopSource = CFSocketCreateRunLoopSource(NULL, client->fSockCF, 0);
        if (client->fRunLoopSource == NULL) {
            err = EINVAL;
        }
    }
    if (err == 0) {
        CFRunLoopAddSource( CFRunLoopGetCurrent(), client->fRunLoopSource, kCFRunLoopDefaultMode);
        
        assert( ! CFSetContainsValue(gClients, client) );
        
        // It's all good.  Record that this client exists.
        
        CFSetAddValue(gClients, client);
    }
    
    // Clean up.
    
    if (err != 0) {
        fprintf(stderr, "ClientCreate: Error %d creating client.\n", err);

        // If client is NULL, we couldn't allocate a client record, therefore 
        // we had nowhere to record clientSockFD, therefore ClientDestroy won't 
        // clean it up.  Thus, we have to do it ourselves.
        
        if (client == NULL) {
            junk = close(clientSockFD);
            assert(junk == 0);
        } else {
            ClientDestroy(client);
        }
        client = NULL;
    }
    if (clientPtr != NULL) {
        *clientPtr = client;
    }
    
    assert( (clientPtr == NULL) || ((err == 0) == (*clientPtr != NULL)) );

    return err;
}

static void ClientDestroy(ClientState *client)
    // Destroys a client.  This is called in a number of different circumstances, 
    // but these basically boil down to:
    // 
    // a) if ClientCreate fails, it's called to destroy the partially-created client, 
    // b) if some sort of communications error happens, it's called to destroy the 
    //    client,
    // c) if the client sends us a goodbye packet, this is called to destroy the client, and 
    // d) on quit, all clients are destroyed.
{
    int     junk;
    
    assert(client != NULL);
    assert(client->fMagic == kClientStateMagic);

    // This following assert is NOT true.  If the client dies before it 
    // gets fully started (that is, we get an error halfway through 
    // ClientCreate), ClientDestroy is called to tidy up the mess but 
    // the client hasn't been added into gClients yet.
    
    // assert( CFSetContainsValue(gClients, client) );
    
    // Remove the client our record of existant clients.
    
    CFSetRemoveValue(gClients, client);
    
    // Clean up the runloop source and CFSocket.
    
    if (client->fRunLoopSource != NULL) {
        CFRunLoopSourceInvalidate(client->fRunLoopSource);
        
        CFRelease(client->fRunLoopSource);
    }
    if (client->fSockCF != NULL) {
        CFSocketInvalidate(client->fSockCF);
        
        CFRelease(client->fSockCF);
    }
    
    // Close the socket itself, but only if we don't have a corresponding 
    // CFSocket; if a CFSocket was created, it takes over responsibility 
    // for closing the socket.
    
    if ( (client->fSockFD != -1) && (client->fSockCF == NULL) ) {
        junk = close(client->fSockFD);
        assert(junk == 0);
    }
    
    // Free any packets waiting to go out to this client.
    
    if (client->fPendingSends != NULL) {
        CFIndex index;
        CFIndex count;

        count = CFArrayGetCount(client->fPendingSends);
        for (index = 0; index < count; index++) {
            free( (void *) CFArrayGetValueAtIndex(client->fPendingSends, index) );
        }
        CFRelease(client->fPendingSends);
    }
    
    // Free any buffered data from this client.
    
    if (client->fBufferedData != NULL) {
        CFRelease(client->fBufferedData);
    }
    
    // Free the client state record itself.
    
    client->fMagic = 'FRE!';
    free(client);
}

#pragma mark Send

static int ClientSendPending(ClientState *client)
    // This routine attempts to send any packets that are queued in the fSendPending 
    // array.  It is somewhat complex.  There are three possible final results.
    //
    // o It successfully sends all packets in the queue.  In this case it returns 0.
    //
    // o The write side of the socket is full (flow controlled).  In this case the 
    //   function enables the socket write callback (kCFSocketWriteCallBack, using 
    //   CFSocketEnableCallBacks) and returns 0.  When socket buffer empties a little, 
    //   CFSocket will send us the kCFSocketWriteCallBack event and we'll resume sending.
    //
    // o It fails for some other reasons (for example, the other end of the socket has 
    //   been closed, causing an EPIPE).  In this case it returns an errno-style error 
    //   indicating the failure.  The caller typically responds by destroying the client.
    //
    // This whole process is further complicated by the possibilty that the socket 
    // buffer might have enough space for half a packet.  In this case you'll get a 
    // short write, that is, write will return a positive number less than its nbytes 
    // parameter.  To handle this case we record the offset into the packet of the first 
    // byte of unwritten data.  When we go to send a packet, we always send from there. 
    // When write accepts some data, we bump the offset by that amount.  If that 
    // completes the send of the packet, we start on next packet, resetting the offset 
    // back to 0.
{
    int                     err;
    Boolean                 done;
    const PacketHeader *    thisPacket;
    ssize_t                 bytesWritten;
    
    err = 0;
    
    // Keep going until we've sent all pending packets for this client.
    
    while ( (err == 0) && (CFArrayGetCount(client->fPendingSends) != 0) ) {
        thisPacket = (const PacketHeader *) CFArrayGetValueAtIndex(client->fPendingSends, 0);
        
        // Try to send this packet by writing it to the socket.
        
        done = false;
        do {
            bytesWritten = write(
                client->fSockFD, 
                ((char *) thisPacket) + client->fPendingSendOffset, 
                thisPacket->fSize - client->fPendingSendOffset
            );
            
            if (bytesWritten > 0) {
                // We're written some bytes.  Adjust fPendingSendOffset by 
                // that amount and see if that completes the packet.
                
                client->fPendingSendOffset += bytesWritten;
                
                if (client->fPendingSendOffset == thisPacket->fSize) {
                    // Packet complete.  Delete it from the head of the 
                    // send list, reset offset back to 0, and let's go 
                    // deal with the next packet.
                    
                    CFArrayRemoveValueAtIndex(client->fPendingSends, 0);
                    free( (void *) thisPacket);
                    client->fPendingSendOffset = 0;
                    done = true;
                } else {
                    // Packet still not fully sent.  The send offset has already 
                    // been updated, so we just loop to try again.
                }
            } else if (bytesWritten == -1) {
                // We got some sort of error.
                
                err = errno;
                switch (err) {
                    case EINTR:
                        // Interrupted.  Do nothing, so we loop and retry this 
                        // send immediately.

                        err = 0;
                        break;
                    case EAGAIN:
                        // Flow controlled.  Break out of the loop with an EAGAIN 
                        // error; we try again when space becomes available.

                        fprintf(stderr, "ClientSendPending: Client %p write-side flow control.\n", client);
                        
                        // Tell the CFSocket that we now /really/ need to be told about 
                        // write space becoming available.  Without this we'll never 
                        // recover if the client stops accepting messages temporarily 
                        // (which causes the socket buffer to fill up and us to get an 
                        // EAGAIN) and then starts accepting messages again.  At that 
                        // point the client will drain the socket buffer, but we'll never 
                        // hear about it because CFSocket doesn't know that we care 
                        // about write space.  With this call CFSocket knows that we 
                        // care, and will send us an kCFSocketWriteCallBack event if 
                        // space becomes available in the socket buffer.
                        
                        CFSocketEnableCallBacks(client->fSockCF, kCFSocketWriteCallBack);
                        break;
                    default:
                        // Errored.  Our response is typically draconian: 
                        // we return the error to our caller, which then kills the client 
                        // completely.
                    
                        fprintf(stderr, "ClientSendPending: Client %p killed because of send error (%d).\n", client, err);
                        break;
                }
            } else {
                assert(false);
            }
        } while ( (err == 0) && ! done );
    }
    
    // As far as the caller is concerned, write-side flow control is not an error.
    
    if (err == EAGAIN) {
        err = 0;
    }
    
    return err;
}

static Boolean ClientSend(ClientState *client, const PacketHeader *packet)
    // Called in various places to send a packet to a client.  
    // This adds it to the send queue and then calls ClientSendPending 
    // to attempt a send.
{
    Boolean         result;
    PacketHeader *  copiedPacket;
    
    assert(client != NULL);
    assert(packet != NULL);
    assert(packet->fSize >= sizeof(PacketHeader));

    // If we've buffered kClientMaximumPendingSends already, the client is 
    // just not reading them.  To avoid us consuming all of our memory buffering 
    // packets for a deaf client, we just kill the client.
    
    result = true;
    if ( CFArrayGetCount(client->fPendingSends) >= kClientMaximumPendingSends ) {
        fprintf(stderr, "ClientSend: Client %p killed because of too many outstanding sends.\n", client);
        
        result = false;
    }

    // Copy the packet data, append that copy to the send queue, and then 
    // give it a kick.
    //
    // The memory allocated here will be freed when the packet is succesfully 
    // sent (SendPending), or the client is destroy.
    
    if (result) {
        copiedPacket = (PacketHeader *) malloc(packet->fSize);
        result = (copiedPacket != NULL);
    }
    if (result) {
        memcpy(copiedPacket, packet, packet->fSize);
        
        CFArrayAppendValue(client->fPendingSends, copiedPacket);
        
        result = ( ClientSendPending(client) == 0 );
    }

    return result;
}

static Boolean ClientSendReply(ClientState *client, const PacketHeader *request, int errNum)
    // Send an RPC reply packet to the client.  You must supply request 
    // because it forms the basis of many of the fields in the reply. 
    // You also have to supply errNum, which is an errno-style error 
    // indicating the fate of the request.
{
    PacketReply     response;

    assert(client  != NULL);
    assert(request != NULL);

    InitPacketHeader(&response.fHeader, kPacketTypeReply, sizeof(response), false);
    // Copy the ID from the request packet, overriding the fID set by InitPacketHeader.
    response.fHeader.fID    = request->fID;
    response.fErr = errNum;

    return ClientSend(client, &response.fHeader);
}

static void ClientGotSpace(ClientState *client)
	// This routine is called by ClientEvent when it receives the kCFSocketWriteCallBack 
	// event, indicating that there is space to write in the client's socket buffer. 
	// It calls ClientSendPending to process any packets that are waiting to be sent.  
	// In most cases this does nothing because the client send queue is empty.  However, 
	// if the client goes deaf, so the socket buffer becomes write-side flow controlled, 
	// packets can back up in the send queue.  When the client starts receiving packets 
	// again, space becomes available in the socket buffer and CFSocket sends us the 
	// kCFSocketWriteCallBack.  We respond to that by resuming our sends.
{
    int             err;
	
	assert(client != NULL);
    
    fprintf(stderr, "ClientGotSpace: Client %p lifted write-side flow control.\n", client);

    err = ClientSendPending(client);
	
	// If the sending failed for any reason (except flow control, for which 
	// ClientSendPending mutates the EAGAIN status to a 0) we kill the client.
	
    if (err != 0) {
        ClientDestroy(client);
    }
}

#pragma mark Receive

// The receive engine is based around ClientGotData, which is the routine that gets 
// called when new data arrives, and a variety of packet handlers for processing 
// specific types of packets and that all have the same form.
//
// A packet handle routine takes two parameters, the client and the packet, neither 
// of which can be NULL, and does the work to process that packet.  This typically 
// involves checking that the packet is valid, doing the job requested by the packet, 
// and then, if the packet is for an RPC, sending the reply.
//
// If the packet handler returns false, the caller (ClientGotData) assumes that 
// something was seriously wrong with the packet and kills the connection to the 
// client.  A packet handler typically does this if the packet itself is malformed; 
// if the job requested by the packet can't be done (for example, there might not 
// be enough memory), the packet handler wouldn't return false but would, instead, 
// send an error status back to the client in the RPC reply.

static Boolean ClientGoodbye(ClientState *client, PacketGoodbye *packet)
    // A packet handler for the Goodbye packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A Goodbye packet is sent by the client to indicate to us that it's closing 
	// its end of the connection.
{
    Boolean     result;
    
    assert(client != NULL);
    assert(packet != NULL);
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketGoodbye));
    if ( result ) {
        result = ClientCheckPacketID(client, &packet->fHeader, kPacketIDNone);
    }
    
    if (result) {
        // During reliability print all of the goodbyes proved to be too verbose, 
        // so I've disabled it for now.
        
        if (false) {
            fprintf(stderr, "%p: Goodbye (%.*s).\n", client, (int) sizeof(packet->fMessage), packet->fMessage);
        }
        
        // Unlike most packet handlers, we return false on success.  This is because 
		// the Goodbye packet tells us that the client has gone away, and thus we 
		// need to kill the client.  It turns out that returning false does the job 
		// without us having to write any special code.
        
        result = false;
    }
    
    return result;
}

static Boolean ClientNOP(ClientState *client, PacketNOP *packet)
    // A packet handler for the NOP packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A NOP RPC does nothing; it's used to test client/server connection.
{
    Boolean     result;
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketNOP));
    
    if (result) {
        fprintf(stderr, "%p: NOP\n", client);
        
        result = ClientSendReply(client, &packet->fHeader, 0);
    }
    
    return result;
}

static Boolean ClientWhisper(ClientState *client, PacketWhisper *packet)
    // A packet handler for the Whisper packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A Whisper RPC causes the server to print the associated message.
{
    Boolean result;
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketWhisper));
    if (result) {
        fprintf(stderr, "%p: Whisper \"%.*s\"\n", client, (int) sizeof(packet->fMessage), packet->fMessage);

        result = ClientSendReply(client, &packet->fHeader, 0);
    }
    
    return result;
}

static Boolean ClientShout(ClientState *client, PacketShout *packet)
    // A packet handler for the Shout packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A Shout packet causes the server to echo the message (in the form 
	// of a Shout packet) to every client that has registered as a listener.
{
    Boolean     result;
    Boolean     sendResult;
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketShout));
    if (result) {
        result = ClientCheckPacketID(client, &packet->fHeader, kPacketIDNone);
    }
	
	// The Shout packet is good.  Let's echo it to each listener.
	
    if (result) {
        ClientState  ** allClients;
        CFIndex         clientCount;
        CFIndex         clientIndex;

        fprintf(stderr, "%p: Shout   \"%.*s\"\n", client, (int) sizeof(packet->fMessage), packet->fMessage);
        
        // We make a snapshot of the client list because clients might disappear 
        // as we talk to them.  That is, the act of talking to the client might 
        // cause us to notice that the client is dead.
        
        clientCount = CFSetGetCount(gClients);
        
        allClients = calloc(clientCount, sizeof(ClientState *));
        if (allClients == NULL) {
            fprintf(stderr, "ClientShout: Shout from %p failed because we couldn't allocate memory.\n", client);
        } else {
            CFSetGetValues(gClients, (const void **) allClients);
            
			// Iterate through the array of clients, sending the Shout packet to each.
			
            for (clientIndex = 0; clientIndex < clientCount; clientIndex++) {
                ClientState *  thisClient;
                
                thisClient = allClients[clientIndex];
                if (thisClient->fListening) {
                    sendResult = ClientSend(thisClient, &packet->fHeader);
                    
                    // Fun fun fun.  If we're sending to ourselves, we return the result 
                    // of the send as our function result, which, if there's a failure, 
                    // will trigger ClientGotData to clean us up.  OTOH, if we're sending 
                    // to another client, we're responsible for cleaning up if there's 
                    // a failure.
                    
                    if (thisClient == client) {
                        result = sendResult;
                    } else {
                        if ( ! sendResult ) {
                            fprintf(stderr, "ClientShout: Shout from %p to %p failed.\n", client, thisClient);
                            
                            ClientDestroy(thisClient);
                        }
                    }
                }
            }
        }

        free(allClients);
    }
    return result;
}

static Boolean ClientListen(ClientState *client, PacketListen *packet)
    // A packet handler for the Listen packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A Listen RPC tells the server that the client wants to hear about shouted 
	// messages.
{
    Boolean     result;
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketListen));
    if (result) {
        if (client->fListening) {
            fprintf(stderr, "ClientListen: Redundant Listen from %p.\n", client);
        } else {
            fprintf(stderr, "%p: Listen\n", client);
        }
        client->fListening = true;
        
        result = ClientSendReply(client, &packet->fHeader, 0);
    }
    
    return result;
}

static Boolean ClientQuit(ClientState *client, PacketQuit *packet)
    // A packet handler for the Quit packet.  See the large comment above for 
	// a discussion of the general form of a packet handler.
	//
	// A Quit RPC causes the server to quit.
{
    Boolean     result;
    
    result = ClientCheckPacketSize(client, &packet->fHeader, sizeof(PacketQuit));
    if (result) {
        fprintf(stderr, "%p: Quit\n", client);
        
        // Stop the main event loop.
        
        CFRunLoopStop( CFRunLoopGetCurrent() );
        
        // This reply should go out immediately.  If the client, for some reason, 
        // is flow controlled, it may not see the response.  But really, that's 
        // the client's fault (-:
        
        result = ClientSendReply(client, &packet->fHeader, 0);
    }
    
    return result;
}

static void ClientGotData(ClientState *client, const void *data)
	// This routine is called by ClientEvent when it receives the kCFSocketDataCallBack 
	// event, indicating that CFSocket has read data from the socket.  The routine 
	// appends the data to the receive buffer (fBufferedData) and then looks through 
	// the receive buffer for complete packets.  For each complete packet it finds, 
	// it calls the packet handler (the various routines above) to process the packet 
	// and then it deletes the packet from the front of the receive buffer.
	// 
	// data is the data read for us by CFSocket.  It's actually a CFDataRef 
	// but, because we're being called from a generic CFSocket event handler, 
	// it's of type (const void *).  We have to do the cast here.
{
    CFDataRef       newData;

	assert(client != NULL);
    
    newData = (CFDataRef) data;
    assert(newData != NULL);
    assert( CFGetTypeID(newData) == CFDataGetTypeID() );

    if ( CFDataGetLength(newData) == 0 ) {
        // A zero length data indicates the end of the data stream; the client is dead 
		// so we just go and remove our record of it.
        
        fprintf(stderr, "ClientGotData: Client %p died unexpectedly.\n", client);
        
        ClientDestroy(client);
    } else {

        // Append the new data to whatever data we have already buffered 
        // (most likely nothing).
        
        CFDataAppendBytes(client->fBufferedData, CFDataGetBytePtr(newData), CFDataGetLength(newData));
        
        // Process packets until we run out of complete ones.
        
        do {
            PacketHeader *  thisPacket;
            Boolean         success;
            
            if ( CFDataGetLength(client->fBufferedData) < sizeof(PacketHeader) ) {
                // Not enough data for the packet header; we're done.
                break;
            }
            
            thisPacket = (PacketHeader *) CFDataGetBytePtr(client->fBufferedData);
            
            if ( thisPacket->fMagic != kPacketMagic ) {
                fprintf(stderr, "ClientGotData: Client %p sent us a packet with bad magic (%.4s).\n", client, (char *) &thisPacket->fMagic);
                
                ClientDestroy(client);
                break;
            }
            
            if (thisPacket->fSize > kPacketMaximumSize) {
                fprintf(stderr, "ClientGotData: Client %p sent us a packet that's just too big (%" PRIu32 ").\n", client, thisPacket->fSize);
                
                ClientDestroy(client);
                break;
            }
            
            if ( CFDataGetLength(client->fBufferedData) < thisPacket->fSize ) {
                // Not enough data for the packet body; we're done.
                break;
            }
			
			// Dispatch to the appropriate packet handler.
            
            switch (thisPacket->fType) {
                case kPacketTypeGoodbye:
                    success = ClientGoodbye(client, (PacketGoodbye *) thisPacket);
                    break;
                case kPacketTypeNOP:
                    success = ClientNOP(client, (PacketNOP *) thisPacket);
                    break;
                case kPacketTypeWhisper:
                    success = ClientWhisper(client, (PacketWhisper *) thisPacket);
                    break;
                case kPacketTypeShout:
                    success = ClientShout(client, (PacketShout *) thisPacket);
                    break;
                case kPacketTypeListen:
                    success = ClientListen(client, (PacketListen *) thisPacket);
                    break;
                case kPacketTypeQuit:
                    success = ClientQuit(client, (PacketQuit *) thisPacket);
                    break;
                default:
                    fprintf(stderr, "ClientGotData: Client %p sent us a packet with an unexpected type (%.4s).\n", client, (char *) &thisPacket->fType);
                    
                    success = false;
                    break;                
            }
            if ( ! success ) {
                ClientDestroy(client);
                break;
            }
            
            // Delete this packet from the front of our packet buffer.
            
            CFDataDeleteBytes(client->fBufferedData, CFRangeMake(0, thisPacket->fSize));
            
        } while (true);
    }
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Debug Infrastructure

static void ClientPrintInfo(const void *value, void *context)
	// Called by PrintServerState to print the state of a particular 
	// client.  Actually passed as a callback to CFSetApplyFunction, 
	// which is why the value parameter is a (const void *) rather 
	// than a (ClientState *).  context is not used in this... context (-;
{
    ClientState *  client;
    static const char * kBoolToStr[2] = { "false", "true" };

	(void)context;
    client = (ClientState *) value;
    assert(client != NULL);
    assert(client->fMagic == kClientStateMagic);
    
    fprintf(stderr, "  Client %p:\n", client);
    fprintf(stderr, "    fSockFD            = %d\n", client->fSockFD);
    fprintf(stderr, "    fSockCF            = %p\n", client->fSockCF);
    fprintf(stderr, "    fRunLoopSource     = %p\n", client->fRunLoopSource);
    fprintf(stderr, "    fBufferedData      = %p (count: %ld)\n", client->fBufferedData, CFDataGetLength(client->fBufferedData));
    fprintf(stderr, "    fPendingSends      = %p (count: %ld)\n", client->fPendingSends, CFArrayGetCount(client->fPendingSends));
    fprintf(stderr, "    fPendingSendOffset = %zd\n", client->fPendingSendOffset);
    fprintf(stderr, "    fListening         = %s\n", kBoolToStr[client->fListening]);    
}

static void PrintServerState(void)
	// Called in response to a SIGUSR1.  This prints a bunch of state 
	// information about the server.  Note that it is not called from a 
	// signal handler directly, rather from SignalRunLoopCallback which 
	// is a runloop callback.  So we can do all sorts of things that 
	// aren't safe in a true signal handler.
{
    fprintf(stderr, "CFLocalServer State\n");
    fprintf(stderr, "-------------------\n");
    if ( CFSetGetCount(gClients) == 0) {
        fprintf(stderr, "Clients: none\n");
    } else {
        fprintf(stderr, "Clients:\n");
        CFSetApplyFunction(gClients, ClientPrintInfo, NULL);
    }
    fprintf(stderr, "\n");
	
    DebugPrintDescriptorTable();
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Server Framework

static void ListeningSocketAcceptCallback(
    CFSocketRef             s, 
    CFSocketCallBackType    type, 
    CFDataRef               address, 
    const void *            data, 
    void *                  info
)
	// This is the CFSocket event callback for the listening socket.  For a 
	// description of the parameters, see the CFSocket documentation.
	//
	// CFSocket calls this routine when it has accepted a new connection on 
	// the socket.  in this case data is a pointer to the newly created 
	// file descriptor that describes the new connection.  This routine 
	// responds by calling into the client layer to create a new client.
{
    (void)s;
	(void)address;
	(void)info;
    
    assert(type == kCFSocketAcceptCallBack);
    assert(   (int *) data  != NULL );
    assert( (*(int *) data) != -1 );

    (void) ClientCreate( (*(int *) data), NULL );
    
    // If ClientCreate fails, it cleans up after itself, including 
    // closing the newly created client socket.  It's even printed 
    // a happy message (well, an unhappy message).  So we do nothing 
	// on failure.
}

#if !defined(_WIN32)
static void SignalRunLoopCallback(const siginfo_t *sigInfo, void *refCon)
    // This routine is called in response to a signal (SIGINT 
	// or SIGUSR1).  It is not, however, a signal handler.  Rather, 
	// we orchestrate to have it called from the runloop (via 
    // the magic of InstallSignalToSocket).  It's purpose 
    // is to a) stop the server when the user types ^C (SIGINT), or 
	// b) print some information about the server (SIGUSR1).
{
	(void)sigInfo;
	(void)refCon;
    
    switch (sigInfo->si_signo) {
        case SIGUSR1:
			// Respond to SIGUSR1 by printing some information about the server.
			
            PrintServerState();
            break;
        case SIGINT:
			// Respond to SIGINT by stopping the server.
			
			// Stop the runloop.  Note that we can get a reference to the runloop by 
			// calling CFRunLoopGetCurrent because this is called from the runloop.
			
			CFRunLoopStop( CFRunLoopGetCurrent() );
			
			// Print a bonus newline to ensure that the next command prompt isn't 
			// printed on the same line as the echoed ^C.
			
			fprintf(stderr, "\n");
            break;
        default:
            assert(false);
            break;
    }
}
#endif

static int SafeBindUnixDomainSocket(int sockFD, const char *socketPath)
	// This routine is called to safely bind the UNIX domain socket 
	// specified by sockFD to the path specificed by socketPath.  To avoid 
	// security problems, socketPath must point it to a sticky directory 
	// (such as "/var/tmp").  This allows us to create the socket with 
	// very specific permissions, without us having to worry about a malicious 
	// process switching stuff out from underneath us.
	//
	// For this test program, socketpath is "/var/tmp/com.apple.dts.CFLocalServer/Socket". 
	// The code calculates parentPath as ""/var/tmp/com.apple.dts.CFLocalServer" 
	// and grandParentPath as "/var/tmp".  Each ancestor has certain key attributes. 
	//
	// o grandParentPath must a sticky directory.  Because it's sticky, we 
	//   can create a directory within it and know that either a) we created 
	//   the directory, and no one else can mess with it because it's sticky, 
	//   or b) the directory exists, in which case we can check it's owner 
	//   and permissions and, if they are set correctly, know that no one else 
	//   can mess with it.
	//
	// o When we create the parentPath directory within grandParentPath, we set its 
	//   permissions to make it readable by everyone (so everyone can connect to our 
	//   server) but writeable only by us (so that only we can create the listening 
	//   socket).  Because parentPath is set this way, we know that no one else 
	//   can modify it to produce a security problem.
	//
	// IMPORTANT:
	// This routine is designed to protect against external attack, not against 
	// being called incorrectly.  It only does minimal checking of socketPath.  
	// For example, if one of the components of socketPath was "..", the security 
	// checking done by this routine might be invalid.  Do not pass an untrusted 
	// socketPath to this routine.
{
    int                 err;
    char *              parentPath;
    char *              grandParentPath;
    char *              lastSlash;
    struct stat         sb;
    struct sockaddr_un  bindReq;
#if !defined(_WIN32)
    static const mode_t kRequiredParentMode = S_IRWXU | (S_IRGRP | S_IXGRP) | (S_IROTH | S_IXOTH); // rwxr-xr-x
#endif

    parentPath      = NULL;
    grandParentPath = NULL;
    
    // sockaddr_un can only hold a very short path (it's 104 bytes long), 
    // so we check that limit right up front.  Note the use of >= in the 
    // check below: we fail if socketPath is exactly 104 chars long because 
    // that would leave no space for the trailing null character.  Looking at 
    // the kernel code, I don't think this is strictly necessary (in fact, 
    // it seems that the kernel code will handle much longer paths than sun_path, 
    // up to an overall sockaddr size ofSOCK_MAXADDRLEN), but I'm being 
    // paranoid.
    
    err = 0;
    if (strlen(socketPath) >= sizeof(bindReq.sun_path)) {
        err = EINVAL;
    }
    
    // Construct parentPath and grandParent path by knocking path components 
    // off the end.
    
    if (err == 0) {
        parentPath = strdup(socketPath);
        if (parentPath == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        lastSlash = strrchr(parentPath, '/');
        if (lastSlash == NULL) {
            fprintf(stderr, "SafeBindUnixDomainSocket: Can't get parent for path (%s).\n", socketPath);
            err = EINVAL;
        } else {
            *lastSlash = 0;
        }
    }
    if (err == 0) {
        grandParentPath = strdup(parentPath);
        if (grandParentPath == NULL) {
            err = ENOMEM;
        }
    }
    if (err == 0) {
        lastSlash = strrchr(grandParentPath, '/');
        if (lastSlash == NULL) {
            fprintf(stderr, "SafeBindUnixDomainSocket: Can't get grandparent for path (%s).\n", socketPath);
            err = EINVAL;
        } else {
            *lastSlash = 0;
        }
    }
    
    // Check that the parent directory is a sticky root-owned directory.  If the 
    // grandparent directory is sticky, we know that any items in that directory 
    // that are owned by us can't be substituted by anyone else (that is: deleted, 
    // moved or renamed, and then replaced by an attacker's item).
    
    if (err == 0) {        
        err = stat(grandParentPath, &sb);
        err = MoreUNIXErrno(err);
    }
#if !defined(_WIN32)
    if ( (err == 0) && ( ! (sb.st_mode & S_ISVTX) || (sb.st_uid != 0) ) ) {
        fprintf(stderr, "SafeBindUnixDomainSocket: Grandparent directory (%s) is not a sticky root-owned directory.\n", grandParentPath);
        err = EINVAL;
    }
    
    // Create the parent directory.  Ignore an EEXIST error because of the 
    // next check.

    if (err == 0) {
        err = mkdir(parentPath, kRequiredParentMode);
        err = MoreUNIXErrno(err);
        
        if (err == EEXIST) {
            err = 0;
        }
    }
#endif
    
    // Check that the parent directory is a directory, is owned by us, and 
    // has the right mode.  This ensures that no one except us can be monkeying 
    // with its contents.  And we know that no one can substitute a /different/ 
    // directory underneath us because its parent (grandParentPath) is sticky.

    if (err == 0) {
        err = stat(parentPath, &sb);
        err = MoreUNIXErrno(err);
    }
#if !defined(_WIN32)
    if ( (err == 0) && (sb.st_uid != geteuid()) ) {
        fprintf(stderr, "SafeBindUnixDomainSocket: Parent (%s) is not owned by us.\n", parentPath);
        err = EINVAL;
    }
    if ( (err == 0) && ! S_ISDIR(sb.st_mode) ) {
        fprintf(stderr, "SafeBindUnixDomainSocket: Parent (%s) is not a directory.\n", parentPath);
        err = EINVAL;
    }
    if ( (err == 0) && ( (sb.st_mode & ACCESSPERMS) != kRequiredParentMode ) ) {
        fprintf(stderr, "SafeBindUnixDomainSocket: Parent (%s) has wrong permissions.\n", parentPath);
        err = EINVAL;
    }
#endif
    
    // If all is well, let's bind our socket.  This involves deleting any existing 
    // socket and recreating our own.  We know we can do this without worrying 
    // about substitution because only we have write access to the parent directory.

    if (err == 0) {
        mode_t              oldUmask;

        // Temporarily set the umask to 0 (the default is 0022) so that the 
        // socket is created rwxrwxrwx.  This allows any user to connect to 
        // our socket.

        oldUmask = umask(0);

        // Delete any existing socket.  We delete the socket when we shut down, 
        // but, if we quit unexpectedly, it could've been left lying around.
        
        (void) unlink(socketPath);

        // Bind the socket, allowing other clients to connect.
        
        bindReq.sun_family = AF_UNIX;
        strcpy(bindReq.sun_path, socketPath);

        err = bind(sockFD, (struct sockaddr *) &bindReq, SUN_LEN(&bindReq));
        err = MoreUNIXErrno(err);

        (void) umask(oldUmask);
    }

    free(parentPath);
    free(grandParentPath);
    
    return err;
}

static void PrintUsage(const char *argv0)
	// Print the program's usage.  Given that it supports no arguments whatsoever, 
	// this is pretty simple.
{
    const char *command;
    
    command = strrchr(argv0, '/');
    if (command == NULL) {
        command = argv0;
    } else {
        command += 1;
    }
    fprintf(stderr, "usage: %s\n", command);
}

int main (int argc, const char * argv[])
    // The primary entry point.
{
    int         err;
    int         junk;
    int         listenerFD;
    int         sockType;
    CFSocketRef listenerCF;
    Boolean     didBind;

#if !defined(_WIN32)
    sockType = AF_UNIX;
#else
    sockType = AF_INET;
#endif

    didBind    = false;
    listenerFD = -1;
    listenerCF = NULL;

	// Check the command line arguments (there shouldn't be any).
	
    err = 0;
    if (argc != 1) {
        PrintUsage(argv[0]);
        err = ECANCELED;
    }
	
#if !defined(_WIN32)
	// Ignore SIGPIPE because it's a deeply annoying concept.  If you don't ignore 
	// SIGPIPE when writing to a UNIX domain socket whose far side has been closed 
	// will trigger a SIGPIPE, whose default action is to terminate the program.
	
    if (err == 0) {
        fprintf(stderr, "CFLocalServer: Starting up (pid: %ld).\n", (long) getpid());
    
        err = MoreUNIXIgnoreSIGPIPE();
    }
    
	// Set up the signal handlers we are interested in.  In this case we redirect 
	// SIGINT and SIGUSR1 to our runloop.  If either of these signals occurs, we 
	// end up executing SignalRunLoopCallback.
	
    if (err == 0) {
        sigset_t    interestingSignals;
        
        (void) sigemptyset(&interestingSignals);
        (void) sigaddset(&interestingSignals, SIGINT);
        (void) sigaddset(&interestingSignals, SIGUSR1);
        
        err = InstallSignalToSocket(
            &interestingSignals,
            CFRunLoopGetCurrent(),
            kCFRunLoopDefaultMode,
            SignalRunLoopCallback,
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

	// Create the initial client set.
	
    if (err == 0) {
		err = ClientInitialise();
    }
    
	// Create our listening socket, bind it, and then wrap it in a CFSocket.
	
    if (err == 0) {
        listenerFD = socket(sockType, SOCK_STREAM, 0);
        err = MoreUNIXErrno(listenerFD);
    }
    if (err == 0) {
        err = SafeBindUnixDomainSocket(listenerFD, kServerSocketPath);
        didBind = (err == 0);
    }
    if (err == 0) {
        err = listen(listenerFD, 5);
        err = MoreUNIXErrno(err);
    }
    if (err == 0) {
        listenerCF = CFSocketCreateWithNative(
            NULL, 
            (CFSocketNativeHandle) listenerFD, 
            kCFSocketAcceptCallBack, 
            ListeningSocketAcceptCallback, 
            NULL);
        if (listenerCF == NULL) {
            err = EINVAL;
        }
    }
    
	// Schedule the listening socket on our runloop.
	
    if (err == 0) {
        CFRunLoopSourceRef  rls;
        
        rls = CFSocketCreateRunLoopSource(NULL, listenerCF, 0);
        if (rls == NULL) {
            err = EINVAL;
        } else {
            CFRunLoopAddSource( CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
            
            // We no longer need this source, so we just release it.
            
            CFRelease(rls);
        }
    }
    
	// Go go gadget server!
	
    if (err == 0) {
        CFRunLoopRun();
    }

    // Clean up.
    
	// Close down any connected clients.
	
	ClientTerminate();
    
	// Clean up our listenenng socket.
	
    if (listenerCF != NULL) {
        CFSocketInvalidate(listenerCF);
        
        CFRelease(listenerCF);
    }

    // CFSocket will close listenerFD when listenerCF is invalidated, so we 
    // don't close it if we did the invalidate above.

    if ( (listenerFD != -1) && (listenerCF == NULL) ) {
#if !defined(_WIN32)
        junk = close(listenerFD);
#else
        junk = closesocket(listenerFD);
#endif
        assert(junk == 0);
    }
    if (didBind) {
        (void) unlink(kServerSocketPath);
    }
    
    if (err == 0) {
        fprintf(stderr, "CFLocalServer: Shutting down.\n");
    } else if (err != ECANCELED) {
        fprintf(stderr, "CFLocalServer: Failed with error %d.\n", err);
    }
    
    return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
