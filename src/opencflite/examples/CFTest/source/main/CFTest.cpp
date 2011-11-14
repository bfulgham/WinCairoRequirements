#include "stdafx.h"
#include "SuperString.h"
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFNumberFormatter.h>
#include <CoreFoundation/CFDateFormatter.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFBundle.h>
#include "CFTest.h"

static SuperString		GetAccentedString()
{
	return 	SuperString("%C3%B1%C3%A9%C3%BC%C3%AE", kCFStringEncodingPercentEscapes);
}

static void	ShowDiacriticSensitiveCompare(bool insensitiveB)
{
	SuperString		resultStr;

	g_pref_diacritic_insensitive_searchB = insensitiveB;
	
	CCFLog()((CFTypeRef)resultStr.ssprintf("diacritic %ssensitive\n", insensitiveB ? "in" : "").ref());
	
	SuperString			str1("NeUi");
	SuperString			str2(GetAccentedString());
	
	bool				diacritic_insensitive_compareB = str1 == str2;
	
	CCFLog()((CFTypeRef)resultStr.ssprintf("%s %s %s\n", 
		   str1.utf8Z(), 
		   diacritic_insensitive_compareB ? "==" : "!=", 
		   str2.utf8Z()).ref());
	
	if (insensitiveB) {
		CF_ASSERT(diacritic_insensitive_compareB);
	} else {
		CF_ASSERT(!diacritic_insensitive_compareB);
	}
}

static void	ShowCompare(const SuperString& str1, const SuperString& str2)
{
	SuperString			format("%s %s %s");
	const char*			ineqZ(str1 < str2 ? "<" : (str2 < str1 ? ">" : "=="));

	format.ssprintf(NULL, str1.utf8Z(), ineqZ, str2.utf8Z());
	CCFLog(true)(format.ref());
}

/******************************************************/
class CParseXML {
	#define						kLogDuringParse		1
	CFStringRef					i_keyStr;
	CFXMLNodeTypeCode			i_lastType;
	
	#if kLogDuringParse
	void	IndentLevel(short levelS) {
		loop (levelS) {
			CCFLog()(CFSTR("\t"));
		}
	}
	#endif

	public: 
	CParseXML() : i_lastType(kCFXMLNodeTypeDocument) { }
	
	void operator()(CCFXMLNode& node, short levelS)
	{
		CFXMLNodeTypeCode	type = node.GetTypeCode();
		
		#if kLogDuringParse
			if (type == i_lastType) {
				CCFLog()(CFSTR("\n"));
			}
		#endif
		
		i_lastType = type;
		
		switch (type) {
				
			case kCFXMLNodeTypeElement: {
				i_keyStr = node.GetString();
				
				#if kLogDuringParse
					IndentLevel(levelS);
					CCFLog()(i_keyStr);
					
					if (!node.GetInfoPtr()->attributes) {
						CCFLog()(CFSTR(": "));
					}
				#endif
				break;
			}
				
			case kCFXMLNodeTypeText: {
				SuperString			valueStr(node.GetString());
				
				#if kLogDuringParse
					CCFLog()(valueStr.ref());
				#endif
				break;
			}
		}
	}
};

#ifdef __MWERKS__
static CFGregorianDate			CFGetGregorianDate(CFAbsoluteTime at, CFTimeZoneRef tz)
{
	CFGregorianDate		gregDate;

	CFAbsoluteTimeGetGregorianDate_MW(at, tz, &gregDate);
	return gregDate;
}
#else
	#define		CFGetGregorianDate		CFAbsoluteTimeGetGregorianDate
#endif

