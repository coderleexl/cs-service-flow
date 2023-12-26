#include "threadpool.h"
#include "base/object_p.h"
#include "future.h"
#include "runnable.h"

#include <assert.h>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

SF_BEGIN_NAMESPACE

#define _MAX_THREAD_COUNT 512

struct ThreadData;
struct ThreadRunnableData {
    ThreadData *thread = nullptr;
    std::shared_ptr<Runnable> runnable = nullptr;
    std::shared_ptr<Future> future;
};

struct ThreadData {
    std::thread thread;

    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::queue<ThreadRunnableData *> workQueue;

    inline int queueSize()
    {
        queueMutex.lock();
        int size = workQueue.size();
        queueMutex.unlock();

        return size;
    }

    inline void pushData(ThreadRunnableData *data)
    {
        queueMutex.lock();
        workQueue.push(data);
        queueCondition.notify_one();
        queueMutex.unlock();
    }

    inline ThreadRunnableData *takeData()
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        while (workQueue.empty()
            || workQueue.front()->future->state() & Future::Paused) {
            queueCondition.wait(lock);
        }

        ThreadRunnableData *data = workQueue.front();
        workQueue.pop();

        return data;
    }
};

class _ThreadPool : public ObjectImpl {
    SF_PUBLIC_CLASS(ThreadPool)

public:
    _ThreadPool(ThreadPool *poll)
        : ObjectImpl(poll)
    {
    }

    static void start(void *arg);

    void prepareThreads(int threadCount);

    void spawnNewThreads(int number);
    void decreaseThreads(int number);

    ThreadRunnableData *allocateRunnableData(const std::shared_ptr<Runnable> &runnable, ThreadData *currentThread);

    ThreadData *scheduleThread();

    int maxThreadCount = 0;
    std::vector<ThreadData *> threads;
};

void _ThreadPool::start(void *arg)
{
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
    runnableData->runnable = runnable;
    runnableData->thread = currentThread;

    std::shared_ptr<Future> future(new Future);
    future->setThreadPool(p());
    future->bindData((void *) runnableData);

    runnableData->future = std::move(future);
    return runnableData;
}

ThreadData *_ThreadPool::scheduleThread()
{
    // least load schedule
    //! @todo add more schedule algorithm
    ThreadData *leastThreadData = nullptr;
    int leastQueueSize = 0;

    std::for_each(threads.begin(), threads.end(),
        [&](ThreadData *data) {
            int queueSize = data->queueSize();
            if (queueSize < leastQueueSize) {
                leastQueueSize = queueSize;
                leastThreadData = data;
            }
        });

    return leastThreadData;
}

void createThreadFuture(ThreadData *data)
{
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
}

int ThreadPool::maxThreads() const
{
    return 0;
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
    threadData->pushData(runnableData);

    return runnableData->future;
}

void ThreadPool::waitForFinished()
{
}

SF_END_NAMESPACE