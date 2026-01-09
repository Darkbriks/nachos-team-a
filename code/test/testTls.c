#include "syscall.h"
#include "tls.h"

void verify_tls_layout(void) {
    tls_t test_tls;
    char *base = (char*)&test_tls;

    int offset_self = (char*)&test_tls.self - base;
    int offset_tid = (char*)&test_tls.tid - base;
    int offset_pid = (char*)&test_tls.pid - base;
    int offset_errno = (char*)&test_tls.errno_val - base;
    int offset_stack_base = (char*)&test_tls.stack_base - base;
    int offset_stack_size = (char*)&test_tls.stack_size - base;
    int offset_flags = (char*)&test_tls.flags - base;
    int offset_retval = (char*)&test_tls.retval - base;
    int offset_tsd = (char*)&test_tls.tsd[0] - base;

    PutString("=== TLS Structure Layout ===\n", 100);

    PutString("sizeof(tls_t) = ", 20);
    PutInt((int)sizeof(tls_t));
    PutString(" bytes\n", 10);

    PutString("Offset self:           ", 30);
    PutInt(offset_self);
    PutString(" (expected 0)\n", 20);

    PutString("Offset tid:            ", 30);
    PutInt(offset_tid);
    PutString(" (expected 4)\n", 20);

    PutString("Offset pid:            ", 30);
    PutInt(offset_pid);
    PutString(" (expected 8)\n", 20);

    PutString("Offset errno_val:      ", 30);
    PutInt(offset_errno);
    PutString(" (expected 12)\n", 20);

    PutString("Offset stack_base:     ", 30);
    PutInt(offset_stack_base);
    PutString(" (expected 16)\n", 20);

    PutString("Offset stack_size:     ", 30);
    PutInt(offset_stack_size);
    PutString(" (expected 20)\n", 20);

    PutString("Offset flags:          ", 30);
    PutInt(offset_flags);
    PutString(" (expected 24)\n", 20);

    PutString("Offset retval:         ", 30);
    PutInt(offset_retval);
    PutString(" (expected 28)\n", 20);

    PutString("Offset tsd[0]:         ", 30);
    PutInt(offset_tsd);
    PutString(" (expected 32)\n", 20);
}

int main(void) {
    verify_tls_layout();

    tls_t my_tls;
    my_tls.self = &my_tls;
    my_tls.tid = 42;
    my_tls.pid = 1;
    my_tls.errno_val = 0;
    my_tls.stack_base = (void*)0xDEADBEEF;
    my_tls.stack_size = 4096;
    my_tls.flags = TLS_FLAG_JOINABLE;
    my_tls.retval = (void*)0;

    PutString("TLS address: ", 15);
    PutInt((int)&my_tls);
    PutChar('\n');

    PutString("self points to: ", 20);
    PutInt((int)my_tls.self);
    PutChar('\n');

    PutString("tid: ", 10);
    PutInt((int)my_tls.tid);
    PutChar('\n');

    return 0;
}