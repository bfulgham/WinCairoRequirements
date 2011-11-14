#ifndef _H_SuperString
#define _H_SuperString

#include <string>
#include <cctype>
#include "string.h"
#include <stdarg.h>
#include "CFUtils.h"

#define	kCodePage_WindowsLatin1				1252
#define	kCFStringEncodingPercentEscapes		(0xfffffffeU)
	
#ifdef __WIN32__	
	typedef std::vector<wchar_t>	WCharVec;
#include "Files.h"
#else
	#include <Carbon/Carbon.h>
#endif

#if !_YAAF_
	#ifndef _CFTEST_
		#ifndef _CONSTRUCTOR_
			#include "StringUtils.h"
		#endif
	#endif
#endif

/****************************************************************/
void		mt_vsnprintf(char *destZ, size_t sizeL, const char *formatZ, va_list &args);
void		LogErr(const char *strZ, OSStatus err, bool crB = true, bool unixB = false);
void		ReportErr(const SuperString& errTypeStr, OSStatus err = 0, bool unixB = false);

/****************************************************************/

char *			CopyFloatToC(float valF, char *bufZ, short precisionS = 2);
float			CStringToFloat(const char *numF);
char*			strrstr(const char* stringZ, const char* findZ);
const char *	CopyLongToC(long valL);
OSType			GetExtension(const char *strZ);

CFStringRef			CFStrCreateWithCurAbsTime();
bool				CFStringContains(CFStringRef inRef, CFStringRef findRef, bool case_sensitiveB = false);
CFComparisonResult	CFStringCompare(CFStringRef str1, CFStringRef str2, bool case_sensitiveB = false);
bool				CFStringEqual(CFStringRef str1, CFStringRef str2, bool case_sensitiveB = false);
bool				CFStringLess(CFStringRef lhs, CFStringRef rhs, bool case_sensitiveB = false);
bool				CFStringIsEmpty(CFStringRef nameRef);

void				SetDefaultEncoding(CFStringEncoding encoding);
bool				IsDefaultEncodingSet();

ustring			&CopyCFStringToUString(
									   CFStringRef			str, 
									   ustring				&result, 
									   CFStringEncoding	encoding	= kCFStringEncodingUTF8, 
									   bool				externalB	= false);

CFStringRef		CFStringCreateWithC(
									const char *		bufZ, 
									CFStringEncoding	encoding = kCFStringEncodingInvalidId);

CFStringRef		CFStringCreateWithCu(
									 const UTF8Char *	bufZ, 
									 CFStringEncoding	encoding = kCFStringEncodingUTF8);

std::string		&CopyCFStringToStd(
								   CFStringRef			str, 
								   std::string			&stdstr, 
								   CFStringEncoding	encoding = kCFStringEncodingInvalidId);

void			CFStrReplaceWith(CFMutableStringRef stringRef, CFStringRef replaceStr, CFStringRef withStr);

extern	bool	g_pref_diacritic_insensitive_searchB;


/****************************************************************/
typedef struct {
	const char	*replaceZ;
	const char	*withZ;
} SuperStringReplaceRec;

class	UniString {
	UTF16Char	i_nullChar;
	UTF16Vec	*i_charVecP;
	
	void		UpdateNamePointer() {
		if (i_charVecP) {
			i_charVecP->push_back(0);
			i_nameP = &(*i_charVecP)[0];
		} else {
			i_nameP = &i_nullChar;
		}
	}
	
public:
	void	Initialize(CFStringRef cf_name, bool forceBigEndianB = false) {
		if (cf_name && !CFStringIsEmpty(cf_name)) {
			ustring		utf16str;
			
			CopyCFStringToUString(cf_name, utf16str, forceBigEndianB ? kCFStringEncodingUTF16BE : kCFStringEncodingUTF16);
			
			//	divide by 2
			i_lengthL	= utf16str.size() >> 1;
			
			if (i_charVecP == NULL) {
				i_charVecP = new UTF16Vec();
			}
			
			UTF16Char	*utf16A = (UTF16Char *)utf16str.c_str();
			
			i_charVecP->assign(&utf16A[0], &utf16A[i_lengthL]);
			//			CFStringGetCharacters(cf_name, CFRangeMake(0, i_lengthL), &(*i_charVecP)[0]);
		} else {
			delete i_charVecP;
			i_charVecP = NULL;
			i_lengthL = 0;
		}
		
		UpdateNamePointer();
	}

