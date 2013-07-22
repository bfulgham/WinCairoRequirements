
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFRuntime.h>
#include <malloc.h>
#include <string.h>
#if DEPLOYMENT_TARGET_LINUX
#include <alloca.h>
#endif

typedef void* SEL;
typedef void* id;

SEL (*__CFGetObjCSelector)(const char *)=0;
void* (*__CFGetObjCClass)(const char *)=0;
extern void* (*__CFSendObjCMsg)(const void*, SEL, ...);

static void __attribute__((constructor)) InitializeFoundation() {
   static void* hFoundation=0;
#if defined(__WIN32__)
   if(!hFoundation) {
      hFoundation=LoadLibrary("Foundation.1.0.dll");

      __CFGetObjCSelector=GetProcAddress(hFoundation, "sel_registerName");
      __CFSendObjCMsg=GetProcAddress(hFoundation, "objc_msgSend");
      __CFGetObjCClass=GetProcAddress(hFoundation, "objc_getClass");
   }
#endif
}

void* __CFISAForTypeID(CFTypeID typeid) {
   if(!__CFGetObjCClass)
      return 0;

   const CFRuntimeClass *cls = _CFRuntimeGetClassWithTypeID(typeid);
   char *name=alloca(strlen(cls->className)+3);

   // build e.g. "NSCFArray" from CFArray
   strcpy(name, "NS");
   strcat(name, cls->className);
   
   void* ret = __CFGetObjCClass(name);
   
   if(!ret)
      ret=__CFGetObjCClass("NSCFType");
   return ret;
}
