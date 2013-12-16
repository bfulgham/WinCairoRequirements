/*
 *  CFUtils.cpp
 *  CFTest
 *
 *  Created by David M. Cotter on 7/23/08.
 *  Copyright 2008 David M. Cotter. All rights reserved.
 *
 */
#include "stdafx.h"

#if defined(_KJAMS_)  && !defined(_KJAMSX_)
	#include "StringUtils.h"
	#include "MessageAlert.h"
#else
	#ifndef _CFTEST_
		#include "XConfig.h"
	#endif

	#include "SuperString.h"
	#include <CoreFoundation/CFDateFormatter.h>
	#include <CoreFoundation/CFNumberFormatter.h>
	#include <CoreFoundation/CFCalendar.h>
#endif

#ifndef _CFTEST_
	#if _KJ_MAC_
		#include "MessageAlert.h"
	#else
		#include <XErrorDialogs.h>
	#endif

	#include "CLocalize.h"
#endif

void	FilterErr(OSStatus err);
void	FilterErr(OSStatus err)
{
	#ifdef kDEBUG
		if (err == eofErr) {
			err = eofErr;
		}
	#endif
}

#if defined(__WIN32__)
	static	bool	s_setLogB = false;
#endif

void CCFLog::operator()(CFTypeRef valRef) {
	#define		USE_CFSHOW		0
	
	#if defined(__WIN32__)
		if (!s_setLogB) {
			s_setLogB = true;
		//	CFSetLogFile(CFSTR("CF_Log.txt"), kCFStringEncodingUTF8);
		}
	#endif

	#if !USE_CFSHOW
		SuperString			valStr;	valStr.Set_CFType(valRef);
		FILE				*log_fileP = stdout;
		
		if (i_crB) {
			valStr.append("\n");
		}
		
		#if defined(__WIN32__)
			
			#ifdef __MWERKS__
				//	console app
				static	HANDLE	consoleH = NULL;
				
				if (!consoleH) {
					consoleH = GetStdHandle(STD_OUTPUT_HANDLE);
				}
				
				unsigned long	outL;
				
				WriteConsoleW(consoleH, valStr.w_str(), valStr.w_strlen(), &outL, NULL);
				
				log_fileP = NULL;
			#else
				//	GUI APP
				static	FILE	*s_logP = NULL;
				
				if (s_logP == NULL) {
					(void)fopen_s(&s_logP, kJams_LogFileName, "a");
				}
				
				log_fileP = s_logP;
			#endif
	
		#endif
		
		if (log_fileP) {
			
			#ifdef _KJAMSX_
				fprintf(log_fileP, "%s", valStr.utf8Z());
				fflush(log_fileP);
			#else
				#if _CFTEST_
					fprintf(log_fileP, "%s", valStr.utf8Z());
					fflush(log_fileP);
				#else
					Log(valStr.utf8Z());
				#endif
			#endif
		}
	#else

		#ifndef __WIN32__
			fflush(stdout);
		#endif
	
		CFShow(valRef);
		if (i_crB) {
			CFShow(CFSTR("\n"));
		}

		#ifndef __WIN32__
			fflush(stdout);
		#endif
	#endif
}

void CCFLog::operator()(CFStringRef keyRef, CFTypeRef valRef) {
	SuperString		keyStr(keyRef);
	
	keyStr.append(": ");
	
	{
		bool	wasB = i_crB;
		i_crB = false;
		
		operator()(keyStr.ref());
		i_crB = wasB;
	}
	
	operator()(valRef);
}

