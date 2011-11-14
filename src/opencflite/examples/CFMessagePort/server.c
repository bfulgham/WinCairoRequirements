#include <CoreFoundation/CoreFoundation.h>

CFDataRef myCallBack(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info) {
     char *message = "Thanks for calling!";
     CFDataRef returnData = CFDataCreate(NULL, (const UInt8 *)message, strlen(message)+1);
     printf("here is our received data: %s\n", CFDataGetBytePtr(data));
     return returnData;  // as stated in header, both data and returnData will be released for us after callback returns
}

main() {
     CFMessagePortRef local = CFMessagePortCreateLocal(NULL, CFSTR("MyPort"), myCallBack, NULL, false);
     CFRunLoopSourceRef source = CFMessagePortCreateRunLoopSource(NULL, local, 0);
     CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
     CFRunLoopRun();    // will not return as long as message port is still valid and source remains on run loop
     CFRelease(local);
}
