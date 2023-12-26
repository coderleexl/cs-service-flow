#define SF_FRIEND_CLASS_IMPL(NAME) \
    friend class NAME;             \
    friend class _##NAME;

#define SF_PRIVATE_CLASS_IMPL(NAME)                                 \
private:                                                            \
    friend class _##NAME;                                           \
                                                                    \
    inline const _##NAME *p() const                                 \
    {                                                               \
        return reinterpret_cast<const _##NAME *>(_impl.get());      \
    }                                                               \
    inline _##NAME *p()                                             \
    {                                                               \
        return reinterpret_cast<_##NAME *>(_impl.get());            \
    }                                                               \
    inline std::shared_ptr<NAME> shared_this()                      \
    {                                                               \
        return std::dynamic_pointer_cast<NAME>(shared_from_this()); \
    }

#define SF_PUBLIC_CLASS_IMPL(NAME)                   \
private:                                             \
    friend class NAME;                               \
                                                     \
    inline const NAME *p() const                     \
    {                                                \
        return reinterpret_cast<const NAME *>(_pub); \
    }                                                \
    inline NAME *p()                                 \
    {                                                \
        return reinterpret_cast<NAME *>(_pub);       \
    }

#define SF_PRIVATE_FUNCTION_IMPL(NAME, ...)                                         \
    void NAME(_MAKE_PARAMS(__VA_ARGS__))                                            \
    {                                                                               \
        vo()->virtual_call(#NAME, _NUM_ARGS(__VA_ARGS__), _MAKE_ARGS(__VA_ARGS__)); \
    }

#define _NUM_ARGS2(X, X64, X63, X62, X61, X60, X59, X58, X57, X56, X55, X54, X53, X52, X51, X50, X49, X48, X47, X46, X45, X44, X43, X42, X41, X40, X39, X38, X37, X36, X35, X34, X33, X32, X31, X30, X29, X28, X27, X26, X25, X24, X23, X22, X21, X20, X19, X18, X17, X16, X15, X14, X13, X12, X11, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N, ...) N
#define _NUM_ARGS(...) _NUM_ARGS2(0, ##__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _MAKE_PARAMS_0()
#define _MAKE_PARAMS_1(type) type arg1
#define _MAKE_PARAMS_2(type1, type2) type1 arg1, type2 arg2
#define _MAKE_PARAMS_3(type1, type2, type3) type1 arg1, type2 arg2, type3 arg3

#define _MAKE_PARAMS_N(N, ...) _MAKE_PARAMS_##N(__VA_ARGS__)
#define _MAKE_PARAMS_FORCE_N(N, ...) _MAKE_PARAMS_N(N, __VA_ARGS__)
#define _MAKE_PARAMS(...) _MAKE_PARAMS_FORCE_N(_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

#define _MAKE_ARGS_0()
#define _MAKE_ARGS_1(type) arg1
#define _MAKE_ARGS_2(type1, type2) arg1, arg2
#define _MAKE_ARGS_3(type1, type2, type3) arg1, arg2, arg3

#define _MAKE_ARGS_N(N, ...) _MAKE_ARGS_##N(__VA_ARGS__)
#define _MAKE_ARGS_FORCE_N(N, ...) _MAKE_ARGS_N(N, __VA_ARGS__)
#define _MAKE_ARGS(...) _MAKE_ARGS_FORCE_N(_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)