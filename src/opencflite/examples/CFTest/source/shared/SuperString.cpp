/*
 *  SuperString.cpp
 *  CFTest
 *
 *  Created by David M. Cotter on 6/25/08.
 *  Copyright 2008 David M. Cotter. All rights reserved.
 *
 */
#include "stdafx.h"
#include "CFUtils.h"

#if _KJ_MAC_
	#include "CApp.h"
	#include "StringUtils.h"
	#include "MessageAlert.h"
#else

	#include "SuperString.h"

	#ifdef _CFTEST_
		#include <algorithm>
	#else
		#include "CLocalize.h"
		#include <XErrorDialogs.h>
	#endif
#endif

#ifndef __WIN32__

	#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
		enum {
			kCFCompareDiacriticInsensitive = 128, /* If specified, ignores diacritics (o-umlaut == o) */
			kCFCompareWidthInsensitive = 256, /* If specified, ignores width differences ('a' == UFF41) */
			kCFCompareForcedOrdering = 512 /* If specified, comparisons are forced to return either kCFCompareLessThan or kCFCompareGreaterThan if the strings are equivalent but not strictly equal, for stability when sorting (e.g. "aaa" > "AAA" with kCFCompareCaseInsensitive specified) */
		};
	#endif

#endif

#if _KJ_MAC_
	ushort		GetSystemVers(void);
	#define		diacritic_insensitiveB		(gApp && gApp->i_prefs && gApp->i_prefs->i_pref.diacritic_insensitive_searchB)
#else
	bool		g_pref_diacritic_insensitive_searchB	= true;
	#define		diacritic_insensitiveB					g_pref_diacritic_insensitive_searchB

	#ifdef _KJAMSX_
		unsigned short		GetSystemVers(void);
	#else
		static unsigned short		GetSystemVers(void) {
			return 0x1050;	//	fake it to be OS 10.5
		}
	#endif
#endif

#if _KJ_MAC_
void			DebugReport(const SuperString& errTypeStr, OSStatus err)
{
	#ifdef kDEBUG
		ReportErr(errTypeStr, err);
	#else
		LogErr(errTypeStr.utf8Z(), err);
	#endif
}

void		LogErr(const char *utf8Z, OSStatus err, bool crB, bool unixB)
{
	if (err && gApp->Logging()) {
		if (err != eofErr && err != userCanceledErr) {
			int	i = 1;
		}
		
		SuperString		str;
		
		if (unixB) {
			str.Set(UnixErrStr(uc(utf8Z), err));
		} else {
			str.Set(ErrStr(uc(utf8Z), err));
		}
		
		Log(str.utf8Z(), crB);
	}
}

void			ReportErr(const SuperString& errTypeStr, OSStatus err, bool unixB)
{
	SuperString			errStr;
	SuperString			str;
	
	if (unixB) {
		errStr.Set(SSLocalize("Unix Error:", "bummer"));
		str.Set(UnixErrStr(errTypeStr, err));
	} else {
		errStr.Set(SSLocalize("Error", "bummer"));
		str.Set(ErrStr(errTypeStr, err));
	}
	
	MessageAlert(errStr.utf8Z(), str.utf8Z());
}

#else
void		LogErr(const char *utf8Z, OSStatus err, bool crB, bool unixB)
{
	if (err) {	//	&& gApp->Logging()) {
		SuperString		keyStr(uc(utf8Z));
		SuperString		valStr((long)err);
		CCFLog			logger(crB);
		
		logger(keyStr.ref(), valStr.ref());
	}
}

void	DebugReport(const char *utf8Z, OSStatus err)
{
	#ifdef rDEBUG
		AlertID(utf8Z, err);
	#else
		LogErr(utf8Z, err);
	#endif
}

void			ReportErr(const SuperString& errTypeStr, OSStatus err, bool unixB)
{
	#ifdef _CFTEST_
		LogErr(errTypeStr.utf8Z(), err);
	#else
		AlertID(errTypeStr.utf8Z(), err);
	#endif
}

