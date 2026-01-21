#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"

static void test_getstring_n_zero(void) {
    TEST_START("getstring_n_zero");

    char buffer[10];

    CLEAR_ERRNO();
    int ret = GetString(buffer, 0);

    ASSERT_EQ(ret, 0, "n=0 devrait retourner 0");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

static void test_getstring_n_negatif(void) {
    TEST_START("getstring_n_negatif");

    char buffer[10];

    CLEAR_ERRNO();
    int ret = GetString(buffer, -5);

    ASSERT_ERROR(ret, "n negatif devrait echouer");
    ASSERT_ERRNO(E_INVAL, "errno devrait etre E_INVAL");

    TEST_PASS();
}

static void test_getstring_null_pointer(void) {
    TEST_START("getstring_null_pointer");

    CLEAR_ERRNO();
    int ret = GetString((char*)0, 10);

    ASSERT_ERROR(ret, "NULL pointer devrait echouer");
    ASSERT_ERRNO(E_FAULT, "errno devrait etre E_FAULT");

    TEST_PASS();
}

static void test_getstring_adresse_invalide(void) {
    TEST_START("getstring_adresse_invalide");

    CLEAR_ERRNO();
    int ret = GetString((char*)0xDEADBEEF, 10);

    ASSERT_ERROR(ret, "adresse invalide devrait echouer");
    ASSERT_ERRNO(E_FAULT, "errno devrait etre E_FAULT");

    TEST_PASS();
}

int main()
{
    TEST_SUITE_START("GetString Error Tests");

    test_getstring_n_zero();
    test_getstring_n_negatif();
    test_getstring_null_pointer();
    test_getstring_adresse_invalide();

    TEST_SUITE_END();
    return 0;
}
