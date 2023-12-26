#ifndef _SF_GLOBAL_HEADER_
#define _SF_GLOBAL_HEADER_

#ifdef SERVICE_FLOW_LIBRARY
#define SF_LIBRARY_EXPORT _declspec(dllexport)
#else
#define SF_LIBRARY_EXPORT
#endif

#ifdef SF_NAMESPACE
#define SF_PREPEND_NAMESPACE(name) ::SF_NAMESPACE::name
#define SF_BEGIN_NAMESPACE \
    namespace SF_NAMESPACE \
    {
#define SF_END_NAMESPACE }
#define SF_USING_NAMESPACE using ::SF_NAMESPACE;
#define SF_FORWARD_DECLARE_CLASS(name) \
    SF_BEGIN_NAMESPACE class name;     \
    SF_END_NAMESPACE                   \
    using SF_PREPEND_NAMESPACE(name);

#define SF_FORWARD_DECLARE_STRUCT(name) \
    SF_BEGIN_NAMESPACE struct name;     \
    SF_END_NAMESPACE                    \
    using SF_PREPEND_NAMESPACE(name);

#else
#define SF_PREPEND_NAMESPACE(name) ::name
#define SF_BEGIN_NAMESPACE
#define SF_END_NAMESPACE
#define SF_USING_NAMESPACE
#define SF_FORWARD_DECLARE_CLASS(name) class name;
#define SF_FORWARD_DECLARE_STRUCT(name) struct name;
#endif

#define SF_FRIEND_CLASS(NAME) SF_FRIEND_CLASS_IMPL(NAME)
#define SF_PRIVATE_CLASS(NAME) SF_PRIVATE_CLASS_IMPL(NAME)
#define SF_PUBLIC_CLASS(NAME) SF_PUBLIC_CLASS_IMPL(NAME)
#define SF_PRIVATE_FUNCTION(NAME, ...) SF_PRIVATE_FUNCTION_IMPL(NAME, __VA_ARGS__)

#include "base/global_p.h"

#endif // _SF_GLOBAL_HEADER_