#endif

CFStringRef			CFStrCreateWithCurAbsTime()
{
	CFAbsoluteTime		curTime(CFAbsoluteTimeGetCurrent());
	UInt64				milliSeconds((UInt64)(curTime * 1000));	//	milliseconds
	CFDictionaryRef		dictRef(NULL);

	return CFStringCreateWithFormat(kCFAllocatorDefault, dictRef, CFSTR("%qu"), milliSeconds);
}

char*	strrstr(const char* stringZ, const char* findZ)
{
	bool		firstB = true, doneB = false;
	const char	*nextZ;
	
	do {
		if (firstB) {
			nextZ = strstr(stringZ, findZ);
		} else {
			nextZ = strstr(&stringZ[1], findZ);
		}
		
		doneB = nextZ == NULL;
		
		if (!doneB) {
			stringZ = nextZ;
		} else if (firstB) {
			stringZ = NULL;
		}
		
		firstB = false;		
	} while (!doneB);
	
	return const_cast<char *>(stringZ);
}

const char *		CopyLongToC(long valL)
{
	static	char	s_bufAC[256];
	
	sprintf(s_bufAC, "%d", valL);
	return s_bufAC;
}

float			CStringToFloat(const char *numF)
{
	float	valF = 0;
	
	sscanf(numF, "%f", &valF);
	return valF;
}

SuperString		FormatBytes(UInt64 bytes)
{
	char	buf[1024];
	double	valF	= (double)bytes;
	char	*labelZ	= "K";
	
	if (bytes < kKiloByte) {
		labelZ	= "b";
	} else {
		valF /= kKiloBytef;
		
		if (valF < kKiloByte) {
			labelZ	= "K";
		} else {
			valF /= kKiloBytef;

			if (valF < kKiloByte) {
				labelZ	= "MB";
			} else {
				valF /= kKiloBytef;

				if (valF < kKiloByte) {
					labelZ	= "GB";
				} else {
					valF /= kKiloBytef;

					labelZ	= "TB";
				}
			}
		}
	}
	
	sprintf(buf, "%.1f %s", valF, labelZ);
	return buf;
}

/********************************************************/
bool		CFStringIsEmpty(CFStringRef nameRef)
{
	return nameRef == NULL || CFStringGetLength(nameRef) == 0;
}

CFStringEncoding	s_file_encoding = kCFStringEncodingInvalidId;

bool				IsDefaultEncodingSet()
{
	return s_file_encoding != kCFStringEncodingInvalidId;
}

void				SetDefaultEncoding(CFStringEncoding encoding)
{
	s_file_encoding = encoding;
}

static CFStringEncoding	ValidateEncoding(CFStringEncoding encoding = kCFStringEncodingInvalidId)
{
	if (encoding == kCFStringEncodingInvalidId) {
		encoding = s_file_encoding;
		
		if (encoding == kCFStringEncodingInvalidId) {
			s_file_encoding = kCFStringEncodingASCII;
			CCFLog()(CFSTR("$$$ Default encoding not set!\n"));
		}
	}
	
	return encoding;
}

#ifndef _KJAMSX_
	class Asciify {
		public: void operator()(char &ch) {
			if (ch < 32 || ch > 126) ch = '?';
		}
	};
#endif