/*****************************************************************************/
bool	Read_PList(const CFURLRef &url, CFDictionaryRef *plistP)
{
	bool						successB = false;
	ScCFReleaser<CFDataRef>     xmlData;
	
	*plistP = NULL;
	
	if (CFURLCreateDataAndPropertiesFromResource(
		kCFAllocatorDefault, url, xmlData.AddressOf(), NULL, NULL, NULL)
	) {
		//Log("created xml from file");
		
		*plistP = (CFDictionaryRef)CFPropertyListCreateFromXMLData(
			kCFAllocatorDefault,
			xmlData,
			kCFPropertyListMutableContainersAndLeaves,
			NULL);
		
		successB = *plistP != NULL;
		
		if (successB) {
			//	Log("created plist from xml");
		} else {
		//	CCFLog()(CFSTR("FAILED converting xml to plist\n"));
		}
	} else {
		//CCFLog()(CFSTR("FAILED creating xml from file\n"));
	}
	
	return successB;
}

OSStatus		Write_PList(
	CFPropertyListRef	plist,
	CFURLRef			urlRef)
{
	OSStatus							err	= noErr;
	ScCFReleaser<CFDataRef>				xmlData;
	
	// Convert the property list into XML data.
	xmlData.Set(CFPropertyListCreateXMLData(kCFAllocatorDefault, plist));
	ETRL(xmlData.Get() == NULL, "creating xml data");
	
	if (!err) {
		(void)CFURLWriteDataAndPropertiesToResource (
			urlRef,		// URL to use
			xmlData,		// data to write
			NULL,   
			&err);
	}
	
	ETRL(err, "writing xml");
	
	return err;
}

OSErr	CFDictionaryCreate(CFMutableDictionaryRef *dictP)
{
	OSErr		err = noErr;
	
	*dictP = CFDictionaryCreateMutable(
		kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	
	if (*dictP == NULL) {
		err = 1;
	}
	
	return err;
}

OSErr	CFArrayCreate(CFMutableArrayRef *arrayP)
{
	OSErr		err = noErr;
	
	*arrayP = CFArrayCreateMutable(
		kCFAllocatorDefault, 0,
		&kCFTypeArrayCallBacks);
	
	if (*arrayP == NULL) {
		err = 1;
	}
	
	return err;
}

/********************************************************/
bool	CCFXmlTree::CreateFromData(CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions)
{
	CCFDictionary	dict(NULL, false);
	CFXMLTreeRef	treeRef = CFXMLTreeCreateFromDataWithError(
			kCFAllocatorDefault, 
			xmlData, 
			dataSource, 
			parseOptions, 
			kCFXMLNodeCurrentVersion,
			dict.ImmutableAddressOf());

	_inherited::adopt(treeRef);
	
	if (treeRef == NULL) {
		CCFLog(true)(dict);
	}

	return treeRef != NULL;
}

bool	Read_XML(const CFURLRef url, CCFXmlTree& xml)
{
	bool						successB = false;
	ScCFReleaser<CFDataRef>		xmlData;
	
	if (CFURLCreateDataAndPropertiesFromResource(
		kCFAllocatorDefault, url, xmlData.AddressOf(), NULL, NULL, NULL)
	) {
		successB = xml.CreateFromData(xmlData.Get(), url, kCFXMLParserSkipWhitespace | kCFXMLParserReplacePhysicalEntities);
	}
	
	return successB;
}

/********************************************************/
CFMutableDictionaryRef		CCFDictionary::Copy()
{
	return CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, Get());
}

bool				CCFDictionary::ContainsKey(CFStringRef keyRef) {
	return !!CFDictionaryContainsKey(Get(), keyRef);
}

bool				CCFDictionary::ContainsKey(const char *utf8Z) {
	SuperString		dictKey(uc(utf8Z));
	
	return ContainsKey(dictKey.ref());
}

CFTypeRef	CCFDictionary::GetValue(const char* keyZ)
{
	SuperString		keyStr(uc(keyZ));

	return GetValue(keyStr.ref());
}

CFStringRef			CCFDictionary::GetAs_String	(const char *utf8Z)
{
	CFStringRef		stringRef((CFStringRef)GetValue(utf8Z));
	
	if (stringRef) {
		CF_ASSERT(CFGetTypeID(stringRef) == CFStringGetTypeID());
	}

	return stringRef;
}

