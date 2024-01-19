#include "future.h"
#include "threadpool.h"

#include "private/object_p.h"

#include <atomic>
#include <condition_variable>
#include <cstdarg>

SF_BEGIN_NAMESPACE

class _Future : public ObjectImpl {
    SF_PUBLIC_CLASS(Future)
public:
    _Future::_Future(Future *pp)
        : ObjectImpl(pp)
    {
        installVirtualHandler({
            { "_setState", &_Future::setState },
            { "_setThreadPool", &_Future::setThreadPool },
        });
    }

    void changeState(Future::State state);

    void addState(int state);
    void removeState(int state);

    static void setState(va_list &args, ObjectImpl *impl);
    static void setThreadPool(va_list &args, ObjectImpl *impl);
    static void bindData(va_list &args, ObjectImpl *impl);

    ThreadPool *threadPool = nullptr;
    void *data = nullptr;
    std::mutex mutex;
    std::condition_variable waitCondition;

    std::atomic<int> state;
};

void _Future::setState(va_list &args, ObjectImpl *impl)
{
    _Future *_p = dynamic_cast<_Future *>(impl);
    Future::State state = va_arg(args, Future::State);

    _p->changeState(state);
}

void _Future::setThreadPool(va_list &args, ObjectImpl *impl)
{
    _Future *_p = dynamic_cast<_Future *>(impl);
    ThreadPool *pool = va_arg(args, ThreadPool *);
    _p->threadPool = pool;
}

void _Future::bindData(va_list &args, ObjectImpl *impl)
{
    _Future *_p = dynamic_cast<_Future *>(impl);
    void *data = va_arg(args, void *);
    _p->data = data;
}

void _Future::changeState(Future::State state)
{
    if (state & Future::Started || state & Future::Running) {
        removeState(Future::NoState | Future::Finished);
        waitCondition.notify_one();
    } else if (state & Future::Finished) {
        removeState(Future::NoState | Future::Started | Future::Running);
        waitCondition.notify_one();
    }

    addState(state);
    if (this->threadPool && this->data)
        this->threadPool->_runnableStateChanged(this->p(), this->data);
}

void _Future::addState(int s)
{
    state.fetch_or(s);
}

void _Future::removeState(int s)
{
    state.fetch_add(s);
}

Future::Future()
    : Object(new _Future(this))
{
}

void Future::cancel()
{
    _Future *p = this->p();
    if (!p->threadPool) {
        //! @todo add comment
        return;
    }

    if (p->state.load() & Future::Canceled)
        return;

    p->removeState(Future::Paused);
    p->addState(Future::Canceled);

    p->threadPool->_runnableStateChanged(this, p->data);
}

void Future::setPaused(bool paused)
{
    _Future *p = this->p();
    if (!p->threadPool) {
        //! @todo add comment
        return;
    }

    if (paused)
        p->addState(Future::Paused);
    else
        p->removeState(Future::Paused);

    p->threadPool->_runnableStateChanged(this, p->data);
}

void Future::waitForStarted()
{
    _Future *p = this->p();
    if (!p->threadPool) {
        //! @todo add comment
        return;
    }

    std::unique_lock<std::mutex> locker(p->mutex);
    while (!(p->state & Future::Started)) {
        p->waitCondition.wait(locker);
    }
}

void Future::waitForFinished()
{
    _Future *p = this->p();
    if (!p->threadPool) {
        //! @todo add comment
        return;
    }

    std::unique_lock<std::mutex> locker(p->mutex);
    while (!(p->state & Future::Finished)) {
        p->waitCondition.wait(locker);
    }
}

Future::State Future::state() const
{
    const _Future *p = this->p();
    return Future::State(p->state.load());
}

SF_END_NAMESPACE
