#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"

static void test_getint_null_pointer(void) {
    TEST_START("getint_null_pointer");

    CLEAR_ERRNO();
    int ret = GetInt((int*)0);

    ASSERT_ERROR(ret, "NULL pointer devrait echouer");
    ASSERT_ERRNO(E_FAULT, "errno devrait etre E_FAULT");

    TEST_PASS();
}

static void test_getint_adresse_invalide(void) {
    TEST_START("getint_adresse_invalide");

    CLEAR_ERRNO();
    int ret = GetInt((int*)0xDEADBEEF);

    ASSERT_ERROR(ret, "adresse invalide devrait echouer");
    ASSERT_ERRNO(E_FAULT, "errno devrait etre E_FAULT");

    TEST_PASS();
}

int main()
{
    TEST_SUITE_START("GetInt Error Tests");

    test_getint_null_pointer();
    test_getint_adresse_invalide();

    TEST_SUITE_END();
}