	#if defined(__MWERKS__)
		typedef long	UniCharCount;
	#endif

	UniCharCount		i_lengthL;
	UniChar				*i_nameP;
	
	UniString(const UniString &uni)	: i_nameP(NULL), i_charVecP(NULL), i_nullChar(0) {
		i_lengthL	= uni.i_lengthL;
		
		if (i_lengthL) {
			i_charVecP = new UTF16Vec();
			i_charVecP->resize(i_lengthL);
			std::copy(&uni.i_nameP[0], &uni.i_nameP[i_lengthL], &(*i_charVecP)[0]);
		}
		
		UpdateNamePointer();
	}
	
	UniString(CFStringRef cf_name, bool forceBigEndianB = false) : i_nameP(NULL), i_charVecP(NULL), i_nullChar(0) {
		Initialize(cf_name, forceBigEndianB);
	}
	
	UniString(const char *nameZ = NULL) : i_nameP(NULL), i_charVecP(NULL), i_nullChar(0), i_lengthL(0) {
		if (nameZ) {
			ScCFReleaser<CFStringRef>	cf_name(CFStringCreateWithC(nameZ));
			
			Initialize(cf_name);
		}
	}
	
	~UniString() {
		delete i_charVecP;
	}
};

#if defined(__WIN32__)
	#ifndef __MACTYPES__
		typedef RECT	Rect;
		typedef UInt32	OSType;
	#endif
#endif

std::string		ULong_To_Hex(UInt32 valueL);
UInt32			Hex_To_ULong(const char *hexZ);
OSType			CharToOSType(const char *bufZ);

typedef std::vector<UTF32Char>		UTF32CharVec;
typedef std::basic_string<UTF32Char, std::char_traits<UTF32Char>, std::allocator<UTF32Char> > ww_string;

class SuperString {
	CFStringRef				i_ref;
	mutable std::string		*i_std;
	mutable UniString		*i_uni;
	mutable ustring			*i_utf8;
	mutable ustring			*i_pstr;
	mutable ww_string		*i_utf32;
	
public:
	const	std::string	&std() const	{	Update_std();	return *i_std;	}
	const	CFStringRef	&ref() const	{	return i_ref;	}
	const	UniString	&uni(bool forceBigEndianB = false) const	{	Update_uni(forceBigEndianB);	return *i_uni;	}
	const	ustring		&utf8() const	{	Update_utf8();	return *i_utf8;	}
	const	char *		c_str() const  	{	return std().c_str();	}
	const	char *		utf8Z() const	{	return (const char *)utf8().c_str();	}
	ConstStr255Param	p_str() const	{	Update_pstr();	return i_pstr->c_str();	}
	const	ww_string&	utf32() const	{	Update_utf32();	return *i_utf32;		}

	#ifdef __WIN32__
		LPCWSTR		w_str() const			{	return (LPCWSTR)uni().i_nameP;		}
		size_t		w_strlen() const		{	return w_size();	}
	#endif
	
	operator const UniString&() const	{	return uni();	}
	operator const std::string&() const	{	return std();	}
	//operator const ustring&() const		{	return utf8();	}	causes all sorts of ambiguities
	operator CFStringRef() const		{	return ref();	}
	operator const UInt8*() const		{	return utf8().c_str();	}

	/************************************/
	SuperString&	Truncate(bool activeB, const Rect& frameR);
	SuperString&	ssprintf(const char *formatZ0, ...);
	SuperString&	format(const char *formatZ0, ...);
	
private:
	void	SetNULL()
	{
		i_ref	= NULL;
		i_std	= NULL;
		i_uni	= NULL;
		i_utf8	= NULL;
		i_utf32 = NULL;
		i_pstr	= NULL;
	}
	
public:
	#ifdef __WIN32__
		SuperString(const wchar_t *wcharZ);

