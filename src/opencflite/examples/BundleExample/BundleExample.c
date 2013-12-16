/*
 * Copyright (c) 2011 Brent Fulgham <bfulgham@gmail.org>.  All rights reserved.
 *
 * Test application to confirm that bundle functions work properly
*/

#include <stdio.h>
#include <stdlib.h>

#include <CoreFoundation/CoreFoundation.h>

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


static void dumpBundleContents(CFBundleRef bundleRef) {
    CFURLRef bundleUrlRef = 0;
    CFDictionaryRef dictRef = 0;
    CFStringRef stringRef = 0;
    CFArrayRef arrayRef = 0;
    CFArrayRef arrayRef2 = 0;

    long longResult = 0;

    stringRef = CFBundleGetIdentifier(bundleRef);
    show(CFSTR("Bundle identifier: %@"), stringRef);

    longResult = CFBundleGetVersionNumber(bundleRef);
    show(CFSTR("Version: %d"), longResult);

    bundleUrlRef = CFBundleCopyBundleURL(bundleRef);
    assert (bundleRef);
    if (!bundleRef)
    {
        show(CFSTR("CFBundleCopyBundleURL() failed."));
        return;
    }
    show(CFSTR("Bundle location (as URL): %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    dictRef = CFBundleGetInfoDictionary(bundleRef);
    assert (bundleRef);
    if (!bundleRef)
    {
        show(CFSTR("CFBundleGetInfoDictionary() failed."));
        return;
    }
    show(CFSTR("Main CFBundle Dictionary: %@:"), dictRef);

    stringRef = CFBundleGetDevelopmentRegion(bundleRef);
    show(CFSTR("Development region: %@"), stringRef);

    bundleUrlRef = CFBundleCopySupportFilesDirectoryURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopySupportFilesDirectoryURL() failed."));
        return;
    }
    show(CFSTR("Support files directory URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    bundleUrlRef = CFBundleCopyResourcesDirectoryURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopyResourcesDirectoryURL() failed."));
        return;
    }
    show(CFSTR("Resources directory URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    bundleUrlRef = CFBundleCopyPrivateFrameworksURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopyPrivateFrameworksURL() failed."));
        return;
    }
    show(CFSTR("Private frameworks directory URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    bundleUrlRef = CFBundleCopySharedFrameworksURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopySharedFrameworksURL() failed."));
        return;
    }
    show(CFSTR("Shared frameworks directory URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    bundleUrlRef = CFBundleCopySharedSupportURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopySharedSupportURL() failed."));
        return;
    }
    show(CFSTR("Shared support directory URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    bundleUrlRef = CFBundleCopyBuiltInPlugInsURL(bundleRef);
    assert (bundleUrlRef);
    if (!bundleUrlRef)
    {
        show(CFSTR("CFBundleCopyBuiltInPlugInsURL() failed."));
        return;
    }
    show(CFSTR("Built-in Plugins URL: %@"), bundleUrlRef);
    CFRelease(bundleUrlRef);

    arrayRef = CFBundleCopyBundleLocalizations(bundleRef);
    if (!arrayRef)
    {
        show(CFSTR("CFBundleCopyBundleLocalizations(): No localizations present."));
    }
    else
    {
       show(CFSTR("Bundle Localizations: %@"), arrayRef);

       arrayRef2 = CFBundleCopyPreferredLocalizationsFromArray(arrayRef);
       if (!arrayRef2)
           show(CFSTR("CFBundleCopyPreferredLocalizationsFromArray(): No PREFERRED localizations."));
       else
           show(CFSTR("PREFERRED Bundle Localizations: %@"), arrayRef2);
       CFRelease(arrayRef2);
       CFRelease(arrayRef);
    }

    arrayRef = CFBundleCopyExecutableArchitectures(bundleRef);
    if (!arrayRef)
    {
        show(CFSTR("CFBundleCopyExecutableArchitectures(): No architectures defined."));
        return;
    }
    show(CFSTR("Executable architectures: %@"), arrayRef);
    CFRelease(arrayRef2);
}

#if defined(WIN32)
static char* safariBundlePath64 = "C:\\Program Files (x86)\\Safari\\Safari.resources";
static char* safariBundlePath = "C:\\Program Files\\Safari\\Safari.resources";
#endif

int main () {   
    CFBundleRef bundleRef = 0;
    CFBundleRef cfLiteBundleRef = 0;
    CFBundleRef safariBundleRef = 0;
    CFURLRef bundleUrlRef = 0;
    CFURLRef cfLiteURLRef = 0;
    CFURLRef safariPathRef = 0;

#if defined(__linux__)
    CFStringRef cfLiteBundlePath = CFSTR("/usr/local/share/CoreServices");
#else
    CFStringRef cfLiteBundlePath = CFSTR("./CFLite.resources");
#endif

    // 1. Use bundle routines on this application.
    printf("1. Test Application Bundle:\n");
    bundleRef = CFBundleGetMainBundle();
    assert (bundleRef);
    if (!bundleRef)
    {
        show(CFSTR("CFBundleGetMainBundle() failed."));
        return -1;
    }
    dumpBundleContents(bundleRef);

    printf("\n\n");

    // 2. Inspect our CFLite bundle.
    printf("2. CFLite Bundle:\n");
    bundleUrlRef = CFBundleCopyBundleURL(bundleRef);
    cfLiteURLRef = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorDefault, cfLiteBundlePath, kCFURLPOSIXPathStyle, false, bundleUrlRef);
    CFRelease(bundleUrlRef);
    show(CFSTR("CFLite path: %@"), cfLiteURLRef);
    cfLiteBundleRef = CFBundleCreate (kCFAllocatorDefault, cfLiteURLRef);
    CFRelease(cfLiteURLRef);
    assert (cfLiteBundleRef);
    if (!cfLiteBundleRef)
    {
        show(CFSTR("CFURLCreateWithFileSystemPathRelativeToBase() failed."));
        return -2;
    }
    dumpBundleContents(cfLiteBundleRef);
    CFRelease(cfLiteBundleRef);

#if defined(WIN32)
    printf("3. Safari Bundle:\n");
    // Try inspecting Safari.
    safariPathRef = CFURLCreateFromFileSystemRepresentation (kCFAllocatorDefault, safariBundlePath, (CFIndex)strlen (safariBundlePath), false); 
    if (!safariPathRef) {
        show(CFSTR("CFURLCreateFromFileSystemRepresentation() failed."));
        return -2;
    }
    show(CFSTR("Try Safari Path (32-bit OS): %@"), safariPathRef);

    safariBundleRef = CFBundleCreate (kCFAllocatorDefault, safariPathRef);
    CFRelease(safariPathRef);
    if (!safariBundleRef)
    {
        safariPathRef = CFURLCreateFromFileSystemRepresentation (kCFAllocatorDefault, safariBundlePath64, (CFIndex)strlen (safariBundlePath64), false); 
        show(CFSTR("Try Safari Path (64-bit OS): %@"), safariPathRef);
        if (!safariPathRef) {
            show(CFSTR("CFURLCreateFromFileSystemRepresentation() failed."));
            return -3;
        }
        safariBundleRef = CFBundleCreate (kCFAllocatorDefault, safariPathRef);
        CFRelease(safariPathRef);
    }

    if (!safariBundleRef) {
        show(CFSTR("Unable to find Safari binary."));
        return -4;
    }

    dumpBundleContents(safariBundleRef);
    CFRelease(safariBundleRef);
#endif

    return 0;
}


