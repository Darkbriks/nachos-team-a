#include "syscall.h"
#include "types.h"
#include "nos_pthread.h"

int main() {
    PutString("=== Test pthread Attr Syscalls ===\n", 36);

    pthread_attr_t attr;
    int detachstate;

    // Test init
    if (pthread_attr_init(&attr) != 0) {
        PutString("FAIL: attr_init\n", 16);
        return 1;
    }
    PutString("PASS: attr_init\n", 16);

    // Test get default
    if (pthread_attr_getdetachstate(&attr, &detachstate) != 0) {
        PutString("FAIL: attr_getdetachstate\n", 26);
        return 1;
    }

    if (detachstate != JOINABLE) {
        PutString("FAIL: default should be JOINABLE\n", 33);
        return 1;
    }
    PutString("PASS: default is JOINABLE\n", 26);

    // Test set
    if (pthread_attr_setdetachstate(&attr, DETACHED) != 0) {
        PutString("FAIL: attr_setdetachstate\n", 26);
        return 1;
    }

    pthread_attr_getdetachstate(&attr, &detachstate);

    if (detachstate != DETACHED) {
        PutString("FAIL: should be DETACHED\n", 25);
        return 1;
    }
    PutString("PASS: set to DETACHED works\n", 28);

    // Test set back to JOINABLE
    if (pthread_attr_setdetachstate(&attr, JOINABLE) != 0) {
        PutString("FAIL: attr_setdetachstate to JOINABLE\n", 39);
        return 1;
    }

    if (pthread_attr_getdetachstate(&attr, &detachstate) != 0) {
        PutString("FAIL: attr_getdetachstate after set to JOINABLE\n", 45);
        return 1;
    }

    if (detachstate != JOINABLE) {
        PutString("FAIL: should be JOINABLE\n", 25);
        return 1;
    }
    PutString("PASS: set to JOINABLE works\n", 28);

    // Test destroy
    if (pthread_attr_destroy(&attr) != 0) {
        PutString("FAIL: attr_destroy\n", 19);
        return 1;
    }
    PutString("PASS: attr_destroy\n", 19);

    PutString("SUCCESS: All attr tests passed\n", 31);
    return 0;
}