		SuperString(const WCharVec& wcharVec) {
			SetNULL();
			Set(*(UTF16Vec *)&wcharVec);
		}
	#endif

	SuperString(const HFSUniStr255 &str) {
		SetNULL();
		
		ScCFReleaser<CFStringRef>	myRef = CFStringCreateWithCharacters(
			kCFAllocatorDefault, 
			str.unicode,
			str.length);
		
		Set(myRef);
	}
	
	SuperString(const char *strZ = NULL, CFStringEncoding encoding = kCFStringEncodingInvalidId) {
		SetNULL();		
		Set(uc(strZ), encoding);
	}
	
	SuperString(const SuperString &sstr) {
		SetNULL();
		Set(sstr.ref());
	}
	
	#ifdef _KJAMS_
	#ifndef _KJAMSX_
		void	Set(const CDHMSF msf, kTimeCodeType type = kTimeCode_NORMAL) {
			char	bufAC[256];
			
			Set(CopyMSFToC(msf, bufAC, type));
		}

		SuperString(const CDHMSF msf, kTimeCodeType type = kTimeCode_NORMAL) {
			SetNULL();
			Set(msf, type);
		}
	#endif
	#endif

	/*
	 FAIL!
	SuperString(const UInt16* strZ) {
		SetNULL();
		
		ScCFReleaser<CFStringRef>	myRef = strZ 
			? CFStringCreateWithC((const char *)strZ, kCFStringEncodingUnicode) : 
			(CFStringRef)CFRetainDebug(CFSTR(""));

		Set(myRef);
	}
	*/
	
	SuperString(const UInt8 *strZ) {
		SetNULL();
		Set(strZ);
	}
	
	SuperString(long valL) {
		SetNULL();
		append(valL);
	}
	
	/*
	 dam!  this causes ambiguation
	 SuperString(double valF) {
	 SetNULL();
	 append((float)valF);
	 }
	 */
	
	SuperString(const ustring &str) {
		SetNULL();
		Set(str.c_str());
	}
	
	SuperString(const std::string &str) {
		SetNULL();
		Set(str.c_str());
	}
	
	SuperString(CFStringRef myRef, bool retainB = true) {
		SetNULL();
		Set(myRef, retainB);
	}
	
	void	Set_CFType(CFTypeRef cfType);
	
	/************************************/
	void	Set_p(ConstStr255Param strZ, CFStringEncoding encoding = kCFStringEncodingInvalidId);
	
	void	Set(const char *strZ) {
		ScCFReleaser<CFStringRef>	myRef = CFStringCreateWithC(strZ);
		
		Set(myRef);
	}
	
	void	Set(const UInt8 *strZ, CFStringEncoding encoding = kCFStringEncodingUTF8) {
		ScCFReleaser<CFStringRef>	myRef;
		
		if (strZ) {
			if (encoding == kCFStringEncodingPercentEscapes) {
				Set(strZ, kCFStringEncodingASCII);
				
				UnEscape();
				return;
			} else {
				myRef.Set(CFStringCreateWithCu(strZ, encoding));
			}
		} else {
			myRef.Set((CFStringRef)CFRetainDebug(CFSTR("")));
		}
		
		Set(myRef);
	}
	