CFDictionaryRef			CCFDictionary::GetAs_Dict	(const char *utf8Z)
{
	CFDictionaryRef		dictRef((CFDictionaryRef)GetValue(utf8Z));

	if (dictRef) {
		CF_ASSERT(CFGetTypeID(dictRef) == CFDictionaryGetTypeID());
	}
	
	return dictRef;
}

SInt32				CCFDictionary::GetAs_SInt32	(const char *utf8Z)
{
	CFNumberRef		valRef((CFNumberRef)GetValue(utf8Z));
	SInt32			valL = 0;

	if (valRef) {
		CF_ASSERT(CFGetTypeID(valRef) == CFNumberGetTypeID());
		CFNumberGetValue(valRef, kCFNumberSInt32Type, &valL);
	}

	return valL;
}

SInt16				CCFDictionary::GetAs_SInt16	(const char *utf8Z)
{
	CFNumberRef		valRef((CFNumberRef)GetValue(utf8Z));
	SInt16			valL = 0;

	if (valRef) {
		CF_ASSERT(CFGetTypeID(valRef) == CFNumberGetTypeID());
		CFNumberGetValue(valRef, kCFNumberSInt16Type, &valL);
	}

	return valL;
}

CFDateRef			CCFDictionary::GetAs_Date	(const char *utf8Z)
{
	CFDateRef		dateRef((CFDateRef)GetValue(utf8Z));
	
	if (dateRef) {
		CF_ASSERT(CFGetTypeID(dateRef) == CFDateGetTypeID());
	}

	return dateRef;
}

CFArrayRef			CCFDictionary::GetAs_Array	(const char *utf8Z)
{
	CFArrayRef			arrayRef((CFArrayRef)GetValue(utf8Z));
	
	if (arrayRef) {
		CF_ASSERT(CFGetTypeID(arrayRef) == CFArrayGetTypeID());
	}
	
	return arrayRef;
}

OSStatus		CFWriteDataToURL(const CFURLRef urlRef, CFDataRef dataRef)
{
	OSStatus	err = noErr;

	(void)CFURLWriteDataAndPropertiesToResource (
		urlRef,			// URL to use
		dataRef,		// data to write
		NULL,			// dictRef meta properties to write
		&err);
	
	return err;
}

void			CCFDictionary::RemoveValue(CFStringRef key)
{
	CFDictionaryRemoveValue(Get(), key);
}

void			CCFDictionary::RemoveValue(const char *utf8Z)
{
	RemoveValue(SuperString(uc(utf8Z)).ref());
}
/*****************************************/
Rect			CCFDictionary::GetAs_Rect		(const char *utf8Z)
{
	Rect				frameR; structclr(frameR);
	CCFDictionary		dict(GetAs_Dict(utf8Z), true);
	
	if (dict.Get()) {
		frameR.left		= dict.GetAs_SInt16(DICT_STR_RECT_LEFT);
		frameR.right	= dict.GetAs_SInt16(DICT_STR_RECT_RIGHT);
		frameR.top		= dict.GetAs_SInt16(DICT_STR_RECT_TOP);
		frameR.bottom	= dict.GetAs_SInt16(DICT_STR_RECT_BOTTOM);
	}
	
	return frameR;
}

void			CCFDictionary::SetValue(const char *utf8Z, const Rect& frameR)
{
	CCFDictionary	dict;
	
	dict.SetValue(DICT_STR_RECT_LEFT,	frameR.left);
	dict.SetValue(DICT_STR_RECT_RIGHT,	frameR.right);
	dict.SetValue(DICT_STR_RECT_TOP,	frameR.top);
	dict.SetValue(DICT_STR_RECT_BOTTOM,	frameR.bottom);

	SetValue(utf8Z, dict.Get());
}

/*****************************************/
bool				CCFDictionary::GetAs_Bool		(const char *utf8Z)
{
	CFBooleanRef		boolRef	= (CFBooleanRef)GetValue(utf8Z);
	
	if (boolRef) {
		CF_ASSERT(CFGetTypeID(boolRef) == CFBooleanGetTypeID());
	}

	return boolRef == kCFBooleanTrue;
}

