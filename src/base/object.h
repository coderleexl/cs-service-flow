#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include "sf_global.h"
#include "vcobject.h"

#include <memory>

SF_BEGIN_NAMESPACE

class ObjectImpl;
class SF_LIBRARY_EXPORT Object : public VirtCallObject, public std::enable_shared_from_this<Object> {
public:
    Object() = default;

    inline ObjectImpl *impl()
    {
        return _impl.get();
    }
    inline const ObjectImpl *impl() const
    {
        return _impl.get();
    }

    inline VirtCallObject *vo()
    {
        return reinterpret_cast<VirtCallObject *>(_impl.get());
    }
    inline const VirtCallObject *vo() const
    {
        return reinterpret_cast<const VirtCallObject *>(_impl.get());
    }

protected:
    Object(ObjectImpl *impl)
        : _impl(impl)
    {
    }

    std::unique_ptr<ObjectImpl> _impl;
};

SF_END_NAMESPACE

#endif // _BASE_OBJECT_H_