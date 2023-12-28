#ifndef _BASE_DATA_H_
#define _BASE_DATA_H_

#include "sf_global.h"

SF_BEGIN_NAMESPACE

struct AbstractData {
    inline virtual ~AbstractData()
    {
    }
};

SF_END_NAMESPACE

#endif // _BASE_DATA_H_