void	CFTest()
{
	SuperString		resultStr;
	
	if (!IsDefaultEncodingSet()) {
		SetDefaultEncoding(kCFStringEncodingASCII);
	}
	
	{
		CCFLog()(CFSTR("------------------Strings---------------\n"));
		CCFLog()(CFSTR("Hello, World!\n"));
		ScCFReleaser<CFDataRef>		dataRef(SuperString("yeah baby").CopyDataRef());
		SuperString					str;
		
		str.Set(dataRef);
		CCFLog()(SuperString("The next line should read <yeah baby>\n").ref());
		CCFLog(true)(str.ref());
		
		SuperString		str1("foscoobyar");
		SuperString		str2("scooby");
		
		resultStr.ssprintf(
			"\n%s %s %s\n", 
			str1.utf8Z(), 
			str1.Contains(str2) ? "contains" : "$$$ Error: does not contain(!?)", 
			str2.utf8Z());
		
		CCFLog()(resultStr.ref());
		
		str1.Replace(str2, "o b");
		CCFLog()(SuperString("The next line should read <foo bar>\n").ref());
		CCFLog(true)(str1.ref());
		
		CCFLog()(CFSTR("------------------Case Insensitive Compare---------------\n"));
		ShowDiacriticSensitiveCompare(false);
		CCFLog()(CFSTR("\n"));
		ShowDiacriticSensitiveCompare(true);
		
		{
			CCFLog()(CFSTR("------------------Inequality---------------\n"));
			SuperString			catStr("Cat");
			SuperString			dogStr("Dog");
			
			ShowCompare(catStr, dogStr);
			CF_ASSERT(catStr < dogStr);
			
			ShowCompare(dogStr, catStr);
			CF_ASSERT(dogStr > catStr);

			ShowCompare(catStr, catStr);
			CF_ASSERT(catStr == catStr);
		}
		
		{
			CCFLog()(CFSTR("------------------Conversion---------------\n"));
			SuperString				jStr(GetAccentedString());
			
			#if defined(__WIN32__)
				#define		kConvertEncode		kCFStringEncodingWindowsLatin1
			#else
				#define		kConvertEncode		kCFStringEncodingMacRoman
			#endif
			
			ustring				j;
			CopyCFStringToUString(jStr.ref(), j, kConvertEncode);
			
			SuperString			convertedJ;  convertedJ.Set(j, kConvertEncode);
			
			CCFLog()(resultStr.ssprintf(
				"The next line should read <%s>\n%s\n", 
				jStr.utf8Z(), convertedJ.utf8Z()).ref());
			
			CCFLog()(resultStr.ssprintf("conversion: %s\n", convertedJ == jStr ? "Success!" : "$$ FAILED!").ref());
		}
	}
	
	{
		CCFLog()(CFSTR("------------------Encoding---------------\n"));
		CFStringEncoding		encoding = CFStringGetSystemEncoding();
		
		CCFLog()(resultStr.ssprintf("Encoding: %s\n", SuperString(CFStringGetNameOfEncoding(encoding)).utf8Z()).ref());
		CCFLog()(resultStr.ssprintf("IANA charset: %s\n", SuperString(CFStringConvertEncodingToIANACharSetName(encoding)).utf8Z()).ref());
		
		#if defined(__WIN32__)
			UInt32					codePage = CFStringConvertEncodingToWindowsCodepage(encoding);
			
			if (encoding == kCFStringEncodingWindowsLatin1) {
				CF_ASSERT(codePage == kCodePage_WindowsLatin1);
			}
			
			CCFLog()(resultStr.ssprintf("codepage: %ld\n", codePage).ref());
			CF_ASSERT(CFStringConvertWindowsCodepageToEncoding(codePage) == encoding);
		#endif		
	}
	
	CCFLog()(CFSTR("------------------Locale---------------\n"));
	ScCFReleaser<CFLocaleRef>				locale(CFLocaleCopyCurrent());
	
	{
		SuperString				localIdStr(CFLocaleGetIdentifier(locale));
		
		CCFLog()(resultStr.ssprintf("Locale ID: %s\n", localIdStr.utf8Z()).ref());
		
		CCFDictionary			dictRef(CFLocaleCreateComponentsFromLocaleIdentifier(
			kCFAllocatorDefault, localIdStr.ref()));
		
		dictRef.for_each(CCFLog(true));
	}
	
	{
		CCFLog()(CFSTR("------------------Preferred Languages---------------\n"));
		ScCFReleaser<CFArrayRef>	arrayRef(CFLocaleCopyPreferredLanguages());
		
		array_for_each(arrayRef, CCFLog(true));
	}
	
	{
		CCFLog()(CFSTR("------------------Calendar---------------\n"));
		ScCFReleaser<CFCalendarRef>		calendarRef(CFCalendarCopyCurrent());
		
		CCFLog()(resultStr.ssprintf("Calendar ID: %s\n", SuperString(CFCalendarGetIdentifier(calendarRef)).utf8Z()).ref());
		
		ScCFReleaser<CFTimeZoneRef>		timeZoneRef(CFCalendarCopyTimeZone(calendarRef));
		CFAbsoluteTime					absTime(CFAbsoluteTimeGetCurrent());
		ScCFReleaser<CFDateRef>			dateRef(CFDateCreate(kCFAllocatorDefault, absTime));
		
		CCFLog(true)(timeZoneRef.Get());
		CCFLog(true)(dateRef.Get());
		
		CFGregorianDate					gregDate(CFGetGregorianDate(absTime, timeZoneRef));
		
//		CFAbsoluteTimeGetGregorianDate_MW(absTime, timeZoneRef, &gregDate);
		CCFLog()(resultStr.ssprintf("year: %d\nmonth: %d\nday: %d\nhour: %d\nminute: %d\nsecond: %f\n", 
			   (int)gregDate.year, 
			   (int)gregDate.month, 
			   (int)gregDate.day, 
			   (int)gregDate.hour, 
			   (int)gregDate.minute, 
			   (float)gregDate.second).ref());
		
		ScCFReleaser<CFDateFormatterRef>	dateFormatterRef(CFDateFormatterCreate(
			kCFAllocatorDefault, locale, kCFDateFormatterFullStyle, kCFDateFormatterFullStyle));
		
		ScCFReleaser<CFStringRef>			dateStr(CFDateFormatterCreateStringWithDate(
			kCFAllocatorDefault, dateFormatterRef, dateRef));
		
		CCFLog()(CFSTR("Make sure time zone is correct in the next line:\n"));
		CCFLog(true)(dateStr.Get());
	}
	
	{
		CCFLog()(CFSTR("------------------Numbers---------------\n"));
		ScCFReleaser<CFNumberFormatterRef>		numFormatRef;
		ScCFReleaser<CFStringRef>				numStr;
		float									numF = 123456.789f;
		ScCFReleaser<CFNumberRef>				numberRef(CFNumberCreate(
			kCFAllocatorDefault, kCFNumberFloat32Type, &numF));
		
		numFormatRef.adopt(CFNumberFormatterCreate(
			kCFAllocatorDefault, locale, kCFNumberFormatterDecimalStyle));
		numStr.adopt(CFNumberFormatterCreateStringWithNumber(
			kCFAllocatorDefault, numFormatRef, numberRef));
		CCFLog(true)(numStr.Get());
		
		numFormatRef.adopt(CFNumberFormatterCreate(
			kCFAllocatorDefault, locale, kCFNumberFormatterCurrencyStyle));
		numStr.adopt(CFNumberFormatterCreateStringWithNumber(
			kCFAllocatorDefault, numFormatRef, numberRef));
		CCFLog(true)(numStr.Get());
	}
	
	{
		CCFLog()(CFSTR("------------------Bundle---------------\n"));	
		
		ScCFReleaser<CFURLRef>			bundleUrlRef;
		ScCFReleaser<CFBundleRef>		bundleRef(CFBundleGetMainBundle(), true);
		
		if (bundleRef.Get() == NULL) {
			CCFLog()(CFSTR("$$ Failed getting bundle!\n"));	
		} else {
			bundleUrlRef.adopt(CFBundleCopyBundleURL(bundleRef));
		}
		
		if (bundleUrlRef.Get() != NULL) {
			
			CCFLog(true)(bundleUrlRef.Get());
			
			if (bundleRef.Get() != NULL) {
				CCFDictionary	dictRef(CFBundleGetInfoDictionary(bundleRef), true);
				
				dictRef.for_each(CCFLog(true));
			}
			
				#if defined(__WIN32__)
					//	you may need to fix this relative URL here
					SuperString				relPathStr("../../");
				#else
					SuperString				relPathStr("../../../");
				#endif
				
			{
				CCFLog()(CFSTR("------------------plist---------------\n"));

				SuperString				testRelPath("test.xml");	testRelPath.prepend(relPathStr);
				ScCFReleaser<CFURLRef>	xmlUrlRef(CFURLCreateWithFileSystemPathRelativeToBase(
					kCFAllocatorDefault, testRelPath.ref(), kCFURLPOSIXPathStyle, false, bundleUrlRef));
				
				if (xmlUrlRef.Get()) {
					CCFDictionary						dictRef;
					ScCFReleaser<CFURLRef>				absUrlRef(CFURLCopyAbsoluteURL(xmlUrlRef));
					
					CCFLog()(resultStr.ssprintf("URL: %s\n", SuperString(CFURLGetString(absUrlRef)).utf8Z()).ref());
					
					if (Read_PList(xmlUrlRef, dictRef.ImmutableAddressOf())) {
						dictRef.for_each(CCFLog(true));
						
						SuperString				outRelpath("out.xml");	outRelpath.prepend(relPathStr);
						ScCFReleaser<CFURLRef>	outXmlUrlRef(CFURLCreateWithFileSystemPathRelativeToBase(
							kCFAllocatorDefault, outRelpath.ref(), kCFURLPOSIXPathStyle, false, bundleUrlRef));
						
						Write_PList(dictRef.Get(), outXmlUrlRef);
					}
				} else {
					CCFLog()(CFSTR("error illegal plist path?\n"));
				}
			}

			{
				CCFLog()(CFSTR("------------------xml---------------\n"));

				SuperString				testRelPath("Chiquitita.xml");	testRelPath.prepend(relPathStr);
				ScCFReleaser<CFURLRef>	xmlUrlRef(CFURLCreateWithFileSystemPathRelativeToBase(
					kCFAllocatorDefault, testRelPath.ref(), kCFURLPOSIXPathStyle, false, bundleUrlRef));
				
				if (xmlUrlRef.Get()) {
					CCFXmlTree					xml;
					ScCFReleaser<CFURLRef>		absUrlRef(CFURLCopyAbsoluteURL(xmlUrlRef));
					
					CCFLog()(resultStr.ssprintf("URL: %s\n", SuperString(CFURLGetString(absUrlRef)).utf8Z()).ref());
					
					if (Read_XML(xmlUrlRef, xml)) {
						CCFLog()(CFSTR("XML file read successfully\n"));
						//xml.for_each(CParseXML());
					} else {
						CCFLog()(CFSTR("$$ FAIL reading xml file\n"));
					}
				} else {
					CCFLog()(CFSTR("error illegal xml path?\n"));
				}
			}
		}
	}
		
	/*
	 bonus points:  CFCharacterSet
	 */
	CCFLog()(CFSTR("--------------------------------------\n"));
}