CFStringRef		CFStringCreateWithC(
	const char *		bufZ, 
	CFStringEncoding	encoding)
{
	if (bufZ) {
		encoding = ValidateEncoding(encoding);
		
		CFStringRef		cf = NULL;
		
		cf = CFStringCreateWithCString(kCFAllocatorDefault, bufZ, encoding);
		if (!cf) cf = CFStringCreateWithCString(kCFAllocatorDefault, bufZ, kCFStringEncodingWindowsLatin1);
		if (!cf) cf = CFStringCreateWithCString(kCFAllocatorDefault, bufZ, kCFStringEncodingISOLatin1);
		if (!cf) cf = CFStringCreateWithCString(kCFAllocatorDefault, bufZ, kCFStringEncodingMacRoman);
		
		if (!cf) {
			#if _KJ_MAC_
				CharVec			newBuf;
				
				newBuf.assign(&bufZ[0], &bufZ[strlen(bufZ)]);
				std::for_each(newBuf.begin(), newBuf.end(), Asciify());
				bufZ = &newBuf[0];

				static	CMutex_long		s_incdec;

				if (s_incdec.inc() == 1) {
					ScCFReleaser<CFStringRef>	encRef(CFStringGetNameOfEncoding(encoding));
					SuperString					formatStr("Illegal String %s with encoding: [%s], the encoding is: %s\n");
					
					formatStr.ssprintf(
						NULL, 
						bufZ, 
						SuperString(encRef.Get()).utf8Z(), 
						CFStringIsEncodingAvailable(encoding) ? "AVAILABLE" : "NOT available");
					
					MessageAlert(formatStr.utf8Z());
				}
				
				s_incdec.dec();
			#else
				CF_ASSERT(cf);
			#endif
			
			return NULL;
		}
		
		return (CFStringRef)CFRetainDebug(cf, false);
	} else {
		return (CFStringRef)CFRetainDebug(CFSTR(""));
	}
}

CFStringRef		CFStringCreateWithCu(
	const UTF8Char *	bufZ, 
	CFStringEncoding	encoding)
{
	return CFStringCreateWithC((const char *)bufZ, encoding);
}

ustring		&CopyCFStringToUString(CFStringRef str, ustring &result, CFStringEncoding encoding, bool externalB)
{
	result.clear();
	
	if (str) {
		#define						kBufSize		256
		UTF8Char					utf8Buf[kBufSize];
		CFRange						cfRange = CFStrGetRange(str);
		CFIndex						resultSize;
		CFIndex						numChars;
		
		encoding = ValidateEncoding(encoding);
		
		while (cfRange.length > 0) {
			
			numChars = CFStringGetBytes(
				str, cfRange, encoding, '?', externalB, 
				&utf8Buf[0], kBufSize, &resultSize);
			
			if (numChars == 0) break;   // Failed to convert anything...
			
			result.append(&utf8Buf[0], &utf8Buf[resultSize]);
			
			cfRange.location	+= numChars;
			cfRange.length		-= numChars;
		}
	}
	
	return result;
}

std::string		&CopyCFStringToStd(
	CFStringRef			str, 
	std::string			&stdstr, 
	CFStringEncoding	encoding)
{
	stdstr.clear();
	
	encoding = ValidateEncoding(encoding);
	
	if (str) {
		const char	*charZ = CFStringGetCStringPtr(str, encoding);
		
		if (charZ) {
			stdstr = charZ;
		} else {
			ustring		ustr;
			
			CopyCFStringToUString(str, ustr, encoding);
			stdstr.assign(ustr.begin(), ustr.end());
		}
	}
	
	return stdstr;
}

std::string		ULong_To_Hex(UInt32 valueL)
{
	char			bufAC[16];
	
	sprintf(bufAC, "%.8lx", (int)valueL);
	return bufAC;
}

UInt32			Hex_To_ULong(const char *hexZ)
{
	UInt32	val;
	
	sscanf(hexZ, "%lx", &val);
	return val;
}

char		*OSTypeToChar(OSType osType, char *bufZ)
{
	osType = CFSwapInt32HostToBig(osType);
	*((OSType *)bufZ) = osType;
	bufZ[4] = 0;
	return bufZ;
}

SuperString		OSTypeToString(OSType osType)
{
	SuperString		str;
	
	if (osType == (OSType)-1) {
		str.Set("-");
	} else {
		char	bufAC[5];
		
		str.Set(OSTypeToChar(osType, bufAC));
	}
	
	return str;
}

