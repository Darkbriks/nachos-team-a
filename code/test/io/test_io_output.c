#include "syscall.h"
#include "nos_errno.h"
#include "test_utilities.h"

/* ============================================================
 * TESTS PUTCHAR
 * ============================================================ */

/**
 * @brief Test PutChar avec un caractère ASCII standard
 * Attendu: le caractère est affiché
 */
static void test_putchar_ascii_standard(void) {
    TEST_START("putchar_ascii_standard");

    PutChar('A');
    PutChar('B');
    PutChar('C');

    /* Vérification visuelle: "ABC" doit être affiché */
    TEST_PASS();
}

/**
 * @brief Test PutChar avec caractères spéciaux
 * Attendu: caractères de contrôle fonctionnent
 */
static void test_putchar_special(void) {
    TEST_START("putchar_special");

    PutChar('\n');
    PutChar('\t');
    PutChar(' ');

    TEST_PASS();
}

/**
 * @brief Test PutChar avec valeurs limites char
 * Attendu: valeurs 0 et 127 acceptées
 */
static void test_putchar_limites(void) {
    TEST_START("putchar_limites");

    PutChar(0);
    PutChar(127);
    PutChar(1);

    TEST_PASS();
}

/**
 * @brief Test PutChar avec caractères numériques
 */
static void test_putchar_digits(void) {
    TEST_START("putchar_digits");

    for (char c = '0'; c <= '9'; c++) {
        PutChar(c);
    }
    PutChar('\n');

    /* Attendu: "0123456789\n" */
    TEST_PASS();
}

/* ============================================================
 * TESTS PUTSTRING - CAS NOMINAUX
 * ============================================================ */

/**
 * @brief Test PutString nominal - chaîne simple
 * Attendu: retourne le nombre de caractères écrits
 */
