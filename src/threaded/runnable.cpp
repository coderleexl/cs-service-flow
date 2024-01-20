#include "runnable.h"

#include "private/object_p.h"

#include <vector>

SF_BEGIN_NAMESPACE

class _Runnable : public ObjectImpl {
    SF_PUBLIC_CLASS(Runnable)
public:
    _Runnable::_Runnable(Runnable *pp)
        : ObjectImpl(pp)
    {
    }

    std::vector<Runnable *> prevRunnables;
    std::vector<Runnable *> nextRunnables;
};

Runnable::Runnable()
    : Runnable(new _Runnable(this))
{
}

void Runnable::precede(Task *task)
{
    Runnable *runnable = dynamic_cast<Runnable *>(task);
    if (!runnable) {
        //! @todo add comment!
        return;
    }

    runnable->p()->prevRunnables.push_back(this);
    this->p()->nextRunnables.push_back(runnable);
}

void Runnable::behind(Task *task)
{
    Runnable *runnable = dynamic_cast<Runnable *>(task);
    if (!runnable) {
        //! @todo add comment!
        return;
    }

    runnable->p()->nextRunnables.push_back(this);
    this->p()->prevRunnables.push_back(runnable);
}

Runnable *Runnable::next() const
{
    if (this->p()->nextRunnables.empty())
        return nullptr;

    auto a = this->p();
    return this->p()->nextRunnables.front();
}

Runnable::Runnable(ObjectImpl *impl)
    : Task(impl)
{
}

SF_END_NAMESPACE
