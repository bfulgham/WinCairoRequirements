/*
 *    Copyright (c) 2009 Grant Erickson <gerickson@nuovations.com>
 *    Copyright (c) 2009 Brent Fulgham <bfulgham@gmail.com>
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
    File:       Common.c

    Contains:   Common code between client and server.

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

$Log: Common.c,v $
Revision 1.2  2005/06/30 20:22:25         
Corrected a bug that would causing the packet's fID field to not be initialised in the RPC case.

Revision 1.1  2005/05/17 12:19:17         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#if defined(WIN32)
#include <stdio.h>
#define snprintf _snprintf

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <fcntl.h>
#include <process.h>

#define SOCK_MAXADDRLEN 255
#define MAXPATHLEN MAX_PATH

#define NI_MAXHOST 1025
#define NI_MAXSERV 32

#define NI_NUMERICHOST 0x02
#define NI_NUMERICSERV 0x08
#else
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/un.h>
#include <netdb.h>
#endif

// Our prototypes

#include "Common.h"

/////////////////////////////////////////////////////////////////

extern int MoreUNIXErrno(int result)
    // See comment in header.
{
    int err;
    
    err = 0;
    if (result < 0) {
#if !defined(_WIN32)
        err = errno;
#else
        err = WSAGetLastError ();
#endif
        assert(err != 0);
    }
    return err;
}

#if !defined(_WIN32)
extern int MoreUNIXIgnoreSIGPIPE(void)
    // See comment in header.
{
	int err;
	struct sigaction signalState;
	
	err = sigaction(SIGPIPE, NULL, &signalState);
	err = MoreUNIXErrno(err);
	if (err == 0) {
		signalState.sa_handler = SIG_IGN;
		
		err = sigaction(SIGPIPE, &signalState, NULL);
		err = MoreUNIXErrno(err);
	}
	
	return err;
}
#endif

extern int MoreUNIXRead( int fd,       void *buf, size_t bufSize, size_t *bytesRead   )
    // See comment in header.
{
	int 	err;
	char *	cursor;
	size_t	bytesLeft;
	ssize_t bytesThisTime;

	assert(fd >= 0);
	assert(buf != NULL);
	
	err = 0;
	bytesLeft = bufSize;
	cursor = (char *) buf;
	while ( (err == 0) && (bytesLeft != 0) ) {
		bytesThisTime = read(fd, cursor, bytesLeft);
		if (bytesThisTime > 0) {
			cursor    += bytesThisTime;
			bytesLeft -= bytesThisTime;
		} else if (bytesThisTime == 0) {
			err = EPIPE;
		} else {
			assert(bytesThisTime == -1);
			
			err = errno;
			assert(err != 0);
			if (err == EINTR) {
				err = 0;		// let's loop again
			}
		}
	}
	if (bytesRead != NULL) {
		*bytesRead = bufSize - bytesLeft;
	}
	
	return err;
}

extern int MoreUNIXWrite(int fd, const void *buf, size_t bufSize, size_t *bytesWritten)
    // See comment in header.
{
	int 	err;
	char *	cursor;
	size_t	bytesLeft;
	ssize_t bytesThisTime;
	
	assert(fd >= 0);
	assert(buf != NULL);

	err = 0;
	bytesLeft = bufSize;
	cursor = (char *) buf;
	while ( (err == 0) && (bytesLeft != 0) ) {
		bytesThisTime = write(fd, cursor, bytesLeft);
		if (bytesThisTime > 0) {
			cursor    += bytesThisTime;
			bytesLeft -= bytesThisTime;
		} else if (bytesThisTime == 0) {
			assert(false);
			err = EPIPE;
		} else {
			assert(bytesThisTime == -1);
			
			err = errno;
			assert(err != 0);
			if (err == EINTR) {
				err = 0;		// let's loop again
			}
		}
	}
	if (bytesWritten != NULL) {
		*bytesWritten = bufSize - bytesLeft;
	}
	
	return err;
}

extern int MoreUNIXSetNonBlocking(int fd)
{
    int err;
    int flags;
 
#if defined(_WIN32)
    unsigned long nonblockopt = 1;
    flags = ioctlsocket (fd, FIONREAD, 0);
    err = ioctlsocket (fd, FIONBIO, &nonblockopt);
#else
    // According to the man page, F_GETFL can't error!
    
    flags = fcntl(fd, F_GETFL, NULL);
    
    err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
    err = MoreUNIXErrno(err);
    
    return err;
}

#if !defined(_WIN32)
static int					gSignalSinkFD   = -1;
static CFSocketRef			gSignalSourceCF = NULL;
static SignalSocketCallback	gClientCallback = NULL;
static void *				gClientCallbackRefCon = NULL;

static void SignalToSocketHandler(int sig, siginfo_t *sigInfo, void *uap)
    // A signal handler that catches the signal and forwards it 
    // to the runloop via gSignalSinkFD.  This code is careful to 
    // only use signal safe routines (except for the asserts, 
    // of course, but they're compiled out on production builds).
{
	ssize_t		junk;
	
	(void)uap;

	assert(gSignalSinkFD != -1);
	
	assert(sig == sigInfo->si_signo);

	junk = write(gSignalSinkFD, sigInfo, sizeof(*sigInfo));
	
	// There's not much I can do if this fails.  Missing a signal 
	// isn't such a big deal, but writing only a partial siginfo_t 
	// to the socket would be bad.
	
	assert(junk == sizeof(*sigInfo));
}

static void SignalCFSocketCallback(
	CFSocketRef				s, 
	CFSocketCallBackType	type, 
	CFDataRef				address, 
	const void *			data, 
	void *					info
)
    // Call in the context of the runloop when data arrives on the 
    // UNIX domain socket shared with the signal handler.  This 
    // reads the information about the signal and passes to the client's 
    // callback.
{
	int			err;
	siginfo_t	sigInfo;

	(void)info;

	assert(gSignalSourceCF != NULL);
	assert(gClientCallback != NULL);
	
	assert(s == gSignalSourceCF);
	assert(type == kCFSocketReadCallBack);
	assert(address == NULL);
	assert(data == NULL);

	err = MoreUNIXRead( CFSocketGetNative(gSignalSourceCF), &sigInfo, sizeof(sigInfo), NULL);
	if (err == 0) {
		gClientCallback(&sigInfo, gClientCallbackRefCon);
	}
	assert(err == 0);
}

extern int InstallSignalToSocket(
	const sigset_t *		sigSet, 
	CFRunLoopRef			runLoop,
	CFStringRef				runLoopMode,
	SignalSocketCallback	callback, 
	void *					refCon
)
    // See comment in header.
{
	int		err;
	int		junk;
	int		sockets[2];
	int		signalSourceFD;
	
	assert(sigSet != NULL);
	assert(runLoop != NULL);
	assert(runLoopMode != NULL);
	assert(callback != NULL);
	
	assert(gSignalSinkFD   == -1);              // don't call me twice
	assert(gSignalSourceCF == NULL);
	assert(gClientCallback == NULL);
	
	signalSourceFD = -1;
	
    // Create a UNIX domain socket pair and assign them to the 
    // sink (where the signal handler writes the information) and 
    // source variables (where the runloop callback reads it).  
    
	err = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
	err = MoreUNIXErrno(err);
	if (err == 0) {
		gSignalSinkFD  = sockets[0];
		signalSourceFD = sockets[1];
	}
    
    // We set the signal sink socket to non-blocking because, if the 
    // socket fills up, there's a possibility we might deadlock with 
    // ourselves (the signal handler blocks trying to write data to 
    // a full socket, but the runloop thread can't read data from the 
    // socket because it has been interrupted by the signal handler).
    
    if (err == 0) {
        err = MoreUNIXSetNonBlocking(gSignalSinkFD);
    }

    // Wrap the destination socket in a CFSocket, and create a 
    // runloop source for it.  The associated callback (SignalCFSocketCallback) 
    // receives information about the signal from the signal handler 
    // and passes it along to the client's callback, but it's now in the context 
    // of the runloop.
    
	if (err == 0) {
		gSignalSourceCF = CFSocketCreateWithNative(
			NULL,
			signalSourceFD, 
			kCFSocketCloseOnInvalidate | kCFSocketReadCallBack, 
			SignalCFSocketCallback, 
			NULL
		);
        if (gSignalSourceCF == NULL) {
            err = EINVAL;
        }
	}
	if (err == 0) {
		CFRunLoopSourceRef	rls;
		int					thisSigNum;

		gClientCallback = callback;
        gClientCallbackRefCon = refCon;
		
		rls = CFSocketCreateRunLoopSource(NULL, gSignalSourceCF, 0);
        if (rls == NULL) {
            err = EINVAL;
        }

		if (err == 0) {
			CFRunLoopAddSource(runLoop, rls, runLoopMode);

            // For each signal in the set, register our signal handler 
            // (SignalToSocketHandler).  Specificy SA_SIGINFO so that 
            // the handler gets lots of yummy signal information.
            
			for (thisSigNum = 1; thisSigNum < NSIG; thisSigNum++) {
				if ( sigismember(sigSet, thisSigNum) ) {
					struct sigaction newSignalAction;
					
					newSignalAction.sa_sigaction = SignalToSocketHandler;
					newSignalAction.sa_flags = SA_SIGINFO;
					sigemptyset(&newSignalAction.sa_mask);

					junk = sigaction(thisSigNum, &newSignalAction, NULL);
					assert(junk == 0);
					
					// Error recovery here would be hard.  We'd have to undo 
					// any previous signal handlers that were installed 
					// (requiring us to get the previous value and remembering 
					// it) and then it would also require us to remove the 
					// run loop source.  All-in-all, not worth the effort 
					// given the very small chance of an error from sigaction.
				}
			}
		}
		
        // We don't need the runloop source from here on, so release our 
        // reference to it.  It still exists because the runloop knows about it.
        
        if (rls != NULL) {
            CFRelease(rls);
        }
	}
	
	// Clean up.

	if (err != 0) {
		gClientCallback = NULL;
		
		if (gSignalSourceCF != NULL) {
			CFSocketInvalidate(gSignalSourceCF);
			CFRelease(gSignalSourceCF);
			gSignalSourceCF = NULL;
		}
		
		if (signalSourceFD != -1) {
			junk = close(signalSourceFD);
			assert(junk == 0);
		}
        
        if (gSignalSinkFD != -1) {
            junk = close(gSignalSinkFD);
            assert(junk == 0);
            
            gSignalSinkFD = -1;
        }
	}
	
	return err;
}
#endif

static char * SockAddrToString(int fd, Boolean peer)
    // Gets either the socket name or the peer name from the socket 
    // (depending on the peer parameter) and converts it to a human 
    // readable string.  The caller is responsible for freeing the 
    // memory.
{
    int			err;
    char *		result;
    size_t		resultLen;
	struct sockaddr addr;
    socklen_t	addrLen;
    
    addrLen = sizeof(addr);
    
    // Socket name, or peer name?
    
    if (peer) {
        err = getpeername(fd, &addr, &addrLen);
    } else {
        err = getsockname(fd, &addr, &addrLen);
    }
    
    // Convert the result to a string.
    
    if ( (err == -1) || (addrLen < offsetof(struct sockaddr, sa_data))) {
        result = strdup("?");
    } else {
        char hostStr[NI_MAXHOST];
        char servStr[NI_MAXSERV];
        
        assert(addrLen >= offsetof(struct sockaddr, sa_data));

        err = getnameinfo(
            &addr, 
            addrLen, 
            hostStr, 
            sizeof(hostStr), 
            servStr, 
            sizeof(servStr), 
            NI_NUMERICHOST | NI_NUMERICSERV
        );
        if (err == 0) {
            // Cool.  getnameinfo did all the heavy lifting, so we just return the results.
            
            resultLen = strlen(hostStr) + 1 + strlen(servStr) + 1;
            result = malloc(resultLen);
            if (result != NULL) {
                snprintf(result, resultLen, "%s %s", hostStr, servStr);
            }
        } else {
            // Drat.  getnameinfo isn't helping out with this address, so we have to do it 
            // all by hand.
            
            switch (addr.sa_family) {
                case AF_UNIX:
                    {
                        struct sockaddr_un * unAddr;
                        
                        unAddr = (struct sockaddr_un *)&addr;
                        result = strdup( unAddr->sun_path );
                    }
                    break;
                default:
                    assert(false);
                    result = strdup("unrecognised address");
                    break;
            };
        }
    }
                
    return result;
}

static int
fdtopath(int fd, char *buffer, size_t size)
{
#if HAVE_F_GETPATH            
	return fcntl(fd, F_GETPATH, buffer);
#else
	char procbuf[MAXPATHLEN];
	int n;

	n = snprintf(procbuf, sizeof(procbuf), "/proc/self/fd/%d", fd);

	if (n > 0) {
		n = readlink(procbuf, buffer, size);
		if (n > 0 && n <= size) {
			buffer[n] = '\0';
		}
		return n;
	} else {
		return -1;
	}
#endif
}

extern void DebugPrintDescriptorTable(void)
    // See comment in header.
{
    int			err;
    int			descCount;
    int			descIndex;
    char		pathBuf[MAXPATHLEN];
    int			sockType;
    socklen_t	sockTypeLen;
    static const char * kSockTypeToStr[] = {
        "unknown    ",
        "SOCK_STREAM",
        "SOCK_DGRAM ",
        "SOCK_RAW   ",
        "SOCK_RDM   ",
        "SOCK_SEQPACKET"            // not going to see this anyway, so don't need to pad everything else to this long length
    };

    fprintf(stderr, "Descriptors:\n");

#if !defined(_WIN32)
    descCount = getdtablesize();
    for (descIndex = 0; descIndex < descCount; descIndex++) {
        if ( fcntl(descIndex, F_GETFD, NULL) != -1 ) {

            // Descriptor is active, let's try to find out what it is.

            // See if we can get a file path from it.

			err = fdtopath(descIndex, pathBuf, sizeof(pathBuf));

            if (err != -1) {
            
                // If it's a file, print its path.
                
                fprintf(stderr, "  %2d file    '%s'\n", descIndex, pathBuf);
            } else {
            
                // See if it's a socket.
                
                sockTypeLen = sizeof(sockType);
                err = getsockopt(descIndex, SOL_SOCKET, SO_TYPE, &sockType, &sockTypeLen);
                if (err != -1) {
                    char *  localStr;
                    char *  peerStr;
                    const char *  sockTypeStr;

                    // If it's a socket, print the local and remote address.
                    
                    localStr = NULL;
                    peerStr  = NULL;
                    
                    localStr = SockAddrToString(descIndex, false);
                    peerStr  = SockAddrToString(descIndex, true);

                    if ( (sockType < 0) || (sockType > (sizeof(kSockTypeToStr) / sizeof(kSockTypeToStr[0]))) ) {
                        sockTypeStr = kSockTypeToStr[0];
                    } else {
                        sockTypeStr = kSockTypeToStr[sockType];
                    }
                    if (sockTypeStr == kSockTypeToStr[0]) {
                        fprintf(stderr, "  %2d socket  %s (%d) %s -> %s\n", descIndex, sockTypeStr, sockType, localStr, peerStr);
                    } else {
                        fprintf(stderr, "  %2d socket  %s %s -> %s\n", descIndex, sockTypeStr, localStr, peerStr);
                    }
                    
                    free(localStr);
                    free(peerStr);
                } else {

                    // No idea.

                    fprintf(stderr, "  %2d unknown\n", descIndex);
                }
            }
        }
    }
#endif
}

extern void InitPacketHeader(PacketHeader *packet, OSType packetType, size_t packetSize, Boolean rpc)
    // See comment in header.
{
    static int sNextID = kPacketIDFirst;
    
    assert(packet != NULL);
    assert(packetSize >= sizeof(PacketHeader));
    
    packet->fMagic = kPacketMagic;
    packet->fType  = packetType;
    if (rpc) {
        // Increment to the next ID, skipping 0.
        
		packet->fID = sNextID;
        sNextID += 1;
        if (sNextID == kPacketIDNone) {
            sNextID = kPacketIDFirst;
        }
    } else {
        packet->fID = kPacketIDNone;
    }
    packet->fSize = packetSize;
}
