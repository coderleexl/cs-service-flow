#ifndef _THREADED_RUNNABLE_H_
#define _THREADED_RUNNABLE_H_

#include "sf_global.h"

#include "base/task.h"

SF_BEGIN_NAMESPACE

class Runnable : public Task {
public:
    Runnable();
    inline virtual ~Runnable() {};

    virtual void precede(Runnable *task);
    virtual void behind(Runnable *task);

    virtual void sameAs(Runnable *task);

private:
    void *_p;
};

SF_END_NAMESPACE

#endif // _THREADED_RUNNABLE_H_