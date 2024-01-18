#include "threadpool.h"
#include "blocked_transporter.h"
#include "future.h"
#include "runnable.h"

#include "base/data.h"
#include "private/object_p.h"

#include <assert.h>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

SF_BEGIN_NAMESPACE

#define _MAX_THREAD_COUNT 512

struct ThreadData;
struct ThreadRunnableData : public AbstractData {
    ThreadData *thread = nullptr;
    std::shared_ptr<Future> future;

    std::shared_ptr<Runnable> headRunnable = nullptr;
    Runnable *currentRunnable = nullptr;
};

struct ThreadData {
    std::thread thread;

    std::mutex queueMutex;
    std::condition_variable queueCondition;
    BlockedTransporter *workTSP;
    std::vector<ThreadRunnableData *> pauseDatas;
};

class _ThreadPool : public ObjectImpl {
    SF_PUBLIC_CLASS(ThreadPool)

public:
    _ThreadPool(ThreadPool *pool);

    static void start(void *arg);
    static ThreadData *currentThreadData(_ThreadPool *threadPool);

    void prepareThreads(int threadCount);
    ThreadData *scheduleThread();
    void spawnNewThreads(int number);
    void decreaseThreads(int number);

    ThreadRunnableData *allocateRunnableData(const std::shared_ptr<Runnable> &runnable, ThreadData *currentThread);

    static void runnableStateChanged(va_list &args, ObjectImpl *impl);
    static void waitRunnableStarted(va_list &args, ObjectImpl *impl);
    static void waitRunnableFinished(va_list &args, ObjectImpl *impl);

    int maxThreadCount = 0;
    std::vector<ThreadData *> threads;
};

_ThreadPool::_ThreadPool(ThreadPool *pool)
    : ObjectImpl(pool)
{
    handlers = {
        { "_runnableStateChanged", &_ThreadPool::runnableStateChanged },
        { "_waitRunnableStarted", &_ThreadPool::waitRunnableStarted },
        { "_waitRunnableFinished", &_ThreadPool::waitRunnableFinished },
    };
}

void _ThreadPool::start(void *arg)
{
    _ThreadPool *_p = reinterpret_cast<_ThreadPool *>(arg);
    ThreadData *threadData = currentThreadData(_p);

    for (;;) {
        do {
            if (threadData->workTSP->dataCount() == 0)
                break;

            ThreadRunnableData *runnableData = dynamic_cast<ThreadRunnableData *>(threadData->workTSP->takeData());
            assert(runnableData);

            if (runnableData->future->state() & Future::Canceled)
                continue;

            if (runnableData->future->state() & Future::Paused) {
                threadData->pauseDatas.push_back(runnableData);
                continue;
            }

            Runnable *runnable = runnableData->currentRunnable;

            do {
                runnableData->future->_setState(Future::State(Future::Started | Future::Running));
                runnable->execute(nullptr /*! @todo context */);
                runnable = runnable->next();
                runnableData->currentRunnable = runnable;

                if (!runnable) {
                    runnableData->future->_setState(Future::Finished);
                    runnableData->future->_bindData(nullptr);
                    delete runnableData;
                    break;
                }

                if (runnableData->future->state() == Future::Paused) {
                    threadData->pauseDatas.push_back(runnableData);
                    break;
                }
            } while (true);
        } while (true);
    }
}

ThreadData *_ThreadPool::currentThreadData(_ThreadPool *threadPool)
{
    assert(threadPool);

    ThreadData *target = nullptr;
    const auto &tid = std::this_thread::get_id();
    auto it = std::find_if(threadPool->threads.begin(), threadPool->threads.end(), [=](const ThreadData *data) {
        return (data->thread.get_id() == tid);
    });

    if (it != threadPool->threads.end())
        return target;

    return (*it);
}

void _ThreadPool::prepareThreads(int threadCount)
{
    if (maxThreadCount == threadCount)
        return;

    if (threadCount > _MAX_THREAD_COUNT) {
        //! @todo add comment
        return;
    }

    int8_t diff = threadCount - maxThreadCount;
    if (diff > 0) {
        spawnNewThreads(diff);
    } else {
        decreaseThreads(diff);
    }
}

