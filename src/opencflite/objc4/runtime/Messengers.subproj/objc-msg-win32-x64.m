#include "objc-private.h"

void *_objc_forward_handler = NULL;
void *_objc_forward_stret_handler = NULL;

Method _cache_getMethod(Class cls, SEL sel, IMP objc_msgForward_imp)
{
    return (Method)0;
}

IMP _cache_getImp(Class cls, SEL sel)
{
	return (IMP)0;
}

OBJC_EXPORT id objc_msgSend(id a, SEL b, ...)
{
    return (id)0;
}

OBJC_EXPORT double objc_msgSend_fpret(id a, SEL b, ...)
{
    return 0;
}

OBJC_EXPORT id objc_msgSendSuper(struct objc_super *a, SEL b, ...)
{
    return (id)0;
}

OBJC_EXPORT void objc_msgSend_stret(void)
{
    return;
}

OBJC_EXPORT id objc_msgSendSuper_stret(struct objc_super *a, SEL b, ...)
{
    return (id)0;
}

OBJC_EXPORT id _objc_msgForward(id a, SEL b, ...)
{
    return (id)0;
}

OBJC_EXPORT id _objc_msgForward_stret(id a, SEL b, ...)
{
    return (id)0;
}

id _objc_msgForward_internal(id a, SEL b, ...)
{
    return (id)0;
}

OBJC_EXPORT id method_invoke(void)
{
    return (id)0;
}

OBJC_EXPORT void method_invoke_stret(void)
{
    return;
}

id _objc_ignored_method(id obj, SEL sel)
{
    return (id)0;
}
