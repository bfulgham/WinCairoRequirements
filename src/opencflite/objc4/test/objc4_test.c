#include <objc/objc.h>
#include <objc/runtime.h>

#include <stdio.h>


int main()
{
	Class myClass = objc_getClass("Object");

	id nilClass = class_createInstance(Nil, 0);
	id testOne = class_createInstance (myClass, 0);

    /*
	testassert(obj->isa == [Fake class]);
    testassert(object_setClass(obj, [Super class]) == [Fake class]);
    testassert(obj->isa == [Super class]);
    testassert(object_setClass(nil, [Super class]) == nil);

    testassert(object_getClass(obj) == buf[0]);
    testassert(object_getClass([Super class]) == [Super class]->isa);
    testassert(object_getClass(nil) == Nil);

    testassert(0 == strcmp(object_getClassName(obj), "Super"));
    testassert(0 == strcmp(object_getClassName([Super class]), "Super"));
    testassert(0 == strcmp(object_getClassName(nil), "nil"));
    
    testassert(0 == strcmp(class_getName([Super class]), "Super"));
    testassert(0 == strcmp(class_getName([Super class]->isa), "Super"));
    testassert(0 == strcmp(class_getName(nil), "nil"));

    succeed(__FILE__);
	*/

	fprintf (stderr, "Done\n");
}
