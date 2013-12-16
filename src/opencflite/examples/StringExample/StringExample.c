/*
 Simple CFString example program.
 Author: Ali Ozer
 4/2/99

 Note: Compile as "gcc -framework CoreFoundation -o stringtest StringExample.c"
 
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


void simpleStringExample(void) {

    CFStringRef str;
    CFDataRef data;
    char *bytes;

    show(CFSTR("------------------Simple Strings---------------"));

    // Create a simple immutable string from a Pascal string and convert it to Unicode
#if defined(__APPLE__)
    str = CFStringCreateWithPascalString(NULL, "\pFoo Bar", kCFStringEncodingASCII);
#else
    str = CFStringCreateWithCString(NULL, "Hello World", kCFStringEncodingASCII);
#endif

    // Create the Unicode representation of the string
    // "0", lossByte, indicates that if there's a conversion error, fail (and return NULL)
    data = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUnicode, 0);

    show(CFSTR("String          : %@"), str);
    show(CFSTR("Unicode data    : %@"), data);

    CFRelease(str);
 
    // Create a string from the Unicode data...
    str = CFStringCreateFromExternalRepresentation(NULL, data, kCFStringEncodingUnicode);

    show(CFSTR("String Out      : %@"), str);

    CFRelease(str);

    // Create a string for which you already have some allocated contents which you want to 
    // pass ownership of to the CFString. The last argument, "NULL," indicates that the default allocator
    // should be used to free the contents when the CFString is freed (or you can pass in CFAllocatorGetDefault()).

    bytes = (char*)CFAllocatorAllocate(CFAllocatorGetDefault(), 6, 0);
    strcpy(bytes, "Hello");

    str = CFStringCreateWithCStringNoCopy(NULL, bytes, kCFStringEncodingASCII, NULL);
    CFRelease(str);

#if defined(__APPLE__)
    // Now create a string with a Pascal string which is not copied, and not freed when the string is
    // This is an advanced usage; obviously you need to guarantee that the string bytes do not go away
    // before the CFString does. 
    str = CFStringCreateWithPascalStringNoCopy(NULL, "\pFoo Bar", kCFStringEncodingASCII, kCFAllocatorNull);
    CFRelease(str);
#endif
}



void stringGettingContentsAsCStringExample(void) {

    CFStringRef str;
    CFDataRef data;
    CFRange rangeToProcess;
    const char *bytes;

    show(CFSTR("------------------C String Manipulations---------------"));

    // Create some test CFString
    // Note that in general the string might contain Unicode characters which cannot
    // be converted to a 8-bit character encoding
    str = CFStringCreateWithCString(NULL, "Hello World", kCFStringEncodingASCII);

    show(CFSTR("Original String : %@"), str);
   
    // First, the fast but unpredictable way to get at the C String contents...
    // This is O(1), meaning it takes constant time.
    // This might return NULL!
    bytes = CFStringGetCStringPtr(str, kCFStringEncodingASCII);
   
    // If that fails, you can try to get the contents by copying it out
    if (bytes == NULL) {
        char localBuffer[10];
        Boolean success;

        // This might also fail, either if you provide a buffer that is too small, 
        // or the string cannot be converted into the specified encoding
        success = CFStringGetCString(str, localBuffer, 10, kCFStringEncodingASCII);
    }
    else
        show(CFSTR("From CStringPtr : %@"), bytes);   
    
    // A pretty simple solution is to use a CFData; this frees you from guessing at the buffer size
    // But it does allocate a CFData...
    data = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingASCII, 0);
    if (data) {
        show(CFSTR("External Rep: %@"), data);   
        bytes = (const char *)CFDataGetBytePtr(data);
    }

    // More complicated but efficient solution is to use a fixed size buffer, and put a loop in
    rangeToProcess = CFRangeMake(0, CFStringGetLength(str));

    while (rangeToProcess.length > 0) {
        UInt8 localBuffer[100];
        CFIndex usedBufferLength;
        CFIndex numChars = CFStringGetBytes(str, rangeToProcess, kCFStringEncodingASCII, 0, FALSE, (UInt8 *)localBuffer, 100, &usedBufferLength);

        if (numChars == 0) break;	// Means we failed to convert anything...

        // Otherwise we converted some stuff; process localBuffer containing usedBufferLength bytes
        // Note that the bytes in localBuffer are not NULL terminated
		
        // Update the remaining range to continue looping
        rangeToProcess.location += numChars;
        rangeToProcess.length -= numChars;
    }
}


void stringGettingAtCharactersExample(void) {

    CFStringRef str;
    const UniChar *chars;

    show(CFSTR("------------------Character Manipulations---------------"));

    // Create some test CFString
    str = CFStringCreateWithCString(NULL, "Hello World", kCFStringEncodingASCII);

    show(CFSTR("Original String : %@"), str);

    // The fastest way to get the contents; this might return NULL though
    // depending on the system, the release, etc, so don't depend on it 
    // (unless you used CFStringCreateMutableWithExternalCharactersNoCopy())
    chars = CFStringGetCharactersPtr(str);

    // If that fails, you can try copying the UniChars out
    // either into some stack buffer or some allocated piece of memory...
    // Using the former is fine, but you need to know the size; the latter
    // always works but requires allocating some memory; not too efficient.
    if (chars == NULL) {
        CFIndex length = CFStringGetLength(str);
        UniChar *buffer = (UniChar*)malloc(length * sizeof(UniChar));
        CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
        // Process the chars...
        free(buffer);
    }
    else
        show(CFSTR("Characters      : %@"), str);

    // You can use CFStringGetCharacterAtIndex() to get at the characters one at a time,
    // but doing a lot of characters this way might get slow...
    // An option is to use "inline buffer" functionality which mixes the convenience of
    // one-at-a-time char access with efficiency of bulk access
    {
        CFStringInlineBuffer inlineBuffer;
        CFIndex length = CFStringGetLength(str);
        CFIndex cnt;

        CFStringInitInlineBuffer(str, &inlineBuffer, CFRangeMake(0, length));

        for (cnt = 0; cnt < length; cnt++) {
            UniChar ch = CFStringGetCharacterFromInlineBuffer(&inlineBuffer, cnt);
            // Process character...
	         (void)ch;   // Dummy processing to prevent compiler warning...
        }
    }
    
}


void stringWithExternalContentsExample(void) {
#define BufferSize 1000
    CFMutableStringRef mutStr;
    UniChar *myBuffer;

    show(CFSTR("------------------External Contents Examples---------------"));

    // Allocate a contents store that is empty (but has space for BufferSize chars)...
    myBuffer = (UniChar*)malloc(BufferSize * sizeof(UniChar));

    // Now create a mutable CFString which uses this buffer
    // The 0 and BufferSize indicate the length and capacity (in UniChars)
    // The kCFAllocatorNull indicates how the CFString should reallocate or free this buffer (in this case, do nothing)

    mutStr = CFStringCreateMutableWithExternalCharactersNoCopy(NULL, myBuffer, 0, BufferSize, kCFAllocatorNull);
    CFStringAppend(mutStr, CFSTR("Appended string... "));
    CFStringAppend(mutStr, CFSTR("More stuff... "));
#if defined(__APPLE__)
    CFStringAppendPascalString(mutStr, "\pA pascal string. ", kCFStringEncodingASCII);
#else
    CFStringAppendCString(mutStr, "A C string. ", kCFStringEncodingASCII);
#endif
    CFStringAppendFormat(mutStr, NULL, CFSTR("%d %4.2f %@..."), 42, -3.14, CFSTR("Hello"));
    
    show(CFSTR("String: %@"), mutStr);

    CFRelease(mutStr);
    free(myBuffer);

    // Now create a similar string, but give CFString the ability to reallocate or free the buffer
    // The last "NULL" argument specifies that the default allocator should be used
    // Here we provide an initial buffer of 32 characters, but if it grows beyond this, it's OK
    // (unlike the previous example, where if the string grew beyond 1000, it's an error)

    myBuffer = (UniChar*)CFAllocatorAllocate(CFAllocatorGetDefault(), 32 * sizeof(UniChar), 0);
    mutStr = CFStringCreateMutableWithExternalCharactersNoCopy(NULL, myBuffer, 0, 32, NULL);

    CFStringAppend(mutStr, CFSTR("Appended string... "));
    CFStringAppend(mutStr, CFSTR("Appended string... "));
    CFStringAppend(mutStr, CFSTR("Appended string... "));
    CFStringAppend(mutStr, CFSTR("Appended string... "));
    CFStringAppend(mutStr, CFSTR("Appended string... "));

    show(CFSTR("String: %@"), mutStr);

    CFRelease(mutStr);
    // Here we don't free the buffer, as CFString does that
}

void stringManipulation(void) {
   
   CFMutableStringRef strChange;
   CFStringRef strOuter, find, replace, find2, replace2, find3, replace3, bigger, smaller, result;
   CFComparisonResult comp;
   CFLocaleRef curLocale;
   Boolean isHyphenationSupported = false;
   
   show(CFSTR("------------------String Manipulations---------------"));

   // Create a simple immutable string from a Pascal string and convert it to Unicode
   strOuter = CFStringCreateWithCString(NULL, "Hello Cruel World", kCFStringEncodingASCII);
   strChange = CFStringCreateMutableCopy(NULL, CFStringGetLength(strOuter), strOuter);
   find = CFStringCreateWithCString(NULL, "Cruel", kCFStringEncodingASCII);
   replace = CFStringCreateWithCString(NULL, "Cool", kCFStringEncodingASCII);
   find2 = CFStringCreateWithCString(NULL, "Keep", kCFStringEncodingASCII);
   replace2 = CFStringCreateWithCString(NULL, "Be", kCFStringEncodingASCII);
   find3 = CFStringCreateWithCString(NULL, "Change.", kCFStringEncodingASCII);
   replace3 = CFStringCreateWithCString(NULL, "Ball.", kCFStringEncodingASCII);

   bigger  = CFStringCreateWithCString(NULL, "version 2.5", kCFStringEncodingASCII);
   smaller = CFStringCreateWithCString(NULL, "version 2.0", kCFStringEncodingASCII);

   show(CFSTR("String Outer       : %@"), strOuter);
   show(CFSTR("String Find        : %@"), find);
   show(CFSTR("String Replace     : %@"), replace);
   
   CFStringFindAndReplace(strChange, find, replace, CFRangeMake(0, CFStringGetLength(strChange)), 0);
   
   show(CFSTR("Replaced           : %@"), strChange);
   
   CFStringAppendCString(strChange, "!  Keep the change.", kCFStringEncodingASCII);
   
   show(CFSTR("Appended           : %@"), strChange);
   
   curLocale = CFLocaleCopyCurrent ();

   isHyphenationSupported = CFStringIsHyphenationAvailableForLocale(curLocale);
   show(CFSTR("Is Hyphenation supported for this locale? %@"), ((isHyphenationSupported) ? CFSTR ("Yes") : CFSTR("No")));

   CFStringUppercase(strChange, curLocale);
   
   show(CFSTR("Upper Cased        : %@"), strChange);
   
   CFStringLowercase(strChange, curLocale);
   
   show(CFSTR("Lower Cased        : %@"), strChange);
   
   CFStringCapitalize(strChange, curLocale);
   
   show(CFSTR("Capitalized        : %@"), strChange);

   CFStringUppercase(strChange, curLocale);
   
   show(CFSTR("Up Cased (again)   : %@"), strChange);

   CFStringFindAndReplace(strChange, find2, replace2, CFRangeMake(0, CFStringGetLength(strChange)), 0);

   show(CFSTR("Replaced?          : %@"), strChange);
   
   CFStringFindAndReplace(strChange, find2, replace2, CFRangeMake(0, CFStringGetLength(strChange)), kCFCompareCaseInsensitive);
   
   show(CFSTR("Case insensitive   : %@"), strChange);
   
   CFStringCapitalize(strChange, curLocale);
   
   show(CFSTR("Capitalized        : %@"), strChange);

   CFStringFindAndReplace(strChange, replace2, find2, CFRangeMake(0, CFStringGetLength(strChange)), kCFCompareAnchored);
   
   show(CFSTR("Should Be Unchanged: %@"), strChange);

   CFStringFindAndReplace(strChange, find3, replace3, CFRangeMake(0, CFStringGetLength(strChange)), kCFCompareAnchored|kCFCompareBackwards);
   
   show(CFSTR("Should Be Changed  : %@"), strChange);
   
   show(CFSTR("Which is bigger %@ or %@?"), bigger, smaller);
   
   comp = CFStringCompare(bigger, smaller, 0);   
   result = (comp == kCFCompareGreaterThan) ? bigger : smaller;   
   show(CFSTR("Base Compare Says  : %@"), result);

   comp = CFStringCompare(bigger, smaller, kCFCompareNumerically);
   result = (comp == kCFCompareGreaterThan) ? bigger : smaller;   
   show(CFSTR("Numerical Compare  : %@"), result);

   CFRelease(curLocale);
   CFRelease(replace);
   CFRelease(find);
   CFRelease(replace2);   
   CFRelease(find2);
   CFRelease(replace3);   
   CFRelease(find3);
   CFRelease(bigger);   
   CFRelease(smaller);
   CFRelease(strChange);
}

Boolean equalValues(CFStringRef number, CFNumberRef expected, CFNumberFormatterStyle style, CFNumberFormatterOptionFlags option)
{
   //CFStringRef enLocaleIdentifier = CFSTR("en_US");
   //CFLocaleRef curLocale = CFLocaleCreate(NULL, enLocaleIdentifier);
   
   CFLocaleRef curLocale = CFLocaleCopyCurrent();
   CFStringRef identifier = CFLocaleGetIdentifier(curLocale);
   CFNumberFormatterRef fmt;
   CFNumberRef val;

   show(CFSTR("Make a Number from : %@"), number);
   
   fmt = CFNumberFormatterCreate (0, curLocale, style);
   val = CFNumberFormatterCreateNumberFromString(0, fmt, number, 0, option);
   
   show(CFSTR("val=%@, should be=%@\n"), val, expected);
   
   if (!val)
      return false;
   
   return (0 == CFNumberCompare(val, expected, 0));
}

void stringHandling(void) {
   
   CFStringRef number;
   int theValue = 80;
   double theOtherValue = 123.26;
   CFNumberRef expected = CFNumberCreate(0, kCFNumberIntType, &theValue);

   show(CFSTR("------------------Number Magic---------------"));
   show(CFSTR("1.  Integer Parsing"));
   show(CFSTR("   (a) Decimal Style"));

   number = CFStringCreateWithCString(NULL, "80.0", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterDecimalStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (b) Currency Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "$80.00", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterCurrencyStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (c) Percent Style (does not work for integers)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "80%", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterPercentStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (d) Scientific Notation Style (does not work for integers)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "8.0E1", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterScientificStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (e) Spell-Out Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "eighty", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterSpellOutStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (f) No Style (decimal)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "80.0", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterNoStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (g) No Style (spell out) (is not expected to work)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "eighty", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterNoStyle, kCFNumberFormatterParseIntegersOnly))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("2.  Decimal Parsing"));
   show(CFSTR("   (a) Decimal Style"));

   CFRelease(expected);
   expected = CFNumberCreate(0, kCFNumberDoubleType, &theOtherValue);
   
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "123.26", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterDecimalStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (b) Currency Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "$123.26", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterCurrencyStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (c) Percent Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "123.26%", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterPercentStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (d) Scientific Notation Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "1.2326e2", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterScientificStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (e) Spell-Out Style"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "one hundred twenty-three point two six", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterSpellOutStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (f) No Style (decimal)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "123.26", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterNoStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   show(CFSTR("   (g) No Style (spell-out) (not expected to work)"));
   CFRelease(number);
   number = CFStringCreateWithCString(NULL, "one hundred twenty three point two six", kCFStringEncodingASCII);   
   if (equalValues(number, expected, kCFNumberFormatterNoStyle, 0))
      show(CFSTR("correct."));
   else
      show(CFSTR("WRONG!!!"));
   
   CFRelease(number);
}

int main () {   
    simpleStringExample();
    stringGettingContentsAsCStringExample();
    stringGettingAtCharactersExample();
    stringWithExternalContentsExample();
    stringManipulation();
    stringHandling();
   
    return 0;
}