OSType		CharToOSType(const char *bufZ)
{
	OSType		osType;
	short		lenL = strlen(bufZ);
	
	osType = *((OSType *)(&bufZ[lenL - 4]));
	osType = CFSwapInt32BigToHost(osType);
	return osType;
}

class SS_ForEach_FindNonDigit {
	public: bool operator()(UTF8Char uch) {
		return !isdigit((char)uch);
	}
};

bool	SuperString::IsNumeric() const
{
	bool		numericB = false;
	
	if (!empty()) {
		ustring&	ustr(const_cast<ustring&>(utf8()));
		
		numericB = std::find_if(ustr.begin(), ustr.end(), SS_ForEach_FindNonDigit()) == utf8().end();
	}
	
	return numericB;
}

class SS_ForEach_Ascii {
	public: void operator()(char &ch) {
		if (ch < 32) ch = '?';
	}
};

SuperString		&SuperString::Ascii()
{
	CharVec		charVec;
	
	charVec.assign(std().begin(), std().end());
	
	std::for_each(charVec.begin(), charVec.end(), SS_ForEach_Ascii());
	charVec.push_back(0);
	Set(&charVec[0]);
	return *this;
}

SuperString&	SuperString::format(const char *formatZ0, ...)
{
	CharVec		buf(2048);
	va_list		args;
	
	CF_ASSERT(utf8().size() < 1024);
	va_start(args, formatZ0);
	
	ScCFReleaser<CFStringRef>	str((CFStringRef)CFRetainDebug(CFStringCreateWithFormatAndArguments(
		kCFAllocatorDefault, 
		NULL, 
		formatZ0 ? SuperString(uc(formatZ0)).ref() : ref(), 
		args), false));
	
	va_end(args);

	Set(str.Get());
	return *this;
}

void	SuperString::Set(CFStringRef myRef, bool retainB)
{
	if (i_ref) {
		CFReleaseDebug(i_ref);
	}
	
	if (myRef == NULL) {
		myRef = CFSTR("");
	}
	
	i_ref = myRef;
	
	if (retainB) {
		CFRetainDebug(i_ref);
	}
	
	delete i_uni;
	i_uni	= NULL;
	
	delete i_std;
	i_std	= NULL;
	
	delete i_utf8;
	i_utf8	= NULL;

	delete i_utf32;
	i_utf32	= NULL;
	
	delete i_pstr;
	i_pstr = NULL;

	#if kDEBUG
		c_str();
	#endif
}

SuperString&		SuperString::Normalize()
{
	ScCFReleaser<CFMutableStringRef>	str1;

	str1.Set(CFStringCreateMutableCopy(kCFAllocatorDefault, 0, ref()));
	
	CFStringNormalize(str1, kCFStringNormalizationFormD);
	Set(str1);
	
	#ifndef __WIN32__
		if (GetSystemVers() >= 0x1040) {
			if (CFStringTransform(str1, NULL, kCFStringTransformStripCombiningMarks, false)) {
				Set(str1);
			}
		}

		if (GetSystemVers() >= 0x1050) {
		//	CFStringFold(str1, CFOptionFlags theFlags, CFLocaleRef theLocale)
		}
	#endif

	return *this;
}
	
void		CFStrReplaceWith(CFMutableStringRef stringRef, CFStringRef replaceStr, CFStringRef withStr)
{
	ScCFReleaser<CFArrayRef>	arrayRef;
	
	arrayRef.Set(CFStringCreateArrayWithFindResults(
		NULL, stringRef, replaceStr, CFStrGetRange(stringRef), kCFCompareCaseInsensitive));
	
	if (arrayRef.Get()) {
		CFRange			*rangeRef;
		
		loop_reverse (CFArrayGetCount(arrayRef)) {
			rangeRef = (CFRange *)CFArrayGetValueAtIndex(arrayRef, _indexS);
			CFStringReplace(stringRef, *rangeRef, withStr);
		}
	}
}

