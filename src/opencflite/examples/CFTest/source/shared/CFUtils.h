#ifndef _H_CFUtils
#define _H_CFUtils

#include <vector>
#include <map>
#include <CoreFoundation/CoreFoundation.h>

#ifndef _CFTEST_
#include "XPlatConditionals.h"
#endif

#define		loop(_n)				for (long _indexS = 0, _maxS = (_n); _indexS < _maxS; _indexS++)
#define		loop2(_n)				for (long _index2S = 0, _max2S = (_n); _index2S < _max2S; _index2S++)
#define		loop_reverse(_n)		for (long _indexS = ((_n) - 1); _indexS >= 0; _indexS--)
#define		loop_range(_begin, _end)	for (long _indexS = _begin, _maxS = (_end); _indexS < _maxS; _indexS++)
#define		loop2_range(_begin, _end)	for (long _index2S = _begin, _max2S = (_end); _index2S < _max2S; _index2S++)
#define		loop_reverse(_n)	for (long _indexS = ((_n) - 1); _indexS >= 0; _indexS--)

#define		CF_ASSERT(_foo)	if (!(_foo)) {					\
	AssertAlert(#_foo, __FILE__, (long)__LINE__, true);	}

int		AssertAlert(const char *msgZ, const char *fileZ, long lineL, bool noThrowB = false);

#define		kJams_LogFileName		"kJams Log file.txt"

#if _KJ_MAC_
	#include "Standard.h"

	#ifdef kDEBUG
		#define		TRACK_RETAINS	1
	#else
		#define		TRACK_RETAINS	0
	#endif

	#if TRACK_RETAINS
		class	ScTrackRetains {
			void	*i_dataP;
			
			public:
			ScTrackRetains();
			~ScTrackRetains();
		};
		
		CFTypeRef	CFRetainDebug(CFTypeRef cf, bool do_itB = true);
		void		CFReleaseDebug(CFTypeRef cf);
	#else
		class	ScTrackRetains {};

		inline CFTypeRef	CFRetainDebug(CFTypeRef cf, bool do_itB = true) {
			return do_itB ? CFRetain(cf) : cf;
		}

		#define	CFReleaseDebug(_x)		CFRelease(_x)
	#endif
#else
	
	void	DebugReport(const char *utf8Z, OSStatus err);

	#define		kKiloByte				1024
	#define		kKiloBytef				1024.0f
	#define		kMegaByte				(kKiloByte * kKiloByte)
	#define		kGigaByte				(kMegaByte * kKiloByte)
	#define		kTeraByte				(kGigaByte * kKiloByte)

	#ifdef __MWERKS__
		typedef struct {
		  short               top;
		  short               left;
		  short               bottom;
		  short               right;
		} Rect;

		struct RGBColor {
		  unsigned short      red;                    /*magnitude of red component*/
		  unsigned short      green;                  /*magnitude of green component*/
		  unsigned short      blue;                   /*magnitude of blue component*/
		};
		typedef struct RGBColor		RGBColor;
		typedef char *				Ptr;
		typedef UInt32				OSType;
		
		#define __MACTYPES__
	#else
		#ifdef _CFTEST_
			#ifdef __WIN32__
				#include <QuickDraw.h>
			#else
				#include <QD/QuickDraw.h>
			#endif
		#else 
			#include "QuickDraw.h"
		#endif
	#endif

	#ifndef _CFTEST_
		#ifndef _CONSTRUCTOR_
			#include "XErrors.h"
		#endif
	#endif

	#ifndef _H_XErrors
		#define ERR_(_ERR, FUNC)	if (!_ERR) { _ERR = (FUNC); }
		#define ERR(FUNC)			ERR_(err, FUNC);
		#define	ERRL(FUNC, _str)	if (!err) { ERR(FUNC); if (err) { 	LogErr("### Error " _str, err); }}
		#define	ETRL(_exp, _str)	{ ERRL(_exp, _str); if (err) { return err;} }

		#define BEX_THROW(ERR)		throw ((long) ERR)	
		#define ETX(EXPR)			{ OSStatus _err = (EXPR); if (_err) BEX_THROW(_err); }
	#endif

	#define	memclr(p, s)	std::memset(p, 0, s)
	#define	structclr(p)	memclr(&p, sizeof(p))

	#ifndef _H_BasicTypes
		typedef std::basic_string<UTF8Char, std::char_traits<UTF8Char>, std::allocator<UTF8Char> > ustring;

		#define uc(_foo) (unsigned char *)(_foo)

		typedef std::vector<char>			CharVec;
		typedef std::vector<UTF8Char>		UCharVec;
		typedef std::vector<UTF16Char>		UTF16Vec;
	#endif

	#define noErr	0

	inline CFTypeRef	CFRetainDebug(CFTypeRef cf, bool do_itB = true) {
		return do_itB ? CFRetain(cf) : cf;
	}

	#define	CFReleaseDebug(_x)		CFRelease(_x)

	#if OPT_MACOS
		#define	COMPILE_FOR_10_4	(MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)	//	1	//	defined(__GNUC__)
	#endif

#endif

#if OPT_MACOS
	#define		kCFURLPlatformPathStyle			kCFURLPOSIXPathStyle
	#define		kCFURLPlatformPathSeparator		"/"
#else
	#define		kCFURLPlatformPathStyle			kCFURLWindowsPathStyle
	#define		kCFURLPlatformPathSeparator		"\\"
#endif


/***************************************************************************************/
typedef std::vector<CFStringRef>	StringRefVec;

static	inline CFRange		CFStrGetRange(CFStringRef ref) {
	return CFRangeMake(0, CFStringGetLength(ref));
}

static	inline CFRange		CFArrayGetRange(CFArrayRef ref) {
	return CFRangeMake(0, CFArrayGetCount(ref));
}

OSErr	CFDictionaryCreate(CFMutableDictionaryRef *dictP);
OSErr	CFArrayCreate(CFMutableArrayRef *arrayP);

OSStatus		CFWriteDataToURL(const CFURLRef urlRef, CFDataRef dataRef);

bool			Read_PList(const CFURLRef &url, CFDictionaryRef *plistP);
OSStatus		Write_PList(
	CFPropertyListRef	plist,
	CFURLRef			urlRef);

CFStringRef		CFStringCreateWithDate(CFDateRef dateRef, bool showTimeB = false);
CFStringRef		CFStringCreateWithNumber(CFNumberRef numRef);
CFDateRef		CFDateCreateWithGregorian(const CFGregorianDate& gregDate);
CFNumberRef		CFNumberCreateWithSInt32(SInt32 valL);

CFStringRef		CFCopyBundleResourcesFSPath();

#ifndef __WIN32__
typedef SInt64                          LongDateTime;
	CFDateRef 		CFDateCreateWithLongDateTime(const LongDateTime &ldt);
	LongDateTime	CFDateToLongDateTime(CFDateRef dateRef);
#endif

/***************************************************************************************/

template <typename T>
class	ScCFReleaser {
	T	i_typeRef;
	
	public:
	ScCFReleaser(T typeRef = NULL, bool retainB = false) : i_typeRef(typeRef)	{
		if (retainB) {
			Retain();
		}
	}
	
	~ScCFReleaser() {
		Release();
	}
	
	CFIndex RetainCount()	{
		CFIndex		countL = 0;
		
		if (i_typeRef) {
			 countL = CFGetRetainCount(i_typeRef);
		}
		
		return countL;
	}
	
	T	Retain()	{	if (i_typeRef) CFRetainDebug(i_typeRef);	return i_typeRef;	}
	T	Release()	{
		CFIndex		countL = RetainCount();
		
		if (i_typeRef) {
			CFReleaseDebug(i_typeRef);
		}

		if (countL == 1) {
			i_typeRef = NULL;
		}
		
		return i_typeRef;
	}
	
	T	Get() const		{	return i_typeRef;	}
	T	Set(T typeRef)	{
		
		if (typeRef) {
			CFRetainDebug(typeRef);
		}
		
		Release();
		
		i_typeRef = typeRef;
		return i_typeRef;
	}
	
	T*	AddressOf()	{	return &i_typeRef;	}
	
	operator CFTypeRef()	{	return i_typeRef;	}
	operator T()			{	return i_typeRef;	}
	operator T*()			{	return AddressOf();	}
	
	T	operator =(T typeRef)	{	return Set(typeRef);	}
	
	T	transfer()	{
		T	ret = i_typeRef;
		
		i_typeRef = NULL;
		return ret;
	}
	
	T	adopt(T typeRef)	{
		Set(NULL);
		i_typeRef = typeRef;
		return i_typeRef;
	}
	
	void	LogCount(const char *nameZ) {	S_LogCount(i_typeRef, nameZ);	}
};

/***************************************************************************************/
class	CDictionaryIterator {
	CFDictionaryRef		i_dict;
	
	static	void 	CB_S_Operator(const void *key, const void *value, void *context) {
		CDictionaryIterator		*thiz = (CDictionaryIterator *)context;
		
		thiz->operator()((CFStringRef)key, value);
	}
	
public:
	CDictionaryIterator(CFDictionaryRef dict) : i_dict(dict) { }
	
	void	for_each() {
		CFDictionaryApplyFunction(i_dict, CB_S_Operator, this);
	}
	
	virtual void	operator()(CFStringRef key, const void *value) {
		
	}
};

template <class Function>
class CDict_ForEach : public CDictionaryIterator {
	Function		i_f;
	
public: CDict_ForEach(CFDictionaryRef dict, Function f) : CDictionaryIterator(dict), i_f(f) { }
	
	void	operator()(CFStringRef keyRef, const void *valRef) {
		i_f(keyRef, valRef);
	}
};

template <class Function>
inline	void	dict_for_each(CFDictionaryRef dict, Function f)
{
	if (dict) {
		CDict_ForEach<Function>		cdict(dict, f);
		
		cdict.for_each();
	}
}

#define		CFArrayApplyFunctionToAll(_array, _cb, _data) \
CFArrayApplyFunction((CFArrayRef)_array, CFArrayGetRange((CFArrayRef)_array), _cb, _data)

class	CArrayIterator {
	CFArrayRef		i_array;
	
	static	void 	CB_S_Operator(const void *value, void *context) {
		CArrayIterator		*thiz = (CArrayIterator *)context;
		
		thiz->operator()(value);
	}
	
public:
	CArrayIterator(CFArrayRef array) : i_array(array) { }
	
	void	for_each() {
		CFArrayApplyFunctionToAll(i_array, CB_S_Operator, this);
	}
	
	virtual void	operator()(const void *value) { }
};

template <class Function>
class CArray_ForEach : public CArrayIterator {
	Function		i_f;
	
public: CArray_ForEach(CFArrayRef dict, Function f) : CArrayIterator(dict), i_f(f) { }
	
	void	operator()(CFTypeRef valRef) {
		i_f(valRef);
	}
};

template <class Function>
inline	void	array_for_each(CFArrayRef array, Function f)
{
	if (array) {
		CArray_ForEach<Function>		carray(array, f);
		
		carray.for_each();
	}
}

#define		DICT_STR_RECT_LEFT		"Left"
#define		DICT_STR_RECT_RIGHT		"Right"
#define		DICT_STR_RECT_TOP		"Top"
#define		DICT_STR_RECT_BOTTOM	"Bottom"

#define		DICT_STR_COLOR_VALUE	"Value"
#define		DICT_STR_COLOR_RED		"Red"
#define		DICT_STR_COLOR_GREEN	"Green"
#define		DICT_STR_COLOR_BLUE		"Blue"

class SuperString;
/*****************************************************
 usage: (same for CCFArray)
 CCFDictionary		dict;							//	allocate a dictionary
 CCFDictionary		dict(existingDict);				//	retain existing dictionary (WILL be released in d'tor, transfers ownership)
 CCFDictionary		dict(existingDict, true);		//	use existing dictionary (will NOT be released, caller responsible for releasing)
 CCFDictionary		dict(NULL, true);				//	NULL dictionary, ONLY use if next instruction is .[Immutable]AddressOf(), use with legacy C APIs
 */
class CCFDictionary : public ScCFReleaser<CFMutableDictionaryRef> {
	typedef	ScCFReleaser<CFMutableDictionaryRef>	_inherited;

	void	validate() {
		if (Get() == NULL) {
			realloc();
		}
	}

	public:
	CCFDictionary(CFDictionaryRef dictRef0 = NULL, bool retainB = false) : 
		_inherited((CFMutableDictionaryRef)dictRef0, retainB) 
	{ if (dictRef0 == NULL && retainB == false) {validate();} }

	CFDictionaryRef*	ImmutableAddressOf()	{	return (CFDictionaryRef *)AddressOf();	}
	operator CFDictionaryRef() const			{	return Get();	}

	void	realloc() {
		CFMutableDictionaryRef	dictRef = CFDictionaryCreateMutable(
			kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);

		if (dictRef == NULL) {
			ETX(1);
		}

		_inherited::adopt(dictRef);
	};
	
	CFMutableDictionaryRef		Copy();

	bool						ContainsKey(CFStringRef keyRef);
	bool						ContainsKey(const char *utf8Z);
	
	bool						empty()	{	return size() == 0;		}
	CFIndex						size() {
		return CFDictionaryGetCount(Get());
	}

	/*********************************************/
	CFTypeID			GetType(CFStringRef key) {
		CFTypeID		typeID(CFNullGetTypeID());
		CFTypeRef		valRef(GetValue(key));

		if (valRef) {
			typeID = CFGetTypeID(valRef);
		}

		return typeID;
	}

	inline CFTypeRef	GetValue(CFStringRef key) {
		CFTypeRef		typeRef = NULL;
		
		if (ContainsKey(key)) {
			typeRef = CFDictionaryGetValue(Get(), key);
		}
		
		return typeRef;
	}

	inline CFTypeRef	GetValue(const char* keyZ);

	CFStringRef			GetAs_String	(const char *utf8Z);
	Rect				GetAs_Rect		(const char *utf8Z);
	bool				GetAs_Bool		(const char *utf8Z);
	SInt32				GetAs_SInt32	(const char *utf8Z);
	UInt32				GetAs_UInt32	(const char *utf8Z)		{	return (UInt32)GetAs_SInt32(utf8Z);	}
	SInt16				GetAs_SInt16	(const char *utf8Z);
	float				GetAs_Float		(const char *utf8Z);
	CFDictionaryRef		GetAs_Dict		(const char *utf8Z);
	CFArrayRef			GetAs_Array		(const char *utf8Z);
	CFDateRef			GetAs_Date		(const char *utf8Z);
	RGBColor			GetAs_Color		(const char *utf8Z);
	Ptr					GetAs_Ptr		(const char *utf8Z)		{	return (Ptr)GetAs_UInt32(utf8Z);	}

	/*********************************************/
	void				RemoveValue		(CFStringRef key);
	void				RemoveValue		(const char *utf8Z);
	
	virtual void		SetRealValue(CFTypeRef key, CFTypeRef val) {
		CFDictionarySetValue(Get(), key, val);
	}

	inline void		SetValue(CFTypeRef key, CFTypeRef val) {
		if (key && val) {
			SetRealValue(key, val);
		}
	}

	void				SetValue(CFStringRef ref, SInt32 value);

	void				SetValue(const char *utf8Z, CFTypeRef val);

	void				SetValue(const char *utf8Z, const Rect& frameR);
	void				SetValue(const char *utf8Z, const char *utf8ValZ);
	void				SetValue(const char *utf8Z, bool value);
	void				SetValue(const char *utf8Z, SInt32 value);
	void				SetValue(const char *utf8Z, UInt32 value)	{	SetValue(utf8Z, (SInt32)value);		}
	void				SetValue(const char *utf8Z, SInt16 value);
	void				SetValue(const char *utf8Z, float value);
	void				SetValue(const char *utf8Z, const SuperString& value);
	void				SetValue(const char *utf8Z, const RGBColor& value);
	void				SetValue(const char *utf8Z, Ptr valueP)	{	SetValue(utf8Z, (UInt32)valueP);	}

	/*********************************************/
	template <class Function>
	inline	void	for_each(Function f) {
		dict_for_each(Get(), f);
	}
};

class CCFArray : public ScCFReleaser<CFMutableArrayRef> {
	typedef	ScCFReleaser<CFMutableArrayRef>	_inherited;

	void	validate() {
		if (Get() == NULL) {
			realloc();
		}
	}

	public:
	CCFArray(CFArrayRef arrayRef0 = NULL, bool retainB = false) : 
		_inherited((CFMutableArrayRef)arrayRef0, retainB)
	{ if (arrayRef0 == NULL && retainB == false) {validate();} }
	
	void	realloc() {
		CFMutableArrayRef	arrayRef = CFArrayCreateMutable(
			kCFAllocatorDefault, 0,
			&kCFTypeArrayCallBacks);

		if (arrayRef == NULL) {
			ETX(1);	//	errCppbad_alloc);
		}

		_inherited::adopt(arrayRef);
	};
	
	bool				empty()	{	return size() == 0;		}
	CFIndex				size() {
		return CFArrayGetCount(Get());
	}

	CFTypeRef			operator[](CFIndex idx) {
		return CFArrayGetValueAtIndex(Get(), idx);
	}

	CFIndex				GetFirstIndexOfValue(CFTypeRef val) {
		return CFArrayGetFirstIndexOfValue(Get(), CFArrayGetRange(Get()), val);
	}
	
	void				push_back(SInt32 val) {
		ScCFReleaser<CFNumberRef>	numberRef(CFNumberCreateWithSInt32(val));

		push_back(numberRef);
	}

	void				push_back(CFTypeRef val) {
		CFArrayAppendValue(Get(), val);
	}

	void				insert(CFIndex idx, CFTypeRef val) {
		CFArrayInsertValueAtIndex(Get(), idx, val);
	}

	SInt32				GetIndValAs_SInt32(CFIndex idx) 
	{
		CFNumberRef			numberRef((CFNumberRef)operator[](idx));
		SInt32				valL = 0;

		if (numberRef) {
			CFNumberGetValue(numberRef, kCFNumberSInt32Type, &valL);
		}

		return valL;
	}
	
	void				set_ind_value(CFIndex idx, CFTypeRef val) {
		CFArraySetValueAtIndex(Get(), idx, val);
	}

	template <class Function>
	inline	void	for_each(Function f) {
		array_for_each(Get(), f);
	}
};
/*****************************************/
class CCFXMLNode {	//	does NOT retain OR release
	CFXMLNodeRef		i_nodeRef;

	public:
	CCFXMLNode(CFXMLNodeRef nodeRef) : i_nodeRef(nodeRef) { }

	CFXMLNodeTypeCode		GetTypeCode()	{	return CFXMLNodeGetTypeCode(i_nodeRef);	}
	CFXMLElementInfo*		GetInfoPtr()	{	return (CFXMLElementInfo *)CFXMLNodeGetInfoPtr(i_nodeRef);	}
	CFStringRef				GetString()		{	return CFXMLNodeGetString(i_nodeRef);	}
	operator				CFXMLNodeRef()	{	return i_nodeRef;	}
};

class CCFXmlTree : public ScCFReleaser<CFXMLTreeRef> {
	typedef	ScCFReleaser<CFXMLTreeRef>	_inherited;

	public:
	CCFXmlTree(CFXMLTreeRef treeRef0 = NULL, bool retainB = false) : _inherited(treeRef0, retainB) { }

	bool	CreateFromData(CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions = kCFXMLParserSkipWhitespace);
	
	CFXMLNodeRef		GetNode()	{	return CFXMLTreeGetNode(Get());	}
	CFIndex				size() {
		return CFTreeGetChildCount(Get());
	}

	CFTypeRef			GetValueAtIndex(CFIndex idx) {
		return CFTreeGetChildAtIndex(Get(), idx);
	}

	CFXMLTreeRef		GetChild(CFStringRef childNameRef) {
		CCFXmlTree	childTree;

		loop (size()) {
			childTree.Set((CFTreeRef)GetValueAtIndex(_indexS));

			CCFXMLNode	childNode(childTree.GetNode());

			if (::CFStringCompare(childNode.GetString(), childNameRef, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
				return childTree.Get();
			}
		}
		return NULL;
	}

	template <class Function>
	inline	void	for_each(Function f, short levelS = 0) {
		loop (size()) {
			CCFXmlTree	xmlTreeNode((CFXMLTreeRef)GetValueAtIndex(_indexS), true);
			CCFXMLNode	node(xmlTreeNode.GetNode());

			f(node, levelS);
			xmlTreeNode.for_each(f, levelS + 1);
		}
	}
};

bool		Read_XML(const CFURLRef url, CCFXmlTree& xml);

/*****************************************************/
//	logging
class CCFLog {
	bool	i_crB;
	
public: 
	CCFLog(bool crB = false) : i_crB(crB) { }
	
	void operator()(CFTypeRef valRef);
	//inline void operator()(void const* valRef)	{	operator()((CFTypeRef)valRef);	}
	void operator()(CFStringRef keyRef, CFTypeRef valRef);
};

#endif