#ifndef NOS_ASSERT_H
#define NOS_ASSERT_H

#ifndef NDEBUG
#define NDEBUG 1
#endif

#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#define assert(expression)                                              \
    do {                                                                \
        if (!(expression)) {                                            \
            fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", \
                    #expression, __FILE__, __LINE__);                   \
            abort();                                                    \
        }                                                               \
    } while (0)
#endif

#endif