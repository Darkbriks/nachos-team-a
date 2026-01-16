#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_START(name) \
    do { \
        tests_run++; \
        PutString("[TEST] ", 10); \
        PutString(name, 100); \
        PutString("... ", 4); \
    } while(0)

#define TEST_PASS() \
    do { \
        tests_passed++; \
        PutString("PASS\n", 5); \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        PutString("FAIL: ", 6); \
        PutString(msg, 100); \
        PutChar('\n'); \
    } while(0)

#define ASSERT_EQ(actual, expected, msg) \
    do { \
        if ((actual) != (expected)) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_STREQ(actual, expected, msg) \
    do { \
        if (strcmp(actual, expected) != 0) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, msg) \
    do { \
        if ((ptr) == NULL) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#endif