#ifndef _THREADED_FUTURE_H_
#define _THREADED_FUTURE_H_

#include "base/object.h"
#include "sf_global.h"

SF_BEGIN_NAMESPACE

class ThreadPool;
class _Future;
class SF_LIBRARY_EXPORT Future : public Object {
public:
    enum State {
        NoState = 0x00,
        Started = 0x01,
        Running = 0x02,
        Canceled = 0x04,
        Paused = 0x08,
        Finished = 0x10,
    };

    Future();
    inline ~Future() {};

    void cancel();
    void setPaused(bool paused);

    void waitForStarted();
    void waitForFinished();

    State state() const;

private:
    void setThreadPool(ThreadPool *pool);
    void bindData(void *data);

    SF_FRIEND_CLASS(ThreadPool)
    SF_PRIVATE_CLASS(Future)
    SF_PRIVATE_FUNCTION(_setState, Future::State)
};

SF_END_NAMESPACE

#endif // _THREADED_FUTURE_H_