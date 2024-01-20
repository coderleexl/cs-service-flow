#include <iostream>

#include "threaded/future.h"
#include "threaded/runnable.h"
#include "threaded/threadpool.h"

#include <thread>

SF_USING_NAMESPACE

class TestRunnable : public Runnable {
public:
    TestRunnable(int count)
        : Runnable()
        , mCount(count)
    {
    }

    void prepare(Context *context) override
    {
    }

    void execute(Context *context) override
    {
        std::cout << "------------" << name() << "---------" << mCount << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string name() const override
    {
        return "Test Runnable";
    }

private:
    int mCount = 0;
};

int main()
{
    auto runnable1 = std::make_shared<TestRunnable>(1);
    TestRunnable runnable2(2);
    TestRunnable runnable3(3);
    TestRunnable runnable4(4);
    TestRunnable runnable5(5);
    TestRunnable runnable6(6);

    runnable1->precede(&runnable2);
    runnable2.precede(&runnable3);
    runnable3.precede(&runnable4);
    runnable4.precede(&runnable5);
    runnable5.precede(&runnable6);

    ThreadPool pool;
    auto feture = pool.start(runnable1);

    while (feture->state() != Future::Finished) {
        std::cout << "State: " << feture->state() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
