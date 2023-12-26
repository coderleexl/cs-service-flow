#ifndef _BASE_VCOBJECT_H_
#define _BASE_VCOBJECT_H_

#include "sf_global.h"

SF_BEGIN_NAMESPACE

class VirtCallObject {
public:
    VirtCallObject()
    {
    }
    inline virtual ~VirtCallObject()
    {
    }

    virtual void virtual_call(const char *name, int argc, ...)
    {
    }
};

SF_END_NAMESPACE

#endif // _BASE_VCOBJECT_H_
