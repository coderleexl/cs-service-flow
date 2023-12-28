#include "blocked_transporter.h"

#include "private/object_p.h"

#include <condition_variable>
#include <memory>
#include <queue>

SF_BEGIN_NAMESPACE

class _BlockedTransporter : public ObjectImpl {
    SF_PUBLIC_CLASS(BlockedTransporter)
public:
    _BlockedTransporter(BlockedTransporter *tran);

private:
    mutable std::mutex mutex;
    std::condition_variable_any condition;
    std::queue<AbstractData *> dataVector;
    bool release;
};

_BlockedTransporter::_BlockedTransporter(BlockedTransporter *tran)
    : ObjectImpl(tran)
    , release(false)
{
}

BlockedTransporter::BlockedTransporter()
    : Transporter(new _BlockedTransporter(this))
{
}

BlockedTransporter::~BlockedTransporter()
{
}

void BlockedTransporter::putData(AbstractData *data)
{
    _BlockedTransporter *p = this->p();
    std::unique_lock<std::mutex> locker(p->mutex);

    p->dataVector.push(data);
    p->condition.notify_all();
}

AbstractData *BlockedTransporter::takeData()
{
    _BlockedTransporter *p = this->p();
    std::unique_lock<std::mutex> locker(p->mutex);

    while (p->dataVector.empty() && !p->release)
        p->condition.wait(p->mutex);

    AbstractData *data = nullptr;
    if (p->dataVector.size() > 0) {
        data = p->dataVector.front();
        p->dataVector.pop();
    }

    return data;
}

AbstractData *BlockedTransporter::firstData() const
{
    const _BlockedTransporter *p = this->p();
    std::unique_lock<std::mutex> locker(p->mutex);

    return p->dataVector.front();
}

int BlockedTransporter::dataCount() const
{
    const _BlockedTransporter *p = this->p();
    std::unique_lock<std::mutex> locker(p->mutex);

    return p->dataVector.size();
}

void BlockedTransporter::release()
{
    _BlockedTransporter *p = this->p();
    std::unique_lock<std::mutex> locker(p->mutex);

    p->release = true;
    p->condition.notify_all();
}

SF_END_NAMESPACE
