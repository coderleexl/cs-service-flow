#ifndef _THREADED_BLOCKED_TRANSPORTER_H_
#define _THREADED_BLOCKED_TRANSPORTER_H_

#include "base/transporter.h"

SF_BEGIN_NAMESPACE

class _BlockedTransporter;

class SF_LIBRARY_EXPORT BlockedTransporter : public Transporter {
    SF_PRIVATE_CLASS(BlockedTransporter)
public:
    BlockedTransporter();
    ~BlockedTransporter();

    void putData(AbstractData *data) override;
    AbstractData *takeData() override;

    AbstractData *firstData() const override;
    int dataCount() const override;

    void release() override;
};

SF_END_NAMESPACE

#endif // _THREADED_BLOCKED_TRANSPORTER_H_
