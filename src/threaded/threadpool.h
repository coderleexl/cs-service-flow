#ifndef _THREADED_THREADPOOL_CPP_
#define _THREADED_THREADPOOL_CPP_

#include "sf_global.h"

#include "base/object.h"

SF_BEGIN_NAMESPACE

class Future;
class Runnable;
class _ThreadPool;

class SF_LIBRARY_EXPORT ThreadPool : public Object {
public:
    ThreadPool();
    ~ThreadPool();

    void setMaxThreads(int number);
    int maxThreads() const;

    static int platformIdealThreadCount();
    int activeThreadCount() const;

    std::shared_ptr<Future> start(const std::shared_ptr<Runnable> &runnable);
    void waitForFinished();

private:
    SF_FRIEND_CLASS(Future)
    SF_PRIVATE_CLASS(ThreadPool)
    SF_PRIVATE_FUNCTION(_runnableStateChanged, Future *, void *)
};

SF_END_NAMESPACE

#endif // _THREADED_THREADPOOL_CPP_