static CFOptionFlags		GetFlags_NormalizeStrings(
	SuperString&	str1, 
	SuperString&	str2, 
	CFOptionFlags	optionFlags = 0
) {
	optionFlags |= (CFOptionFlags)(0
		| kCFCompareNonliteral
		| kCFCompareLocalized);

	if (diacritic_insensitiveB) {
		static	bool	s_diacritic_compare_inittedB = false;
		static	bool	s_has_diacritic_insensitive_compareB;
		
		if (!s_diacritic_compare_inittedB) {
		
			#ifdef __WIN32__
				SuperString		str_e("e");
				UInt32			e_grave(CFSwapInt32HostToBig(0xC3A90000));
				SuperString		str_e_grave((UInt8*)&e_grave);
				
				s_has_diacritic_insensitive_compareB = ::CFStringCompare(
					str_e.ref(), str_e_grave.ref(), (CFOptionFlags)kCFCompareDiacriticInsensitive)
					== kCFCompareEqualTo;
			#else
				s_has_diacritic_insensitive_compareB = GetSystemVers() >= 0x1050;
			#endif
			
			s_diacritic_compare_inittedB = true;
		}
		
		if (s_has_diacritic_insensitive_compareB) {
			optionFlags |= (CFOptionFlags)(0
				| kCFCompareDiacriticInsensitive
				| kCFCompareWidthInsensitive);
		} else {
			str1.Normalize();
			str2.Normalize();
		}
	}
	
	return optionFlags;
}

bool					CFStringContains(CFStringRef inRef, CFStringRef findRef, bool case_sensitiveB)
{
	if (inRef == NULL || findRef == NULL) {
		return false;
	}
	
	SuperString		str1(inRef), str2(findRef);
	CFOptionFlags	optionFlags = case_sensitiveB ? 0 : kCFCompareCaseInsensitive;
	
	optionFlags = GetFlags_NormalizeStrings(str1, str2, optionFlags);
	
	return !!CFStringFindWithOptions(str1.ref(), str2.ref(), CFStrGetRange(str1.ref()), optionFlags, NULL);
}

CFComparisonResult		CFStringCompare(CFStringRef ref1, CFStringRef ref2, bool case_sensitiveB)
{
	CFComparisonResult		compareResult = kCFCompareEqualTo;
	
	if ((ref1 == NULL) || (ref2 == NULL)) {
		
		if ((ref1 == NULL) ^ (ref2 == NULL)) {
			if (ref1) {
				compareResult = kCFCompareLessThan;
			} else {
				compareResult = kCFCompareGreaterThan;
			}
		}
	} else {
		SuperString		str1(ref1), str2(ref2);
		CFOptionFlags	optionFlags = case_sensitiveB ? 0 : kCFCompareCaseInsensitive;
		
		optionFlags = GetFlags_NormalizeStrings(str1, str2, optionFlags);
		compareResult = ::CFStringCompare(str1.ref(), str2.ref(), optionFlags);
	}
	
	return compareResult;
}

bool		CFStringEqual(CFStringRef str1, CFStringRef str2, bool case_sensitiveB)
{
	return CFStringCompare(str1, str2, case_sensitiveB) == kCFCompareEqualTo;
}

bool		CFStringLess(CFStringRef lhs, CFStringRef rhs, bool case_sensitiveB)
{
	bool	lessB = CFStringCompare(lhs, rhs, case_sensitiveB) == kCFCompareLessThan;
	
	return lessB;
}

SuperString		operator+(const SuperString &lhs, SuperString rhs)
{
	SuperString		str(lhs);
	
	str.append(rhs);
	return str;
}

class	CLowerfier {
	public: void operator()(char& ch) {
		ch = tolower(ch);
	}
};

void	OSType_ToLower(OSType *type)
{
	char	*testZ = (char *)type;
	
	std::for_each(&testZ[0], &testZ[4], CLowerfier());
}

