#ifndef _BASE_OBJECT_P_H_
#define _BASE_OBJECT_P_H_

#include "base/object.h"

SF_BEGIN_NAMESPACE

class ObjectImpl : public VirtCallObject {
public:
    ObjectImpl(Object *pub)
        : _pub(pub)
    {
    }
    ~ObjectImpl()
    {
        _pub = nullptr;
    }

    inline Object *object()
    {
        return _pub;
    }
    inline const Object *object() const
    {
        return _pub;
    }

    inline VirtCallObject *vo()
    {
        return reinterpret_cast<VirtCallObject *>(_pub);
    }
    inline const VirtCallObject *vo() const
    {
        return reinterpret_cast<const VirtCallObject *>(_pub);
    }

protected:
    Object *_pub = nullptr;
};

SF_END_NAMESPACE

#endif // _BASE_OBJECT_P_H_
