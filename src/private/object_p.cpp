#include "object_p.h"

#include <cassert>

SF_BEGIN_NAMESPACE

ObjectImpl::ObjectImpl(Object *pub)
    : _pub(pub)
{
}

ObjectImpl::~ObjectImpl()
{
    _pub = nullptr;
}

Object *ObjectImpl::object()
{
    return _pub;
}

const Object *ObjectImpl::object() const
{
    return _pub;
}

VirtCallObject *ObjectImpl::vo()
{
    return reinterpret_cast<VirtCallObject *>(_pub);
}

const VirtCallObject *ObjectImpl::vo() const
{
    return reinterpret_cast<const VirtCallObject *>(_pub);
}

void ObjectImpl::installVirtualHandler(const std::unordered_map<const char *, VirtualHandler> &handlers)
{
    for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        this->handlers.insert(std::move(*it));
    }
}

void ObjectImpl::installVirtualHandler(const char *name, VirtualHandler handler)
{
    assert(name && handler);
    this->handlers.insert(std::make_pair(name, handler));
}

void ObjectImpl::virtual_call(const char *name, int argc, ...)
{
    auto it = handlers.find(name);
    if (it == handlers.end()) {
        //! @todo add comment!
        return;
    }

    va_list args;
    va_start(args, argc);
    it->second(args, this);
    va_end(args);
}

SF_END_NAMESPACE