SuperString&		SuperString::ToLower()
{
	ScCFReleaser<CFMutableStringRef>	capsRef(CFStringCreateMutableCopy(kCFAllocatorDefault, 0, i_ref));
	
	CFStringLowercase(capsRef, CFLocaleGetSystem());
	Set(capsRef);
	return *this;
}

bool			SuperString::IsAllCaps()
{
	ScCFReleaser<CFMutableStringRef>	capsRef(CFStringCreateMutableCopy(kCFAllocatorDefault, 0, i_ref));
	
	CFStringUppercase(capsRef, CFLocaleGetSystem());
	return CFStringEqual(i_ref, capsRef, true);
}

OSType		GetExtension(const char *strZ)
{
	OSType	ext;
	short	lenS = strlen(strZ);

	if (lenS >= 4) {
		ext = CharToOSType(&strZ[lenS - 4]);
	} else {
		CharVec		str;	str.reserve(4 + lenS + 2);
		
		loop (4) {
			str.push_back(' ');
		}
		
		loop (lenS) {
			str.push_back(strZ[_indexS]);
		}

		str.push_back(0);

		ext = CharToOSType(&str[str.size() - 4]);
	}

	OSType_ToLower(&ext);
	return ext;
}

#if _KJ_MAC_
void		mt_vsnprintf(char *destZ, size_t sizeL, const char *formatZ, va_list &args)
{
	static	CMutex	*S_vsprintMutexP = NULL;
	
	if (S_vsprintMutexP == NULL) {
		S_vsprintMutexP = new CMutex;

		if (S_vsprintMutexP->GetErr()) {
			fprintf(stdout, "error creating vsprintf mutex");
		}
	}
	
	{
		CCritical	sc(S_vsprintMutexP);

		vsnprintf(destZ, sizeL - 1, formatZ, args);
		destZ[sizeL - 1] = 0;
	}
}
#else
void		mt_vsnprintf(char *destZ, size_t sizeL, const char *formatZ, va_list &args)
{
	vsnprintf(destZ, sizeL - 1, formatZ, args);
	destZ[sizeL - 1] = 0;
}
#endif


void	SuperString::Update_utf32() const
{
	if (!i_utf32) {
		i_utf32 = new ww_string;
		
		ustring			str;
		
		CopyCFStringToUString(i_ref, str, kCFStringEncodingUTF32LE);

		size_t			sizeL(str.size() >> 2);
		UTF32Char		*utf32A = (UTF32Char *)str.c_str();
		
		i_utf32->assign(&utf32A[0], &utf32A[sizeL]);
	}
}

SuperString&	SuperString::ww_pop_front(long num_utf32_charsL)
{
	ustring			ustr;
	long			sizeL(utf32().size());
	
	if (sizeL <= num_utf32_charsL) {
		clear();
	} else {
		UTF32CharVec	vec(sizeL - num_utf32_charsL);
		
		vec.assign(i_utf32->begin() + num_utf32_charsL, i_utf32->end());
		
		ScCFReleaser<CFStringRef>	myRef(CFStringCreateWithBytes(
			kCFAllocatorDefault, (UInt8 *)&vec[0], vec.size() << 2, kCFStringEncodingUTF32LE, false));

		Set(myRef);
	}

	return *this;
}

SuperString&	SuperString::ww_resize(long num_utf32_charsL)
{
	UTF32CharVec	vec(num_utf32_charsL);
	
	vec.assign(i_utf32->begin(), i_utf32->begin() + num_utf32_charsL);
	
	ScCFReleaser<CFStringRef>	myRef(CFStringCreateWithBytes(
		kCFAllocatorDefault, (UInt8 *)&vec[0], vec.size() << 2, kCFStringEncodingUTF32LE, false));

	Set(myRef);
	return *this;
}

