#ifndef _THREADED_RUNNABLE_H_
#define _THREADED_RUNNABLE_H_

#include "sf_global.h"

#include "base/task.h"

SF_BEGIN_NAMESPACE

class SF_LIBRARY_EXPORT Runnable : public Task {
public:
    Runnable();
    virtual ~Runnable() = default;

    virtual void precede(Task *task) override;
    virtual void behind(Task *task) override;
    virtual Runnable *next() const override;

protected:
    Runnable(ObjectImpl *impl);

private:
    SF_PRIVATE_CLASS(Runnable)
};

SF_END_NAMESPACE

#endif // _THREADED_RUNNABLE_H_