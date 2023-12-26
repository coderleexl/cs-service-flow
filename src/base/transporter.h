#ifndef _BASE_TRANSPORTER_H_
#define _BASE_TRANSPORTER_H_

#include "sf_global.h"

SF_BEGIN_NAMESPACE

struct Message;
class SF_LIBRARY_EXPORT Transporter {
public:
    Transporter() = default;
    virtual ~Transporter() {};

    virtual void putMessage(Message* msg) = 0;
    virtual Message* takeMessage()        = 0;

    virtual Message* firstMessage() const = 0;
    virtual int messageCount() const      = 0;
};

SF_END_NAMESPACE

#endif // _BASE_TRANSPORTER_H_