SuperString&	SuperString::ReplaceTable(SuperStringReplaceRec *recA, long sizeL)
{
	ScCFReleaser<CFMutableStringRef>	capsRef(CFStringCreateMutableCopy(kCFAllocatorDefault, 0, i_ref));
	
	loop (sizeL) {
		CFStrReplaceWith(capsRef, SuperString(recA[_indexS].replaceZ).ref(), SuperString(recA[_indexS].withZ).ref());
	}
	
	Set(capsRef);
	return *this;
}

SuperStringReplaceRec		s_ampReplacementsA[] = {
	{	"&lt;",		"<"		},
	{	"&gt;",		">"		},
	{	"&quot;",	"\""	},
	{	"&amp;",	"&"		},
	{	"&apos;",	"'"		},
};
#define		kAmpReplacementSize		(sizeof(s_ampReplacementsA) / sizeof(SuperStringReplaceRec))

SuperString		&SuperString::UnEscapeAmpersands()
{
	ReplaceTable(s_ampReplacementsA, kAmpReplacementSize);
	return *this;
}

bool	SuperString::Split(const char *splitZ, SuperString *rhsP0, bool from_endB)
{
	bool			splitB;
	
	if (rhsP0) {
		rhsP0->clear();
	}
	
	if (from_endB) {
		splitB = strrstr(utf8Z(), splitZ) != NULL;
	} else {
		splitB = strstr(utf8Z(), splitZ) != NULL;
	}
	
	if (splitB) {
		UCharVec	bufAC;
		
		bufAC.assign(utf8().begin(), utf8().end());
		bufAC.push_back(0);
		
		UTF8Char	*chZ;
		
		if (from_endB) {
			chZ	= (UTF8Char *)strrstr((char *)&bufAC[0], (char *)splitZ);
		} else {
			chZ	= (UTF8Char *)strstr((char *)&bufAC[0], (char *)splitZ);
		}
		
		SuperString		temp(chZ + strlen(splitZ));
		*chZ = 0;
		Set(&bufAC[0]);
		
		if (rhsP0) {
			rhsP0->Set(temp);
		}
	}
	
	return splitB;
}

bool	SuperString::rSplit(const char *splitZ, SuperString *lhsP0, bool from_endB)
{
	SuperString		rhs;
	bool			splitB = Split(splitZ, &rhs, from_endB);
	
	if (lhsP0) {
		lhsP0->clear();
	}

	if (splitB) {
		if (lhsP0) {
			lhsP0->Set(*this);
		}
		
		Set(rhs);
	}
	
	return splitB;
}


SuperString&	SuperString::ssprintf(const char *formatZ0, ...)
{
	CharVec			buf(64 * kKiloByte);
	va_list			args;
	SuperString		formatStr(formatZ0);
	
	CF_ASSERT(utf8().size() < 1024);
	va_start(args, formatZ0);
	mt_vsnprintf(&buf[0], buf.size(), formatZ0 ? formatStr.utf8Z() : utf8Z(), args);
	va_end(args);
	
	Set(uc(&buf[0]));
	return *this;
}

SuperString	&		SuperString::pop_back(UInt32 numCharsL)
{
	ustring		ustr(utf8());
	
	if (!ustr.empty()) {
		ustr.resize(ustr.size() - numCharsL);
		Set(ustr.c_str());
	}
	
	return *this;
}

#ifdef __WIN32__

	#if defined(__MWERKS__)
		class CIsZero {
			public: bool operator()(const wchar_t& ch) {
				return ch == 0;
			}
		};

		static size_t	wcslen(const wchar_t *wcharZ) {
			const wchar_t*		it(std::find_if(&wcharZ[0], &wcharZ[512], CIsZero()));
			
			return std::distance(&wcharZ[0], it);
		}
	#endif

	SuperString::SuperString(const wchar_t *wcharZ)
	{
		UTF16Vec	vec((UTF16Char *)&wcharZ[0], (UTF16Char *)&wcharZ[wcslen(wcharZ)]);

		SetNULL();
		Set(vec);
	}
