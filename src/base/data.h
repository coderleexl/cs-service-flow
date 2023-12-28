#ifndef _BASE_DATA_H_
#define _BASE_DATA_H_

#include "sf_global.h"

SF_BEGIN_NAMESPACE

struct SF_LIBRARY_EXPORT AbstractData {
    AbstractData() = default;
    virtual ~AbstractData() = default;
};

SF_END_NAMESPACE

#endif // _BASE_DATA_H_