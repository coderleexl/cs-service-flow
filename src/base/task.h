#ifndef _BASE_TASK_H_
#define _BASE_TASK_H_

#include <string>

#include "object.h"
#include "sf_global.h"

SF_BEGIN_NAMESPACE

class Context;
class SF_LIBRARY_EXPORT Task : public Object {
public:
    Task();
    virtual ~Task() = default;

    virtual std::string name() const = 0;

    virtual void precede(Task *task);
    virtual void behind(Task *task);
    virtual Task *next() const;

    virtual void prepare(Context *context) = 0;
    virtual void execute(Context *context) = 0;

protected:
    Task(ObjectImpl *impl);
};

SF_END_NAMESPACE

#endif // _BASE_TASK_H_