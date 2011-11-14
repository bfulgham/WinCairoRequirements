/*
 Simple CFAllocator example program, demonstrating creation and use of custom CFAllocators.
 Author: Ali Ozer
 7/7/99

 Copyright (c) 1999-2004, Apple Computer, Inc., all rights reserved.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>


// This function will print the provided arguments (printf style varargs) out to the console.
// Note that the CFString formatting function accepts "%@" as a way to display CF types.
// For types other than CFString and CFNumber, the result of %@ is mostly for debugging
// and can differ between releases and different platforms.

void show(CFStringRef formatString, ...) {
    CFStringRef resultString;
    CFDataRef data;
    va_list argList;

    va_start(argList, formatString);
    resultString = CFStringCreateWithFormatAndArguments(NULL, NULL, formatString, argList);
    va_end(argList);

    data = CFStringCreateExternalRepresentation(NULL, resultString, CFStringGetSystemEncoding(), '?');

    if (data != NULL) {
    	printf ("%.*s\n\n", (int)CFDataGetLength(data), CFDataGetBytePtr(data));
    	CFRelease(data);
    }
       
    CFRelease(resultString);
}


// "Counting" Allocator
// A simple custom allocator that will count memory allocations and place guards to check memory corruption.

void verifyMemory(int *ptr) {
    int size = ptr[0];
    if (ptr[1] != 0x42424242) abort();
    if (ptr[2 + size / sizeof(int)] != 0x42424242) abort();
    if (ptr[2 + size / sizeof(int) + 1] != 0x42424242) abort();
    if (ptr[2 + size / sizeof(int) + 2] != 0x42424242) abort();
    if (ptr[2 + size / sizeof(int) + 3] != 0x42424242) abort();
}

static void *countingRealloc(void *oPtr, CFIndex size, CFOptionFlags hint, void *info) {
    int *ptr = (int*)oPtr;

    // Round it up to int alignment.
    size = ((size + sizeof(int) - 1) / sizeof(int)) * sizeof(int);

    if (ptr) {
        ptr = ptr - 2;	// Get back to the the originally allocated piece
        verifyMemory(ptr);
        ptr = (int*)realloc(ptr, size + sizeof(int) * 6);
    } else {
        ptr = (int*)malloc(size + sizeof(int) * 6);
    }
    ptr[0] = size;	// Doesn't include the guards
    ptr[1] = 0x42424242;
    ptr[2 + size / sizeof(int)] = 0x42424242;
    ptr[2 + size / sizeof(int) + 1] = 0x42424242;
    ptr[2 + size / sizeof(int) + 2] = 0x42424242;
    ptr[2 + size / sizeof(int) + 3] = 0x42424242;

    return (void *)(ptr + 2);
}

static void *countingAlloc(CFIndex size, CFOptionFlags hint, void *info) {
    (*(int *)info)++;		// Increment count
    return countingRealloc(NULL, size, hint, info);
}

static void countingDealloc(void *ptr, void *info) {
    (*(int *)info)--;		// Decrement count
    ptr = (int *)ptr - 2;	// Restore pointer to the actual malloc block
    verifyMemory((int*)ptr);
    free(ptr);
}

// Returns info about the number of outstanding allocations

static CFIndex NumOutstandingAllocations(CFAllocatorRef alloc) {
    CFAllocatorContext context;
    context.version = 0;
    CFAllocatorGetContext(alloc, &context);
    return (*(int *)(context.info));
}

// Creates a "counting allocator" --- keeps track of number of
// outstanding allocations and also uses guards to catch out of bound writes

static CFAllocatorRef CreateCountingAllocator(CFAllocatorRef alloc) {
    CFAllocatorContext context = {0, NULL, NULL, (CFAllocatorReleaseCallBack)free, NULL, countingAlloc, countingRealloc, countingDealloc, NULL};
    context.info = malloc(sizeof(int));
    // The info field points to an int which keeps number of allocations/deallocations
    // The "free" in slot 3 of the context assures the info field is properly freed when the allocator is
    *(int *)(context.info) = 0;
    // Create and return the allocator
    return CFAllocatorCreate(alloc, &context);
}



// "Reduced Malloc" Allocator
// A simple custom allocator that tries to avoid allocations.
// This allocator will allocate one chunk of memory containing n blocks of a certain size each,
// and manage these as the memory pool. If larger requests or more requests come in, this
// allocator will fall back on allocating actual memory.
// Note: This allocator isn't that great; it's provided as sample code only.

// This struct holds the actual blocks for the memory plus the necessary info
// (The allocated size of this struct will depend on the desired number of blocks)

typedef struct {
    CFAllocatorRef allocator;
    CFIndex size;
    CFIndex numBlocks;
    CFIndex firstAvailable;
    CFIndex numAdditionalAllocations;	// Stats for finding out how much addt'l memory was allocated
    void *mem[0];
} rmInfo;

// This function is used to free the above structure when the allocator is deallocated

void rmInfoFree(const void *actualInfo) {
    rmInfo *info = (rmInfo *)actualInfo;
    CFAllocatorRef allocator = info->allocator;
    CFAllocatorDeallocate(allocator, info);
    CFRelease(allocator);
}

#define rmNextBlockNum(info, blockNum) *(int *)(((unsigned char *)&(info->mem)) + (blockNum * info->size))
// #define rmBlock(info, blockNum) ((&(info->mem)) + (blockNum * info->size))
#define rmBlock(info, blockNum) (((unsigned char *)&(info->mem)) + (blockNum * info->size))
#define rmBlockNumber(info, ptr) (((unsigned char *)ptr - (unsigned char *)rmBlock(info, 0)) / info->size)
#define isRMBlock(info, ptr) (ptr >= (void *)rmBlock(info, 0) && ptr <= (void *)rmBlock(info, info->numBlocks))

static void rmMarkBlockAsAvailable(rmInfo *info, CFIndex blockNum) {
    // Put this block at the head of the available list
    rmNextBlockNum(info, blockNum) = info->firstAvailable;
    info->firstAvailable = blockNum;
}

static void rmMarkFirstAvailableAsUnavailable(rmInfo *info) {
    // This will only happen for the first block, so take it off the free list and return
    info->firstAvailable = rmNextBlockNum(info, info->firstAvailable);
}

// The function pointers for the allocator context

static void *rmRealloc(void *oPtr, CFIndex size, CFOptionFlags hint, void *actualInfo) {
    rmInfo *info = (rmInfo *)actualInfo;
    if (isRMBlock(info, oPtr)) {
	void *ptr;
        if (size <= info->size) return oPtr;	// Easy case...
	ptr = CFAllocatorAllocate(info->allocator, size, hint);
	memmove(ptr, oPtr, info->size);
	rmMarkBlockAsAvailable(info, rmBlockNumber(info, oPtr));
	info->numAdditionalAllocations++;
	return ptr;
    } else {
	return CFAllocatorReallocate(info->allocator, oPtr, size, hint);
    }
}

static void *rmAlloc(CFIndex size, CFOptionFlags hint, void *actualInfo) {
    rmInfo *info = (rmInfo *)actualInfo;
    if ((info->firstAvailable == -1) || (size > info->size)) {
	// No more available; allocate addt'l memory
	info->numAdditionalAllocations++;
        return CFAllocatorAllocate(info->allocator, size, hint);
    } else {
	void *ptr = rmBlock(info, info->firstAvailable);
	rmMarkFirstAvailableAsUnavailable(info);
        return ptr;
    }
}

static void rmDealloc(void *ptr, void *actualInfo) {
    rmInfo *info = (rmInfo *)actualInfo;
    if (isRMBlock(info, ptr)) {
	// It is one of ours, so mark it as available
	rmMarkBlockAsAvailable(info, rmBlockNumber(info, ptr));
    } else {
	// It is not, so directly deallocate
	info->numAdditionalAllocations--;
        CFAllocatorDeallocate(info->allocator, ptr);
    }
}

static CFIndex NumAdditionalAllocations(CFAllocatorRef alloc) {
    CFAllocatorContext context;
    context.version = 0;
    CFAllocatorGetContext(alloc, &context);
    return ((rmInfo *)(context.info))->numAdditionalAllocations;
}

// Creates a "reduced malloc allocator".

static CFAllocatorRef CreateReducedMallocAllocator(CFAllocatorRef alloc, CFIndex size, CFIndex numBlocks) {
    CFIndex cnt;
    rmInfo *info;
    CFAllocatorContext context = {0, NULL, NULL, rmInfoFree, NULL, rmAlloc, rmRealloc, rmDealloc, NULL};

    // Create and initialize the info block and the memory
    info = (rmInfo*)CFAllocatorAllocate(alloc, (sizeof(rmInfo) + size * numBlocks), 0);
    info->size = size;
    info->numBlocks = numBlocks;
    info->firstAvailable = -1;
    info->numAdditionalAllocations = 0;
    for (cnt = 0; cnt < numBlocks; cnt++) rmMarkBlockAsAvailable(info, cnt);
    // Retain the allocator used to allocate the memory
    info->allocator = (CFAllocatorRef)((alloc) ? CFRetain(alloc) : CFRetain(CFAllocatorGetDefault()));
    // And hang on to the info block
    context.info = info;
    // Finally, create and return the allocator
    return CFAllocatorCreate(alloc, &context);
}



// Now some example code which uses these allocators

void countingAllocatorExample(void) {

    CFStringRef str1, str2, str3;
    CFMutableStringRef mStr;

    CFAllocatorRef countingAllocator;

    show(CFSTR("Counting Allocator Test"));

    countingAllocator = CreateCountingAllocator(NULL);

    show(CFSTR("At start, number of allocations: %d"), NumOutstandingAllocations(countingAllocator));

    str1 = CFStringCreateWithCString(countingAllocator, "Hello World", kCFStringEncodingASCII);
#if defined(__APPLE__)
    str2 = CFStringCreateWithPascalString(countingAllocator, "\pHello World", kCFStringEncodingASCII);
#else
    str2 = CFStringCreateWithCString(countingAllocator, "Hello World", kCFStringEncodingASCII);
#endif
    mStr = CFStringCreateMutableCopy(countingAllocator, 0, str1);
    str3 = CFStringCreateCopy(countingAllocator, mStr);

    show(CFSTR("After creation, number of allocations: %d"), NumOutstandingAllocations(countingAllocator));

    CFStringAppend(mStr, str1);
    CFStringAppend(mStr, str1);
    CFStringAppend(mStr, str1);

    show(CFSTR("After mutations, number of allocations: %d"), NumOutstandingAllocations(countingAllocator));

    CFRelease(str1);
    CFRelease(str2);
    CFRelease(mStr);
    CFRelease(str3);

    show(CFSTR("After releasing, number of allocations: %d"), NumOutstandingAllocations(countingAllocator));

    CFRelease(countingAllocator);
}


void reducedMallocAllocatorExample(void) {

    CFStringRef str1, str2, str3;
    CFMutableStringRef mStr;

    CFAllocatorRef reducedMallocAllocator;

    show(CFSTR("Reduced Malloc Allocator Test"));

    // Create a reduced memory allocator; have it start with 3 blocks of 40 bytes each.

    reducedMallocAllocator = CreateReducedMallocAllocator(NULL, 40, 3);

    show(CFSTR("At start, number of addt'l allocations: %d"), NumAdditionalAllocations(reducedMallocAllocator));

    str1 = CFStringCreateWithCString(reducedMallocAllocator, "Hello World", kCFStringEncodingASCII);
#if defined(__APPLE__)
    str2 = CFStringCreateWithPascalString(reducedMallocAllocator, "\pHello World", kCFStringEncodingASCII);
#else
    str2 = CFStringCreateWithCString(reducedMallocAllocator, "Hello World", kCFStringEncodingASCII);
#endif
    mStr = CFStringCreateMutableCopy(reducedMallocAllocator, 0, str1);
    str3 = CFStringCreateCopy(reducedMallocAllocator, mStr);

    show(CFSTR("After creation, number of addt'l allocations: %d"), NumAdditionalAllocations(reducedMallocAllocator));

    CFStringAppend(mStr, str1);
    CFStringAppend(mStr, str1);
    CFStringAppend(mStr, str1);

    show(CFSTR("After mutations, number of addt'l allocations: %d"), NumAdditionalAllocations(reducedMallocAllocator));

    CFRelease(str1);
    CFRelease(str2);
    CFRelease(mStr);
    CFRelease(str3);

    show(CFSTR("After releasing, number of addt'l allocations: %d"), NumAdditionalAllocations(reducedMallocAllocator));

    CFRelease(reducedMallocAllocator);
}


int main (int argc, const char *argv[]) {
    countingAllocatorExample();
    reducedMallocAllocatorExample();

    return 0;
}