void				CCFDictionary::SetValue			(const char *utf8Z, bool valB)
{
	SetValue(utf8Z, valB ? kCFBooleanTrue : kCFBooleanFalse);
}
/*****************************************/
float				CCFDictionary::GetAs_Float		(const char *utf8Z)
{
	float				valF = 0;
	CFNumberRef			valRef((CFNumberRef)GetValue(utf8Z));
	
	if (valRef) {
		CF_ASSERT(CFGetTypeID(valRef) == CFNumberGetTypeID());
		CFNumberGetValue(valRef, kCFNumberFloatType, &valF);
	}
		
	return valF;
}

void				CCFDictionary::SetValue			(const char *utf8Z, float valF)
{
	ScCFReleaser<CFNumberRef>	numberRef(CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &valF));

	SetValue(utf8Z, numberRef.Get());
}

/*****************************************/
RGBColor			CCFDictionary::GetAs_Color		(const char *utf8Z)
{
	RGBColor			valR		= { 0, 0, 0 };
	CCFDictionary		dict(GetAs_Dict(utf8Z), true);
	
	if (dict.Get()) {
		valR.red		= (UInt16)GetAs_UInt32(DICT_STR_COLOR_RED);
		valR.green		= (UInt16)GetAs_UInt32(DICT_STR_COLOR_GREEN);
		valR.blue		= (UInt16)GetAs_UInt32(DICT_STR_COLOR_BLUE);
	}
	
	return valR;
}

void				CCFDictionary::SetValue(const char *utf8Z, const RGBColor& value)
{
	CCFDictionary		dict;
	
	dict.SetValue(DICT_STR_COLOR_RED,	(UInt32)value.red);
	dict.SetValue(DICT_STR_COLOR_GREEN, (UInt32)value.green);
	dict.SetValue(DICT_STR_COLOR_BLUE,	(UInt32)value.blue);
	
	SetValue(utf8Z, dict.Get());
}
/*****************************************/

CFNumberRef			CFNumberCreateWithSInt32(SInt32 valL)
{
	return CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &valL);
}

void				CCFDictionary::SetValue(CFStringRef keyRef, SInt32 value)
{
	ScCFReleaser<CFNumberRef>	numberRef(CFNumberCreateWithSInt32(value));
	
	SetValue(keyRef, numberRef.Get());
}

void				CCFDictionary::SetValue(const char *utf8Z, SInt32 value)
{
	ScCFReleaser<CFNumberRef>	numberRef(CFNumberCreateWithSInt32(value));
	
	SetValue(utf8Z, numberRef.Get());
}

void				CCFDictionary::SetValue(const char *utf8Z, SInt16 value)
{
	ScCFReleaser<CFNumberRef>	numberRef(CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &value));
	
	SetValue(utf8Z, numberRef.Get());
}

void				CCFDictionary::SetValue(const char *utf8Z, CFTypeRef val)
{
	SuperString		keyStr(uc(utf8Z));
	
	SetValue(keyStr.ref(), val);
}
 
void				CCFDictionary::SetValue(const char *utf8Z, const SuperString& value)
{
	SetValue(utf8Z, value.ref());
}

/**********************************************************************/
#ifndef __WIN32__
CFDateRef 		CFDateCreateWithLongDateTime(const LongDateTime &ldt)
{
	CFDateRef		dateRef = NULL;
	
	if (ldt != 0) {
		OSStatus		err		= noErr;
		CFAbsoluteTime	absT;
		
		err = UCConvertLongDateTimeToCFAbsoluteTime(ldt, &absT);
		if (!err) {
			dateRef = CFDateCreate(kCFAllocatorDefault, absT);
		}
	}
	
	return dateRef;
}

LongDateTime	CFDateToLongDateTime(CFDateRef dateRef)
{
	LongDateTime		ldt;	structclr(ldt);
	
	if (dateRef) {
		CFAbsoluteTime		absT = CFDateGetAbsoluteTime(dateRef);

		UCConvertCFAbsoluteTimeToLongDateTime(absT, &ldt);
	}

	return ldt;
}
#endif