	void	Escape() {
		Set(CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, ref(), NULL, NULL, kCFStringEncodingUTF8), false);
	}

	void	UnEscape() {
		Set(CFURLCreateStringByReplacingPercentEscapes(kCFAllocatorDefault, ref(), CFSTR("")));
	}
	
	void	Set(const ustring& utf8, CFStringEncoding encoding = kCFStringEncodingUTF8) {
		Set(utf8.c_str(), encoding);
	}
	
	void	Set(const UTF16Vec &vec) {
		ScCFReleaser<CFStringRef>	myRef = CFStringCreateWithCharacters(kCFAllocatorDefault, &vec[0], vec.size());
		
		Set(myRef);
	}
	
	void	assign(const UTF8Char *beginZ, const UTF8Char *endZ, CFStringEncoding encoding = kCFStringEncodingUTF8) {
		ustring		str(beginZ, endZ);
		
		Set(str, encoding);
	}
	
	void	Set(const SuperString &sstr) {
		if (&sstr != this) {
			Set(sstr.i_ref);
		}
	}
	
  	CFStringRef	Retain() const {	CFRetainDebug(i_ref);	return i_ref;	}
	
	void	Set(CFStringRef myRef, bool retainB = true);
	
	/************************************/
	~SuperString() {
		Set((CFStringRef)NULL, false);
	}
	
	void	Release() {
		CFReleaseDebug(i_ref);
	}
	
	/************************************/
	void	Update_pstr() const {
		
		if (!i_pstr) {
			i_pstr = new ustring;
			i_pstr->push_back(0);
			Update_std();
			i_pstr->insert(i_pstr->end(), i_std->begin(), i_std->end());
			(*i_pstr)[0] = i_pstr->size() - 1;
			
			delete i_std;
			i_std	= NULL;
		}
	}
	
	void	Update_uni(bool forceBigEndianB) const {
		if (!i_uni) {
			i_uni = new UniString(i_ref, forceBigEndianB);
		}
	}
	
	void	Update_std() const {
		if (!i_std) {
			i_std = new std::string;
			CopyCFStringToStd(i_ref, *i_std);
		}
	}
	
	void	Update_utf32() const;
	void	Update_utf8() const {
		if (!i_utf8) {
			i_utf8 = new ustring;
			CopyCFStringToUString(i_ref, *i_utf8);
		}
	}

	/************************************/
	void	clear()	{	Set((CFStringRef)NULL);	}
	
	bool				Contains(const SuperString& other) const {
		return strstr(utf8Z(), other.utf8Z()) != NULL;
	}
	
	//	returns number of utf8 bytes (not characters) that match the start of the other string
	size_t				MatchStart(const SuperString& other, char delimiterCh = 0);

	bool				StartsWith(const SuperString& other) {
		return MatchStart(other) == other.size();
	}
	
	SuperString&		ReplaceTable(SuperStringReplaceRec *recA, long sizeL);
	SuperString&		Replace(const SuperString& replaceStr, const SuperString& withStr) {
		ScCFReleaser<CFMutableStringRef>	newRef(CFStringCreateMutableCopy(NULL, 0, i_ref));
		
		CFStrReplaceWith(newRef, replaceStr.ref(), withStr.ref());
		Set(newRef);
		return *this;
	} 
	
	SuperString		&UnderScoresToSpaces() {
		return Replace("_", " ");
	}
	
	//	returns a new string, does not modify original
	SuperString			md5() const;
	
	SuperString&		Scramble();
	SuperString&		UnScramble();
	
	SuperString&		trim();

	#if _KJ_MAC_
	void			MakeRecoverable(bool old_styleB = false) {
		ScCFReleaser<CFStringRef>		sc(CFStrCreateRecoverableName(i_ref, false));
		
		Set(sc);
	}
	
	SuperString&	Recover() {
		ScCFReleaser<CFStringRef>		sc(CFStrRecoverName(i_ref));
		
		Set(sc);
		return *this;
	}

	SuperString&	RecoverCaps(bool titleCaseB = true) {
		Recover();
		InterCaps(false, titleCaseB);
		MakeRecoverable();
		return *this;
	}
	
	#endif
	
	CFDataRef		CopyDataRef() {
		return CFStringCreateExternalRepresentation(
			kCFAllocatorDefault, i_ref, kCFStringEncodingUTF8, 0);
	}
	
	void			Set(CFDataRef dataRef) {
		ScCFReleaser<CFStringRef>	myRef = CFStringCreateFromExternalRepresentation(
			kCFAllocatorDefault, dataRef, kCFStringEncodingUTF8);

		Set(myRef);
	}
	
	size_t			size() const	{	return utf8().size();		}	//	utf8 chars
	size_t			w_size() const	{	return uni().i_lengthL;		}	//	utf16 chars
	size_t			ww_size() const	{	return utf32().size();		}	//	utf32 chars
	
	/*
	 ambiguous
	 
	 char			operator[](size_t indexS) {
	 char	ch = 0;
	 
	 if (utf8().size() > indexS) {
	 ch = utf8().c_str()[indexS];
	 }
	 
	 return ch;
	 }
	 */
	
	UTF32Char		GetIndChar_ww(size_t indexL = 0) const {
		UTF32Char		ch = 0;
		
		if (indexL >= 0 && indexL < utf32().size()) {
			ch = utf32()[indexL];
		}
		
		return ch;
	}
	
	UTF8Char		GetIndChar(size_t indexL = 0) const {
		UTF8Char		ch = 0;
		
		if (indexL >= 0 && indexL < utf8().size()) {
			ch = utf8()[indexL];
		}
		
		return ch;
	}
	
	UTF8Char		GetIndCharR(long indexL = 0) const {
		return GetIndChar(utf8().size() - (indexL + 1));
	}
	
	SuperString&	ToUpper();
	SuperString&	ToLower();
	bool			IsAllCaps();
	SuperString&	Normalize();
	SuperString&	InterCaps(bool allow_line_breaksB = false, bool titleCaseB = true);
	SuperString&	TheToEnd();
	
	SuperString&	operator =(const SuperString &other) {	Set(other);	return *this;	}
	SuperString&	operator =(const char *otherZ) {	Set(otherZ);	return *this;	}
	
	bool			operator<(CFStringRef other) const {
		return CFStringLess(i_ref, other);
	}
	
	bool			operator>(CFStringRef other) const {
		return CFStringLess(other, i_ref);
	}
	
	bool			operator ==(CFStringRef other) {
		return CFStringEqual(i_ref, other);
	}
	
	bool			operator ==(CFStringRef other) const {
		return CFStringEqual(i_ref, other);
	}
	
	bool			operator !=(CFStringRef other)	{
		return ! operator==(other);
	}
	
	bool			operator ==(const SuperString &other) {
		return operator==(other.i_ref);
	}
	
	bool			operator ==(const SuperString &other) const {
		return operator==(other.i_ref);
	}
	
	bool			operator !=(const SuperString &other)	{
		return ! operator==(other.i_ref);
	}

	bool			operator !=(SuperString &other)	{
		return ! operator==(other.i_ref);
	}
	
	bool	IsNumeric() const;
	bool	empty() const		{	return CFStringIsEmpty(i_ref);	}

