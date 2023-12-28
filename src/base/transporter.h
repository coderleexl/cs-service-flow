#ifndef _BASE_TRANSPORTER_H_
#define _BASE_TRANSPORTER_H_

#include "object.h"
#include "sf_global.h"

SF_BEGIN_NAMESPACE

struct AbstractData;
class SF_LIBRARY_EXPORT Transporter : public Object {
public:
    Transporter();
    virtual ~Transporter();

    virtual void putData(AbstractData *msg) = 0;
    virtual AbstractData *takeData() = 0;

    virtual AbstractData *firstData() const = 0;
    virtual int dataCount() const = 0;

    virtual void release() = 0;

protected:
    Transporter(ObjectImpl *impl);
};

SF_END_NAMESPACE

#endif // _BASE_TRANSPORTER_H_