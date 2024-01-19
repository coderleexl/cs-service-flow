#ifndef _BASE_OBJECT_P_H_
#define _BASE_OBJECT_P_H_

#include "base/object.h"

#include <cstdarg>
#include <unordered_map>

SF_BEGIN_NAMESPACE

class ObjectImpl : public VirtCallObject {
public:
    ObjectImpl(Object *pub);
    ~ObjectImpl();

    Object *object();
    const Object *object() const;

    VirtCallObject *vo();
    const VirtCallObject *vo() const;

    typedef void (*VirtualHandler)(va_list &, ObjectImpl *);
    void installVirtualHandler(const std::unordered_map<const char *, VirtualHandler> &handlers);
    void installVirtualHandler(const char *name, VirtualHandler handler);

    virtual void virtual_call(const char *name, int argc, ...) override;

protected:
    Object *_pub = nullptr;
    std::unordered_map<const char *, VirtualHandler> handlers;
};

SF_END_NAMESPACE

#endif // _BASE_OBJECT_P_H_