/*
	CFNumberRef		CopyAs_NumberRef()'
	CFDateRef		CopyAs_DateRef()'
	CFURLRef		CopyAs_URLRef()'

	UInt32			GetAs_UInt32();
	UInt16			GetAs_UInt16();
	SInt16			GetAs_SInt16();
*/

	OSType	GetAs_OSType() const	{	return CharToOSType(utf8Z());	}
	SInt32	GetAs_SInt32() const {	return ::atoi(c_str());	}
	float	GetAs_Float() const	{	return CStringToFloat(c_str());	}

	OSType	GetAsOSType() const	{	return GetAs_OSType();	}
	long	value_long() const	{	return GetAs_SInt32();	}
	float	value_float() const	{	return GetAs_Float();	}

	UInt32	hex_to_ulong()		{	return Hex_To_ULong(c_str());	}
	
	//	returns *this
	SuperString&	ww_pop_front(long num_utf32_charsL = 1);

	//	returns the chars popped off the front
	SuperString		pop_front(size_t numL = 1)	{
		SuperString			str;
		
		if (utf8().size() <= numL) {
			str.Set(*this);
			clear();
		} else {
			UCharVec	bufAC(numL);
			
			std::copy(i_utf8->begin(), i_utf8->begin() + numL, &bufAC[0]);
			bufAC.push_back(0);
			
			str.Set(&bufAC[0]);
			Set(&(utf8().c_str())[numL]);
		}
		
		return str;
	}
	
	short	count_match(const char *matchZ) {
		short		countS = 0;
		const char	*chZ = (const char *)utf8().c_str();
		
		do {
			chZ = strstr(chZ, matchZ);
			if (chZ) {
				++countS;
				++chZ;
			}
		} while (chZ);
		
		return countS;
	}
	
	bool	rSplit(const char *splitZ, SuperString *lhsP0 = NULL, bool from_endB = false);	
	bool	Split(const char *splitZ, SuperString *rhsP0 = NULL, bool from_endB = false);
	
	SuperString	&pop_back(UInt32 numCharsL = 1);
	
	OSType	&pop_ext(OSType *extP) const;
	
	OSType	get_ext() const {
		OSType		ext;
		
		pop_ext(&ext);
		return ext;
	}
	
	SuperString	&pop_ext(SuperString *extP0 = NULL) {
		ustring				ustr(utf8());
		const unsigned char	*dotP;
		
		dotP = uc(strrchr(utf8Z(), '.'));
		
		if (dotP) {
			if (extP0) {
				extP0->Set(dotP);
			}
			
			ustr.resize(ustr.size() - strlen((char *)dotP));
			
			Set(ustr);
		}
		
		return *this;
	}
	
	/******************************/
	SuperString		&resize(long num_utf8_charsL) {
		ustring		ustr(utf8());
		
		ustr.resize(num_utf8_charsL);
		Set(ustr);
		return *this;
	}

	SuperString		&ww_resize(long num_utf32_charsL);

	SuperString		&append(const ustring &other) {
		Set(utf8() + other);
		return *this;
	}
	
	SuperString		&append(SuperString &other) {
		return append(other.utf8());
	}
	
	SuperString		&append(CFStringRef myRef) {
		return append(SuperString(myRef).utf8());
	}
	
	SuperString		&append(const char *other) {
		return append(SuperString(other).utf8());
	}
	
	SuperString		&append(long valueL)
	{
		char	bufAC[32];
		
		::sprintf(bufAC, "%ld", valueL);
		return append(bufAC);
		//		std::string		strStr;
		
		//		return append(NumToCString(valueL, strStr));
	}
	
	SuperString		&UnEscapeAmpersands();

	SuperString		&append(char ch)
	{
		char	bufAC[2];
		
		bufAC[0] = ch;
		bufAC[1] = 0;
		
		SuperString		appendStr(bufAC, kCFStringEncodingASCII);
		
		append(appendStr);
		return *this;
	}
	
	SuperString		&append(float valueF, short precS = 1)
	{
		char	bufAC[32];
		
		CopyFloatToC(valueF, bufAC, precS);
		return append(bufAC);
	}
	
	/****************************/
	SuperString		&prepend(const ustring &other) {
		Set(other + utf8());
		return *this;
	}
	
	SuperString		&prepend(const SuperString &other) {
		return prepend(other.utf8());
	}
	
	SuperString		&prepend(const char *other) {
		return prepend(SuperString(other).utf8());
	}
	
	SuperString		&prepend(long valueL) {
		char	bufAC[32];
		
		::sprintf(bufAC, "%.2ld", valueL);
		return prepend(bufAC);
	}
	
	SuperString		&Ascii();
	SuperString		&Localize();
};

SuperString		operator+(const SuperString &lhs, SuperString rhs);

char			*OSTypeToChar(OSType osType, char *bufZ);
SuperString		OSTypeToString(OSType osType);
void			IncrementNumberAtEndOfString(SuperString *strP);
SuperString		FormatBytes(UInt64 bytes);

#if OPT_WINOS
inline SuperString		SuperString_CreateFrom_UniChar(
	UniCharCount     sizeL,
	const UniChar *  nameA)
{
	CF_ASSERT(nameA[sizeL] == 0);
	return SuperString(reinterpret_cast<const wchar_t *>(nameA));
}
#endif

#if _KJ_MAC_
	void			SP_VerifyStringExists(SP_IndexType stringType, SuperString *strP);
	bool			Dict_Get_Str(CFDictionaryRef dict, const char *keyZ, SuperString *strP);
	SuperString		Array_GetIndStr(CFArrayRef array, long indexL);
#endif

typedef std::vector<SuperString>	SStringVec;

#endif
