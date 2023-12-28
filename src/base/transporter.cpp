#include "transporter.h"

#include "private/object_p.h"

SF_BEGIN_NAMESPACE

Transporter::Transporter()
    : Object(nullptr)
{
}

Transporter::~Transporter()
{
}

Transporter::Transporter(ObjectImpl *impl)
    : Object(impl)
{
}

SF_END_NAMESPACE
