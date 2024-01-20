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

    std::unique_ptr<BlockedTransporter> workTSP;
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
    void markThreadInactive();

    ThreadRunnableData *allocateRunnableData(const std::shared_ptr<Runnable> &runnable, ThreadData *currentThread);

    static void runnableStateChanged(va_list &args, ObjectImpl *impl);

    int maxThreadCount = 2;
    int activeThreadCount = 0;

    std::mutex mutex;
    std::condition_variable waitCondition;
    std::vector<ThreadData *> threads;
};

_ThreadPool::_ThreadPool(ThreadPool *pool)
    : ObjectImpl(pool)
{
    installVirtualHandler("_runnableStateChanged", &_ThreadPool::runnableStateChanged);
}

void _ThreadPool::start(void *arg)
{
    _ThreadPool *_p = reinterpret_cast<_ThreadPool *>(arg);
    std::unique_lock<std::mutex> locker(_p->mutex);
    ThreadData *threadData = currentThreadData(_p);
    if (!threadData) {
        _p->markThreadInactive();
        locker.unlock();
        return;
    }

    locker.unlock();

    for (;;) {
        do {
            ThreadRunnableData *runnableData = dynamic_cast<ThreadRunnableData *>(threadData->workTSP->takeData());
            if (!runnableData)
                break;

            if (runnableData->future->state() & Future::Canceled)
                break;

            if (runnableData->future->state() & Future::Paused) {
                locker.lock();
                threadData->pauseDatas.push_back(runnableData);
                locker.unlock();
                break;
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
                    locker.lock();
                    threadData->pauseDatas.push_back(runnableData);
                    locker.unlock();
                    break;
                }
            } while (true);
        } while (true);

        locker.lock();
        if (!std::any_of(_p->threads.begin(), _p->threads.end(),
                std::bind(std::equal_to<ThreadData *>(), std::placeholders::_1, threadData))) {
            _p->markThreadInactive();
            locker.unlock();
            break;
        }

        locker.unlock();
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

    if (it == threadPool->threads.end())
        return target;

    return (*it);
}

void _ThreadPool::prepareThreads(int threadCount)
{
    std::unique_lock<std::mutex> locker(mutex);
    if (threads.size() == threadCount)
        return;

    if (threadCount > _MAX_THREAD_COUNT) {
        //! @todo add comment
        return;
    }

    int8_t diff = threadCount - threads.size();
    if (diff > 0) {
        spawnNewThreads(diff);
    } else {
        decreaseThreads(std::abs(diff) > threadCount ? threadCount : std::abs(diff));
    }
}

void _ThreadPool::spawnNewThreads(int number)
{
    for (int i = 0; i < number; ++i) {
        ThreadData *data = new ThreadData;
        data->workTSP.reset(new BlockedTransporter);
        threads.push_back(data);
        activeThreadCount++;

        std::thread th(&_ThreadPool::start, this);
        data->thread = std::move(th);
    }
}

void _ThreadPool::decreaseThreads(int number)
{
    ThreadPool *_p = this->p();
    if (number == this->threads.size()) {
        _p->waitForFinished();
        return;
    }

    auto upwardElement = [](ThreadData *prev, ThreadData *next) {
        return prev->workTSP->dataCount() > next->workTSP->dataCount();
    };

    std::vector<ThreadData *> readyRemoveThreads = { threads.begin(), threads.begin() + number };
    std::make_heap(readyRemoveThreads.begin(), readyRemoveThreads.end(), upwardElement);
    std::sort_heap(readyRemoveThreads.begin(), readyRemoveThreads.end(), upwardElement);

    for (auto it = threads.begin() + number; it != threads.end();) {
        if ((*it)->workTSP->dataCount() < readyRemoveThreads.front()->workTSP->dataCount()) {
            std::pop_heap(readyRemoveThreads.begin(), readyRemoveThreads.end(), upwardElement);
            readyRemoveThreads.pop_back();
            readyRemoveThreads.push_back(std::move(*it));
            std::push_heap(readyRemoveThreads.begin(), readyRemoveThreads.end(), upwardElement);
            it = threads.erase(it);
        } else {
            ++it;
        }
    }

    for (ThreadData *threadData : readyRemoveThreads) {
        if (threadData->workTSP->dataCount() == 0) {
            threadData->workTSP->release();
            continue;
        }

        while (threadData->workTSP->dataCount() > 0) {
            AbstractData *data = threadData->workTSP->takeData();
            ThreadData *targetThreadData = scheduleThread();
            targetThreadData->workTSP->putData(data);
        }

        for (ThreadRunnableData *pausedData : threadData->pauseDatas) {
            ThreadData *targetThreadData = scheduleThread();
            targetThreadData->workTSP->putData(pausedData);
        }

        delete threadData;
    }
}

void _ThreadPool::markThreadInactive()
{
    activeThreadCount--;
    if (activeThreadCount == 0)
        waitCondition.notify_all();
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

ThreadData *_ThreadPool::scheduleThread()
{
    std::unique_lock<std::mutex> locker(mutex);
    // least load schedule
    //! @todo add more schedule algorithm
    ThreadData *leastThreadData = nullptr;
    int leastDataCount = 0;

    std::for_each(threads.begin(), threads.end(),
        [&](ThreadData *data) {
            int dataCount = data->workTSP->dataCount();
            if (dataCount <= leastDataCount) {
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
    waitForFinished();
}

void ThreadPool::setMaxThreads(int number)
{
    _ThreadPool *_p = this->p();
    if (_p->threads.empty()) {
        // Do not create any threads before the pool has started.
        return;
    }

    if (_p->maxThreadCount == number)
        return;

    _p->maxThreadCount = number;
    if (!_p->threads.empty()) {
        _p->prepareThreads(_p->maxThreadCount);
    }
}

int ThreadPool::maxThreads() const
{
    const _ThreadPool *_p = this->p();
    return _p->maxThreadCount;
}

int ThreadPool::platformIdealThreadCount()
{
    //! @todo add platform method.
    return 1;
}

int ThreadPool::activeThreadCount() const
{
    return p()->activeThreadCount;
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
    _ThreadPool *_p = this->p();
    std::unique_lock<std::mutex> locker(_p->mutex);

    std::vector<ThreadData *> threads_tmp;
    std::swap(_p->threads, threads_tmp);

    for (ThreadData *threadData : threads_tmp) {
        threadData->workTSP->release();
    }

    while (!_p->activeThreadCount == 0) {
        _p->waitCondition.wait(locker);
    }

    for (ThreadData *threadData : threads_tmp) {
        for (ThreadRunnableData *runnaleData : threadData->pauseDatas) {
            runnaleData->future->_setState(Future::Canceled);
            runnaleData->future->_bindData(nullptr);

            delete runnaleData;
            threadData->pauseDatas.clear();
        }

        threadData->thread.join();
        delete threadData;
    }
}

SF_END_NAMESPACE