#endif

void	SuperString::Set_p(ConstStr255Param strZ, CFStringEncoding encoding)
{
	UCharVec		charVec;
	
	charVec.assign(&strZ[1], &strZ[strZ[0] + 1]);
	charVec.push_back(0);
	Set(&charVec[0], encoding);
}

OSType&		SuperString::pop_ext(OSType *extP) const {
	*extP = GetExtension(c_str());
	return *extP;
}

void	SuperString::Set_CFType(CFTypeRef cfType)
{
	CFTypeID						cfTypeID(CFGetTypeID(cfType));
	
	if (cfTypeID == CFStringGetTypeID()) {
		Set((CFStringRef)cfType);
	} else {
		ScCFReleaser<CFStringRef>		descIDRef(CFCopyTypeIDDescription(cfTypeID));
		ScCFReleaser<CFStringRef>		descRef(CFCopyDescription(cfType));
		SuperString						descIDStr(descIDRef.Get()), descStr(descRef.Get());
		SuperString						logStr("Type: <%s>, Value: <%s>");
		
		logStr.ssprintf(NULL, descIDStr.utf8Z(), descStr.utf8Z());
		Set(logStr);
	}
}

SuperString&		SuperString::ToUpper()
{
	ScCFReleaser<CFMutableStringRef>	capsRef(CFStringCreateMutableCopy(kCFAllocatorDefault, 0, i_ref));
	
	CFStringUppercase(capsRef, CFLocaleGetSystem());
	Set(capsRef);
	return *this;
}

char *			CopyFloatToC(float valF, char *bufZ, short precisionS)
{
	char	formatAC[32];
	
	sprintf(formatAC, "%%.%df", precisionS);
	sprintf(bufZ, formatAC, valF);
	return bufZ;
}

bool			IsSpecialString(SuperString in_key, bool **writtenB = NULL);
SuperString		&SuperString::Localize()
{
	#ifndef _CFTEST_
	bool						menuB = memcmp("Menu", c_str(), 4) == 0;
	bool						dlgB = memcmp("dlg", c_str(), 3) == 0;
	CFStringRef					stringRef = ref();
	SuperString					key;
	const char *				dictZ;
	ScCFReleaser<CFStringRef>	str;
	bool						specialB = false;

	if (dlgB) {
		specialB = IsSpecialString(*this);
		
		if (specialB) {
			Split("/", &key, true);
			key.prepend("dlg/");
			stringRef = key.ref();
		}

		dictZ = kLocalized_Dialogs;
	} else if (menuB) {
		dictZ = kLocalized_Menus;
	} else {
		dictZ = kLocalized_Strings;
	}
		
	*str.AddressOf() = CFStrLocalize(dictZ, stringRef);

	#ifndef kDEBUG
		if (dlgB) {
			if (specialB) {
				if (key == str) {
					Set(key);
					Split("/", &key, true);
					str.Set(key.ref());
				}
			} else if (*this == str) {
				Split("/", &key, true);
				str.Set(key.ref());
			}
		}
	#endif

	Set(str);
	#endif

	return *this;
}

void		IncrementNumberAtEndOfString(SuperString *strP)
{
	ustring		ustr(strP->utf8());
	size_t		startOfNumS = ustr.size();
	
	while (isdigit(ustr[startOfNumS - 1])) {
		startOfNumS--;
	}

	if (startOfNumS == ustr.size()) {
		ustr.append(uc(" 1"));
	} else {
		int		numberI;
		char	numAC[32];
		
		sscanf((char *)&ustr[startOfNumS], "%d", &numberI);
		ustr.resize(startOfNumS);

		if (ustr[ustr.size() - 1] != ' ') {
			ustr.append(uc(" "));
		}

		sprintf(numAC, "%d", ++numberI);
		ustr.append(uc(numAC));
	}
	
	strP->Set(ustr);
}