CFDateRef		CFDateCreateWithGregorian(const CFGregorianDate& gregDate)
{
	ScCFReleaser<CFCalendarRef>		calendarRef(CFCalendarCopyCurrent());
	ScCFReleaser<CFTimeZoneRef>		timeZoneRef(CFCalendarCopyTimeZone(calendarRef));
	
	return CFDateCreate(kCFAllocatorDefault, CFGregorianDateGetAbsoluteTime(gregDate, timeZoneRef));
}

CFStringRef		CFStringCreateWithDate(CFDateRef dateRef, bool showTimeB)
{
	ScCFReleaser<CFLocaleRef>			localeRef(CFLocaleCopyCurrent());
	ScCFReleaser<CFDateFormatterRef>	formatterRef(CFDateFormatterCreate(
		kCFAllocatorDefault, localeRef, 
		kCFDateFormatterShortStyle, showTimeB ? kCFDateFormatterShortStyle : kCFDateFormatterNoStyle));
	CFStringRef							ref(CFDateFormatterCreateStringWithDate(
		kCFAllocatorDefault, formatterRef, dateRef));
	
	return (CFStringRef)CFRetainDebug(ref, false); // don't retain, just track that it's existing refcount
}

CFStringRef		CFStringCreateWithNumber(CFNumberRef numRef)
{
	ScCFReleaser<CFLocaleRef>			localeRef(CFLocaleCopyCurrent());
	ScCFReleaser<CFNumberFormatterRef>	formatterRef(CFNumberFormatterCreate(
		kCFAllocatorDefault, localeRef, kCFNumberFormatterNoStyle));
	CFStringRef							ref(CFNumberFormatterCreateStringWithNumber(
		kCFAllocatorDefault, formatterRef, numRef));

	return (CFStringRef)CFRetainDebug(ref, false);
}

/*
CFStringRef		CFCopyBundleResourcesFSPath()
{
	ScCFReleaser<CFBundleRef>		exeRef(CFBundleGetMainBundle(), true);
	ScCFReleaser<CFURLRef>			exeUrlRef(CFBundleCopyBundleURL(exeRef));
	SuperString						dirStr(CFURLCopyFileSystemPath(exeUrlRef, kCFURLPlatformPathStyle));
	
	dirStr.append(kCFURLPlatformPathSeparator "Contents" kCFURLPlatformPathSeparator "Resources");
	return dirStr.Retain();
}
*/

/**********************************************************************/

#if _KJ_MAC_
#include "CApp.h"

int		AssertAlert(const char *msgZ, const char *fileZ, long lineL, bool noThrowB)
{
	bool	incB = MPTaskIsPreemptive(NULL);
	
	if (incB) {
		++gApp->i_callbackL;
		noThrowB = true;
	}

	bool	mainB = gApp->i_callbackL == 0;

	if (incB) {
		--gApp->i_callbackL;
	}

	SuperString		formatStr(SSLocalize("Assert Fail: %s, in file: '%s' at line %ld", "eeeks!"));
	
	formatStr.ssprintf(NULL, msgZ, fileZ, lineL);

	if (mainB) {
		MessageAlert(formatStr.utf8Z());
	} else {
		PostAlert(formatStr.utf8Z());
		
		#ifdef kDEBUG
		static bool	s_showB = true;
		if (s_showB) {
			Debugger();	
		}
		#endif
	}
	
	if (!mainB && !noThrowB) {
		ETX(ERR_Assert_Failed);
	}
	
	return 1;
}
#else
int		AssertAlert(const char *msgZ, const char *fileZ, long lineL, bool noThrowB)
{
	SuperString		formatStr("$$$ Assert Fail: %s, in file: '%s' at line %ld\n");
	
	formatStr.ssprintf(NULL, msgZ, fileZ, lineL);
	CCFLog()(formatStr.ref());
	ReportErr(formatStr.utf8Z(), -1);
	return 1;
}
#endif
