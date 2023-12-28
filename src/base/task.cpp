#include "task.h"

#include "private/object_p.h"

SF_BEGIN_NAMESPACE

Task::Task()
    : Object(nullptr)
{
}

void Task::precede(Task *task)
{
}

void Task::behind(Task *task)
{
}

Task::Task(ObjectImpl *impl)
    : Object(impl)
{
}

SF_END_NAMESPACE
