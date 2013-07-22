/*
 *  date_test.c
 *  CFLite
 *
 *  A compilation of various examples from the "Date and Times Programming Guide for CoreFoundation".
 *
 *  http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFDatesAndTimes/CFDatesAndTimes.html
 *
 */

#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>

bool check_date_constructors ()
{
   CFAbsoluteTime      absTime;
   CFDateRef           aCFDate;
   
   CFShow(CFSTR("Checking date constructors:"));

   absTime = CFAbsoluteTimeGetCurrent();
   aCFDate = CFDateCreate(kCFAllocatorDefault, absTime);
   
   CFShow(CFSTR("Absolute Time is"));
   printf("The current absolute time is %f\n", absTime);
   CFShow(CFSTR("Equivalent CFDate object is"));
   CFShow(aCFDate);
   
   printf("\n");
   
   return true;
}

bool check_date_comparison ()
{
   CFDateRef           date1, date2;
   
   // Standard Core Foundation comparison result.
   CFComparisonResult result;
   
   CFShow(CFSTR("Checking date comparison functions:"));
   
   // Create two CFDates from absolute time.
   date1 = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent());
   date2 = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent());
   
   // Pass NULL for the context param.
   result = CFDateCompare(date1, date2, NULL);
   
   switch (result) {
      case kCFCompareLessThan:
         CFShow(CFSTR("date1 is before date2!\n"));
         break;
      case kCFCompareEqualTo:
         CFShow(CFSTR("date1 is the same as date2!\n"));
         break;
      case kCFCompareGreaterThan:
         CFShow(CFSTR("date1 is after date2!\n"));
         break;
   }
   
   printf("\n");

   return true;
}

bool check_gregorian_dates ()
{
   Boolean             status;
   CFGregorianDate     gregDate;
   CFAbsoluteTime      absTime;
   
   long                weekOfYear, dayOfWeek;
   
   CFShow(CFSTR("Checking Gregorian date functions"));
   
   // Construct a Gregorian date.
   gregDate.year = 1999;
   gregDate.month = 11;
   gregDate.day = 23;
   gregDate.hour = 17;
   gregDate.minute = 33;
   gregDate.second = 22.7;
   
   // Check the validity of the date.
   status = CFGregorianDateIsValid(gregDate, kCFGregorianAllUnits);
   printf("Is my Gregorian date valid? %d\n", status);
   
   // Convert the Gregorian date to absolute time.
   absTime = CFGregorianDateGetAbsoluteTime(gregDate, NULL);
   printf("The Absolute Time from a Gregorian date is: %f\n", absTime);
   
   CFShow(CFSTR("This corresponds to the following:"));
   weekOfYear = CFAbsoluteTimeGetWeekOfYear (absTime, NULL);
   dayOfWeek = CFAbsoluteTimeGetDayOfWeek (absTime, NULL);
   
   printf("Week of the year for %d-%d-%d is %ld\n", gregDate.month, gregDate.day, gregDate.year, weekOfYear);
   printf("Day of the week for %d-%d-%d is %ld\n", gregDate.month, gregDate.day, gregDate.year, dayOfWeek);

   printf("\n");

   return true;
}

int main (int argc, const char** argv)
{
   check_date_constructors ();
   check_date_comparison ();
   check_gregorian_dates ();
   return 0;
}