static void test_putstring_nominal(void) {
    TEST_START("putstring_nominal");

    char msg[] = "Hello, NachOS!";
    int len = 14;

    CLEAR_ERRNO();
    int ret = PutString(msg, len);
    PutChar('\n');

    ASSERT_EQ(ret, len, "retour incorrect");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec n plus grand que la chaîne
 * Attendu: s'arrête au '\0', retourne strlen(s)
 */
static void test_putstring_n_superieur_strlen(void) {
    TEST_START("putstring_n_superieur_strlen");

    char msg[] = "Court";

    CLEAR_ERRNO();
    int ret = PutString(msg, 10);
    PutChar('\n');

    ASSERT_EQ(ret, 5, "retour devrait etre strlen de la chaine");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec n égal à strlen
 * Attendu: écrit exactement n caractères
 */
static void test_putstring_n_egal_strlen(void) {
    TEST_START("putstring_n_egal_strlen");

    char msg[] = "Exact";

    CLEAR_ERRNO();
    int ret = PutString(msg, 5);
    PutChar('\n');

    ASSERT_EQ(ret, 5, "retour incorrect");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec chaîne vide et n > 0
 */
static void test_putstring_chaine_vide(void) {
    TEST_START("putstring_chaine_vide");

    char msg[] = "";

    CLEAR_ERRNO();
    int ret = PutString(msg, 10);

    ASSERT_EQ(ret, 0, "retour devrait etre n");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec caractères spéciaux dans la chaîne
 */
static void test_putstring_special_chars(void) {
    TEST_START("putstring_special_chars");

    char msg[] = "Tab:\tNewline:\nEnd";
    int len = 17;

    CLEAR_ERRNO();
    int ret = PutString(msg, len);
    PutChar('\n');

    ASSERT_EQ(ret, len, "retour incorrect");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec exactement MAX_STRING_SIZE (256) caractères
 */
static void test_putstring_max_string_size(void) {
    TEST_START("putstring_max_string_size");

    char buffer[MAX_STRING_SIZE + 1];
    for (int i = 0; i < MAX_STRING_SIZE; i++) {
        buffer[i] = 'A' + (i % 26);
    }
    buffer[MAX_STRING_SIZE] = '\0';

    CLEAR_ERRNO();
    int ret = PutString(buffer, MAX_STRING_SIZE);
    PutChar('\n');

    ASSERT_EQ(ret, MAX_STRING_SIZE, "retour incorrect pour MAX_STRING_SIZE");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec plus de MAX_STRING_SIZE
 */
static void test_putstring_proche_max_put_string(void) {
    TEST_START("putstring_proche_max_put_string");

    char buffer[1001];
    for (int i = 0; i < 1000; i++) {
        buffer[i] = '0' + (i % 10);
    }
    buffer[1000] = '\0';

    CLEAR_ERRNO();
    int ret = PutString(buffer, 1000);
    PutChar('\n');

    ASSERT_EQ(ret, 1000, "retour incorrect");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/* ============================================================
 * TESTS PUTSTRING - CAS D'ERREUR
 * ============================================================ */

/**
 * @brief Test PutString avec n = 0
 * Attendu: retourne 0, pas d'erreur
 */
static void test_putstring_n_zero(void) {
    TEST_START("putstring_n_zero");

    char msg[] = "Test";

    CLEAR_ERRNO();
    int ret = PutString(msg, 0);

    ASSERT_EQ(ret, 0, "n=0 devrait retourner 0");
    ASSERT_ERRNO(E_SUCCESS, "errno devrait etre 0");

    TEST_PASS();
}

/**
 * @brief Test PutString avec n négatif
 * Attendu: retourne -1, errno = E_INVAL
 */
static void test_putstring_n_negatif(void) {
    TEST_START("putstring_n_negatif");

    char msg[] = "Test";

    CLEAR_ERRNO();
    int ret = PutString(msg, -1);

    ASSERT_ERROR(ret, "n negatif devrait echouer");
    ASSERT_ERRNO(E_INVAL, "errno devrait etre E_INVAL");

    TEST_PASS();
}

/**
 * @brief Test PutString avec n très négatif
 * Attendu: retourne -1, errno = E_INVAL
 */
static void test_putstring_n_tres_negatif(void) {
    TEST_START("putstring_n_tres_negatif");

    char msg[] = "Test";

    CLEAR_ERRNO();
    int ret = PutString(msg, -999999);

    ASSERT_ERROR(ret, "n tres negatif devrait echouer");
    ASSERT_ERRNO(E_INVAL, "errno devrait etre E_INVAL");

    TEST_PASS();
}

/**
 * @brief Test PutString avec pointeur NULL
 * Attendu: retourne -1, errno = E_FAULT
 */
static void test_putstring_null_pointer(void) {
    TEST_START("putstring_null_pointer");

    CLEAR_ERRNO();
    int ret = PutString((char*)0, 10);

    ASSERT_ERROR(ret, "NULL pointer devrait echouer");
    ASSERT_ERRNO(E_FAULT, "errno devrait etre E_FAULT");

    TEST_PASS();
}

/* ============================================================
 * TESTS PUTINT - CAS NOMINAUX
 * ============================================================ */

/**
 * @brief Test PutInt avec zéro
 */
static void test_putint_zero(void) {
    TEST_START("putint_zero");

    PutInt(0);
    PutChar('\n');

    /* Attendu: "0\n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec entier positif simple
 */
static void test_putint_positif_simple(void) {
    TEST_START("putint_positif_simple");

    PutInt(42);
    PutChar('\n');

    /* Attendu: "42\n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec entier négatif simple
 */
static void test_putint_negatif_simple(void) {
    TEST_START("putint_negatif_simple");

    PutInt(-42);
    PutChar('\n');

    /* Attendu: "-42\n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec INT_MAX
 */
static void test_putint_int_max(void) {
    TEST_START("putint_int_max");

    PutInt(2147483647);
    PutChar('\n');

    /* Attendu: "2147483647\n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec INT_MIN
 */
static void test_putint_int_min(void) {
    TEST_START("putint_int_min");

    PutInt(-2147483648);
    PutChar('\n');

    /* Attendu: "-2147483648\n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec plusieurs valeurs consécutives
 */
static void test_putint_consecutif(void) {
    TEST_START("putint_consecutif");

    for (int i = 0; i < 5; i++) {
        PutInt(i);
        PutChar(' ');
    }
    PutChar('\n');

    /* Attendu: "0 1 2 3 4 \n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec puissances de 10
 */
static void test_putint_puissances_10(void) {
    TEST_START("putint_puissances_10");

    int val = 1;
    for (int i = 0; i < 10; i++) {
        PutInt(val);
        PutChar(' ');
        val *= 10;
    }
    PutChar('\n');

    /* Attendu: "1 10 100 1000 ... 1000000000 \n" */
    TEST_PASS();
}

/**
 * @brief Test PutInt avec valeurs négatives consécutives
 */
static void test_putint_negatifs_consecutifs(void) {
    TEST_START("putint_negatifs_consecutifs");

    for (int i = -5; i <= 0; i++) {
        PutInt(i);
        PutChar(' ');
    }
    PutChar('\n');

    /* Attendu: "-5 -4 -3 -2 -1 0 \n" */
    TEST_PASS();
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(void) {
    TEST_SUITE_START("Syscalls I/O Output");

    TEST_SECTION("PutChar - Cas nominaux");
    test_putchar_ascii_standard();
    test_putchar_special();
    test_putchar_limites();
    test_putchar_digits();

    TEST_SECTION("PutString - Cas nominaux");
    test_putstring_nominal();
    test_putstring_n_superieur_strlen();
    test_putstring_n_egal_strlen();
    test_putstring_chaine_vide();
    test_putstring_special_chars();
    test_putstring_max_string_size();
    test_putstring_proche_max_put_string();

    TEST_SECTION("PutString - Cas d'erreur");
    test_putstring_n_zero();
    test_putstring_n_negatif();
    test_putstring_n_tres_negatif();
    test_putstring_null_pointer();

    TEST_SECTION("PutInt - Cas nominaux");
    test_putint_zero();
    test_putint_positif_simple();
    test_putint_negatif_simple();
    test_putint_int_max();
    test_putint_int_min();
    test_putint_consecutif();
    test_putint_puissances_10();
    test_putint_negatifs_consecutifs();

    TEST_SUITE_END();

    return 0;
}