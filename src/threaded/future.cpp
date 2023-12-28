#include "future.h"
#include "threadpool.h"

#include "private/object_p.h"

#include <atomic>
#include <cstdarg>

SF_BEGIN_NAMESPACE

class _Future : public ObjectImpl {
    SF_PUBLIC_CLASS(Future)
public:
    _Future::_Future(Future *pp)
        : ObjectImpl(pp)
    {
    }

    void virtual_call(const char *name, int argc, ...) final;

    void changeState(Future::State state);

    void addState(int state);
    void removeState(int state);

    ThreadPool *threadPool = nullptr;
    void *data = nullptr;
    std::atomic<int> state;
};

void _Future::virtual_call(const char *name, int argc, ...)
{
    if (!strcmp(name, "_setState")) {
        // _setState
        va_list args;
        va_start(args, argc);
        Future::State state = va_arg(args, Future::State);
        va_end(args);

        changeState(state);
    }
}

void _Future::changeState(Future::State state)
{
    switch (state) {
        case Future::Started: {
            removeState(Future::NoState | Future::Running | Future::Finished);
            addState(Future::Started);
            if (threadPool)
                threadPool->_runnableStateChanged(this->p());
        } break;

        case Future::Running: {
            removeState(Future::NoState | Future::Started | Future::Finished);
            addState(Future::Running);
            if (threadPool)
                threadPool->_runnableStateChanged(this->p());
        } break;

        case Future::Finished: {
            removeState(Future::NoState | Future::Started | Future::Running);
            addState(Future::Finished);
            if (threadPool)
                threadPool->_runnableStateChanged(this->p());
        } break;

        default:
            break;
    }
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
    if (p->threadPool) {
        //! @todo add comment
        return;
    }

    if (p->state.load() & Future::Canceled)
        return;

    p->removeState(Future::Paused);
    p->addState(Future::Canceled);

    p->threadPool->_runnableStateChanged(this);
}

void Future::setPaused(bool paused)
{
    _Future *p = this->p();
    if (p->threadPool) {
        //! @todo add comment
        return;
    }

    if (paused)
        p->addState(Future::Paused);
    else
        p->removeState(Future::Paused);

    p->threadPool->_runnableStateChanged(this);
}

void Future::waitForStarted()
{
    //! @todo add condition
}

void Future::waitForFinished()
{
    _Future *p = this->p();
    if (p->threadPool) {
        //! @todo add comment
        return;
    }

    p->threadPool->_waitRunnableFinished(this);
}

Future::State Future::state() const
{
    return NoState;
}

void Future::setThreadPool(ThreadPool *pool)
{
    _Future *p = this->p();
    p->threadPool = pool;
}

void Future::bindData(void *data)
{
    _Future *p = this->p();
    p->data = data;
}

SF_END_NAMESPACE
