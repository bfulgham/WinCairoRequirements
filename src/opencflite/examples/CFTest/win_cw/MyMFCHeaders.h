#ifndef __MYMFCHEADERS_H__
#define __MYMFCHEADERS_H__

/*
	Generic header for any MFC configuration:

	To build without precompiled headers, either use your 
	local stdafx.h or MyMFCHeaders.pch++ as the prefix file.

	This header can be used with debug/release or shared/static runtimes
	as long as the "x86 CodeGen" / "Runtime configuration" option is
	set to a value other than "Custom".  Otherwise, use the appropriate
	MyMFC[DLL][D]Headers.mch for your configuration. 
*/

/*	All configurations are forced to use __cdecl / x86 multithreaded codegen */

#ifndef _MFC_PCH_CPP
	#ifdef _DLL
		#ifdef _DEBUG
			#define _MFC_PCH_CPP	"MyMFCDLLDHeaders.mch"
		#else
			#define _MFC_PCH_CPP	"MyMFCDLLHeaders.mch"
		#endif
	#else
		#ifdef _DEBUG
			#define _MFC_PCH_CPP	"MyMFCDHeaders.mch"
		#else
			#define _MFC_PCH_CPP	"MyMFCHeaders.mch"
		#endif
	#endif
#endif

#define _CFTEST_
#include _MFC_PCH_CPP

#undef _MFC_PCH_CPP

#endif	/*__MYMFCHEADERS_H__*/

