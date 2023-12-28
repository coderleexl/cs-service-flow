#include "runnable.h"

#include "private/object_p.h"

SF_BEGIN_NAMESPACE

Runnable::Runnable()
    : Task(nullptr)
{
}

void Runnable::precede(Runnable *task)
{
}

void Runnable::behind(Runnable *task)
{
}

void Runnable::sameAs(Runnable *task)
{
}

Runnable::Runnable(ObjectImpl *impl)
    : Task(impl)
{
}

SF_END_NAMESPACE
