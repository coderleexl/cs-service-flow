#include "object.h"

#include "private/object_p.h"

SF_BEGIN_NAMESPACE

Object::Object(ObjectImpl *impl)
    : _impl(impl)
{
}

Object::~Object()
{
}

SF_END_NAMESPACE
