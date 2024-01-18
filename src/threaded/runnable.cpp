#include "runnable.h"

#include "private/object_p.h"

SF_BEGIN_NAMESPACE

Runnable::Runnable()
    : Task(nullptr)
{
}

Runnable::~Runnable()
{
}

void Runnable::precede(Task *task)
{
}

void Runnable::behind(Task *task)
{
}

Runnable *Runnable::next() const
{
    return nullptr;
}

Runnable::Runnable(ObjectImpl *impl)
    : Task(impl)
{
}

SF_END_NAMESPACE
