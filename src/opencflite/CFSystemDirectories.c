/*
 * Copyright (c) 2008-2012 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
 *
 * This source code is a modified version of the CoreFoundation sources released by Apple Inc. under
 * the terms of the APSL version 2.0 (see below).
 *
 * For information about changes from the original Apple source release can be found by reviewing the
 * source control system for the project at https://sourceforge.net/svn/?group_id=246198.
 *
 * The original license information is as follows:
 * 
 * Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFSystemDirectories.c
	Copyright (c) 1997-2011, Apple Inc. All rights reserved.
	Responsibility: Kevin Perry
*/

/*
        This file defines CFCopySearchPathForDirectoriesInDomains().
        On MacOS 8, this function returns empty array.
        On Mach, it calls the System.framework enumeration functions.
        On Windows, it calls the enumeration functions defined here.
*/

#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CoreFoundation_Prefix.h>
#include "CFInternal.h"

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED

/* We use the System framework implementation on Mach.
*/
#include <libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <NSSystemDirectories.h>

CFSearchPathEnumerationState __CFStartSearchPathEnumeration(CFSearchPathDirectory dir, CFSearchPathDomainMask domainMask) {
    return (NSSearchPathEnumerationState)NSStartSearchPathEnumeration((NSSearchPathDirectory)dir, (NSSearchPathDomainMask)domainMask);
}

CFSearchPathEnumerationState __CFGetNextSearchPathEnumeration(CFSearchPathEnumerationState state, uint8_t *path, CFIndex pathSize) {
    CFSearchPathEnumerationState result;
    // NSGetNextSearchPathEnumeration requires a MAX_PATH size
    if (pathSize < PATH_MAX) {
        uint8_t tempPath[PATH_MAX];
        result = NSGetNextSearchPathEnumeration(state, (char *)tempPath);
        strlcpy((char *)path, (char *)tempPath, pathSize);
    } else {
        result = NSGetNextSearchPathEnumeration(state, (char *)path);
    }
    return result;
}

#elif DEPLOYMENT_TARGET_WINDOWS
CFSearchPathEnumerationState __CFStartSearchPathEnumeration(CFSearchPathDirectory dir, CFSearchPathDomainMask domainMask) {
    CFSearchPathEnumerationState result = 0;
    //return (NSSearchPathEnumerationState)NSStartSearchPathEnumeration((NSSearchPathDirectory)dir, (NSSearchPathDomainMask)domainMask);
    return result;
}

CFSearchPathEnumerationState __CFGetNextSearchPathEnumeration(CFSearchPathEnumerationState state, uint8_t *path, CFIndex pathSize) {
    CFSearchPathEnumerationState result = 0;
    /*
    // NSGetNextSearchPathEnumeration requires a MAX_PATH size
    if (pathSize < PATH_MAX) {
        uint8_t tempPath[PATH_MAX];
        result = NSGetNextSearchPathEnumeration(state, (char *)tempPath);
        strlcpy((char *)path, (char *)tempPath, pathSize);
    } else {
        result = NSGetNextSearchPathEnumeration(state, (char *)path);
    }
    */
    return result;
}

#elif DEPLOYMENT_TARGET_LINUX
CFSearchPathEnumerationState __CFStartSearchPathEnumeration(CFSearchPathDirectory dir, CFSearchPathDomainMask domainMask) {
    return 0;
}

CFSearchPathEnumerationState __CFGetNextSearchPathEnumeration(CFSearchPathEnumerationState state, uint8_t *path, CFIndex pathSize) {
    return 0;
}
#endif


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS_SYNC || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD

CFArrayRef CFCopySearchPathForDirectoriesInDomains(CFSearchPathDirectory directory, CFSearchPathDomainMask domainMask, Boolean expandTilde) {
    CFMutableArrayRef array;
    CFSearchPathEnumerationState state;
    CFIndex homeLen = -1;
    char cPath[CFMaxPathSize], home[CFMaxPathSize];

    array = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    state = __CFStartSearchPathEnumeration(directory, domainMask);
    while ((state = __CFGetNextSearchPathEnumeration(state, (uint8_t *)cPath, sizeof(cPath)))) {
	CFURLRef url = NULL;
	if (expandTilde && (cPath[0] == '~')) {
	    if (homeLen < 0) {
		CFURLRef homeURL = CFCopyHomeDirectoryURLForUser(NULL);
		if (homeURL) {
		    CFURLGetFileSystemRepresentation(homeURL, true, (uint8_t *)home, CFMaxPathSize);
		    homeLen = (CFIndex)strlen(home);
		    CFRelease(homeURL);
		}
	    }
            if (homeLen + strlen(cPath) < CFMaxPathSize) {
		home[homeLen] = '\0';
		strlcat(home, &cPath[1], sizeof(home));
		url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)home, strlen(home), true);
	    }
	} else {
	    url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)cPath, strlen(cPath), true);
	}
	if (url) {
	    CFArrayAppendValue(array, url);
	    CFRelease(url);
	}
    }
    return array;
}

#endif

#undef numDirs
#undef numApplicationDirs
#undef numLibraryDirs
#undef numDomains
#undef invalidDomains
#undef invalidDomains

