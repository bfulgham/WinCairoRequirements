/*
 *    Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>
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
    File:       Protocol.h

    Contains:   Definition of the communication protocol between client and server.

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

$Log: Protocol.h,v $
Revision 1.1  2005/05/17 12:19:23         
First checked in.


*/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

/////////////////////////////////////////////////////////////////

// System interfaces

#include <CoreFoundation/CoreFoundation.h>


/////////////////////////////////////////////////////////////////

// Packet types

enum {
    kPacketTypeGoodbye = 'GDBY',            // sent by client to server before signing off
    kPacketTypeNOP     = 'NOOP',            // no operation, test for client/server RPC
    kPacketTypeReply   = 'RPLY',            // all RPC replies are of this type
    kPacketTypeWhisper = 'WSPR',            // client/server RPC to print message on server
    kPacketTypeShout   = 'SHOU',            // sent by client to server, which echoes it to all listening clients
    kPacketTypeListen  = 'LSTN',            // client/server RPC to register for shouts
    kPacketTypeQuit    = 'QUIT'             // client/server RPC to tell server to quit
};

// Well known packet IDs (for the fID field of the packet header).
// kPacketIDFirst is just a suggestion.  The server doesn't require 
// that the client use sequential IDs starting from 1; the IDs are 
// for the client to choose.  However, the use of kPacketIDNone for 
// some packets (those that don't have a reply) is a hard requirement 
// in the server.  

enum {
    kPacketIDNone  = 0,
    kPacketIDFirst = 1
};

// kPacketMagic is first four bytes of every packet.  If the packet stream 
// gets out of sync, this will quickly detect the problem.

enum {
    kPacketMagic = 'LSPM'               // for Local Server Packet Magic
};

// kPacketMaximumSize is an arbitrary limit, chosen so that we can detect 
// if the packet streams get out of sync or if a client goes mad.

enum {  
    kPacketMaximumSize = 100 * 1024     // just basic sanity checks
};

// IMPORTANT:
// The following structures define the packets sent between the client and 
// the server.  For a network protocol you'd need to worry about byte ordering, 
// but this isn't a network protocol (it always stays on the same machine) so 
// I don't have to worry.  However, I do need to worry about having 
// size invariant types (so that 32- and 64-bit clients and servers are 
// all compatible) and structure alignment (so that code compiled by different 
// compilers is compatible).  Size invariant types is easy, and represented 
// by the structures below.  Alignment is trickier, and I'm mostly just 
// glossing over the issue right now.

// PacketHeader is the header at the front of every packet.

typedef uint32_t FourCharCode;
typedef FourCharCode OSType;

struct PacketHeader {
    OSType          fMagic;             // must be kPacketMagic
    OSType          fType;              // kPacketTypeGoodbye etc 
    int32_t         fID;                // kPacketIDNone or some other value
    uint32_t        fSize;              // includes size of header itself
};
typedef struct PacketHeader PacketHeader;

struct PacketGoodbye {                  // reply: n/a
    PacketHeader    fHeader;            // fType is kPacketTypeGoodbye, fID must be kPacketIDNone
    char            fMessage[32];       // Just for fun.
};
typedef struct PacketGoodbye PacketGoodbye;

struct PacketNOP {                      // reply: PacketReply
    PacketHeader    fHeader;            // fType is kPacketTypeNOP, fID echoed
};
typedef struct PacketNOP PacketNOP;

struct PacketReply {                    // reply: n/a
    PacketHeader    fHeader;            // fType is kPacketTypeReply, fID is ID of request
    int32_t         fErr;               // result of operation, errno-style
};
typedef struct PacketReply PacketReply;

struct PacketWhisper {                  // reply: PacketReply
    PacketHeader    fHeader;            // fType is kPacketTypeWhisper, fID echoed
    char            fMessage[32];       // message to print
};
typedef struct PacketWhisper PacketWhisper;

struct PacketShout {                    // reply: none
    PacketHeader    fHeader;            // fType is kPacketTypeShout, fID must be kPacketIDNone
    char            fMessage[32];       // message for each of the clients
};
typedef struct PacketShout PacketShout;

// Shouts are echoed to anyone who listens, including sender.

struct PacketListen {                   // reply: PacketReply
    PacketHeader    fHeader;            // fType is kPacketTypeListen, fID echoed
};
typedef struct PacketListen PacketListen;

struct PacketQuit {                     // reply: PacketReply
    PacketHeader    fHeader;            // fType is kPacketTypeQuit, fID echoed
};
typedef struct PacketQuit PacketQuit;

// IMPORTANT:
// The location of the kServerAddress socket file is intimately tied to the 
// security of the server.  The file should be placed in a directory that's 
// read access to everyone who needs access to the service, but only write 
// accessible to the UID that's running the server.  Failure to follow this  
// guideline may make your server subject to security exploits.
//
// To prevent this sort of silliness I require that the socket be created 
// in a directory owned by the server's user ("com.apple.dts.CFLocalServer") 
// within a directory that's sticky ("/var/tmp", not "/tmp" because that's 
// periodically cleaned up). See SafeBindUnixDomainSocket (in "Server.c") for 
// the details about how I achieve this.

#define kServerSocketPath "/var/tmp/com.apple.dts.CFLocalServer/Socket"

#endif