void _ThreadPool::spawnNewThreads(int number)
{
    for (int i = 0; i < number; ++i) {
        std::thread th(&_ThreadPool::start, this);

        ThreadData *data = new ThreadData;
        data->thread = std::move(th);
        threads.push_back(data);
    }
}

void _ThreadPool::decreaseThreads(int number)
{
}

ThreadRunnableData *_ThreadPool::allocateRunnableData(const std::shared_ptr<Runnable> &runnable, ThreadData *currentThread)
{
    ThreadRunnableData *runnableData = new ThreadRunnableData;
    runnableData->headRunnable = runnable;
    runnableData->currentRunnable = runnable.get();
    runnableData->thread = currentThread;

    std::shared_ptr<Future> future(new Future);
    future->_setThreadPool(p());
    future->_bindData(static_cast<void *>(runnableData));

    runnableData->future = std::move(future);
    return runnableData;
}

void _ThreadPool::runnableStateChanged(va_list &args, ObjectImpl *impl)
{
    Future *future = va_arg(args, Future *);
    void *bindedData = va_arg(args, void *);

    _ThreadPool *_p = dynamic_cast<_ThreadPool *>(impl);
    ThreadRunnableData *runnableData = static_cast<ThreadRunnableData *>(bindedData);
    Future::State state = future->state();

    if ((state & Future::Running) && !(state & Future::Paused) && !(state & Future::Canceled)) {
        ThreadData *threadData = runnableData->thread;
        threadData->pauseDatas.erase(std::remove_if(threadData->pauseDatas.begin(),
            threadData->pauseDatas.end(), [runnableData](ThreadRunnableData *data) { return data == runnableData; }));

        threadData->workTSP->putData(runnableData);
    } else if (state & Future::Canceled) {
        ThreadData *threadData = runnableData->thread;
        threadData->pauseDatas.erase(std::remove_if(threadData->pauseDatas.begin(),
            threadData->pauseDatas.end(), [runnableData](ThreadRunnableData *data) { return data == runnableData; }));

        future->_bindData(nullptr);
        delete runnableData;
    }
}

void _ThreadPool::waitRunnableStarted(va_list &args, ObjectImpl *impl)
{
    Future *future = va_arg(args, Future *);
}

void _ThreadPool::waitRunnableFinished(va_list &args, ObjectImpl *impl)
{
    Future *future = va_arg(args, Future *);
}

ThreadData *_ThreadPool::scheduleThread()
{
    // least load schedule
    //! @todo add more schedule algorithm
    ThreadData *leastThreadData = nullptr;
    int leastDataCount = 0;

    std::for_each(threads.begin(), threads.end(),
        [&](ThreadData *data) {
            int dataCount = data->workTSP->dataCount();
            if (dataCount < leastDataCount) {
                leastDataCount = dataCount;
                leastThreadData = data;
            }
        });

    return leastThreadData;
}

ThreadPool::ThreadPool()
    : Object(new _ThreadPool(this))
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::setMaxThreads(int number)
{
    _ThreadPool *_p = this->p();
    if (_p->threads.empty()) {
        // Do not create a thread before it has started.
        return;
    }

    if (_p->maxThreadCount == number)
        return;

    _p->maxThreadCount = number;
    _p->prepareThreads(number);
}

int ThreadPool::maxThreads() const
{
    const _ThreadPool *_p = this->p();
    return _p->maxThreadCount;
}

int ThreadPool::platformIdealThreadCount()
{
    return 0;
}

int ThreadPool::activeThreadCount() const
{
    return p()->threads.size();
}

std::shared_ptr<Future> ThreadPool::start(const std::shared_ptr<Runnable> &runnable)
{
    _ThreadPool *_p = this->p();

    if (_p->threads.empty()) {
        _p->prepareThreads(_p->maxThreadCount);

        assert(!_p->threads.empty());
    }

    ThreadData *threadData = _p->scheduleThread();
    assert(threadData);

    ThreadRunnableData *runnableData = _p->allocateRunnableData(runnable, threadData);
    threadData->workTSP->putData(runnableData);

    return runnableData->future;
}

void ThreadPool::waitForFinished()
{
}

SF_END_NAMESPACE