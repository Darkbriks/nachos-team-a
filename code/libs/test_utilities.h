/**
 * @file test_utilities.h
 * @brief Bibliothèque de test pour NachOS userland
 *
 * Fournit des macros pour structurer les tests unitaires des syscalls.
 *
 * Usage:
 *   TEST_SUITE_START("Nom de la suite");
 *   TEST_SECTION("Catégorie");
 *   test_fonction();
 *   TEST_SUITE_END();
 *
 * Dans chaque fonction de test:
 *   TEST_START("nom_du_test");
 *   ASSERT_*(conditions...);
 *   TEST_PASS();
 */

#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "syscall.h"
#include "nos_errno.h"

/* ============================================================
 * Compteurs globaux
 * ============================================================ */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* ============================================================
 * Helpers internes
 * ============================================================ */

static inline void __test_print(const char* s) {
    int len = 0;
    while (s[len] != '\0' && len < 256) len++;
    PutString((char*)s, len);
}

static inline void __test_print_int(int n) {
    PutInt(n);
}

static inline void __test_newline(void) {
    PutChar('\n');
}

/* ============================================================
 * Macros de structure de suite de tests
 * ============================================================ */

/**
 * @brief Démarre une suite de tests
 * @param name Nom descriptif de la suite
 */
#define TEST_SUITE_START(name) \
    do { \
        __test_print("========================================\n"); \
        __test_print("  SUITE: "); \
        __test_print(name); \
        __test_newline(); \
        __test_print("========================================\n"); \
        tests_run = 0; \
        tests_passed = 0; \
        tests_failed = 0; \
    } while(0)

/**
 * @brief Termine une suite et affiche le résumé
 */
#define TEST_SUITE_END() \
    do { \
        __test_print("----------------------------------------\n"); \
        __test_print("  RESULTAT: "); \
        __test_print_int(tests_passed); \
        __test_print("/"); \
        __test_print_int(tests_run); \
        __test_print(" tests OK"); \
        if (tests_failed > 0) { \
            __test_print(" ("); \
            __test_print_int(tests_failed); \
            __test_print(" ECHECS)"); \
        } \
        __test_newline(); \
        __test_print("========================================\n"); \
    } while(0)

/**
 * @brief Délimite une section de tests
 * @param name Nom de la section
 */
#define TEST_SECTION(name) \
    do { \
        __test_print("\n--- "); \
        __test_print(name); \
        __test_print(" ---\n"); \
    } while(0)

/* ============================================================
 * Macros de test individuel
 * ============================================================ */

/**
 * @brief Démarre un test individuel
 * @param name Nom du test
 */
#define TEST_START(name) \
    do { \
        tests_run++; \
        __test_print("[TEST] "); \
        __test_print(name); \
        __test_print(" ... "); \
    } while(0)

/**
 * @brief Marque le test comme réussi
 */
#define TEST_PASS() \
    do { \
        tests_passed++; \
        __test_print("OK\n"); \
    } while(0)

/**
 * @brief Marque le test comme échoué avec un message
 * @param msg Message d'erreur
 */
#define TEST_FAIL(msg) \
    do { \
        tests_failed++; \
        __test_print("ECHEC: "); \
        __test_print(msg); \
        __test_newline(); \
    } while(0)

/**
 * @brief Marque le test comme échoué avec valeurs attendue/obtenue
 * @param msg Message d'erreur
 * @param expected Valeur attendue
 * @param actual Valeur obtenue
 */
#define TEST_FAIL_VALUES(msg, expected, actual) \
    do { \
        tests_failed++; \
        __test_print("ECHEC: "); \
        __test_print(msg); \
        __test_print(" (attendu="); \
        __test_print_int(expected); \
        __test_print(", obtenu="); \
        __test_print_int(actual); \
        __test_print(")\n"); \
    } while(0)

/**
 * @brief Saute un test (non exécuté)
 * @param reason Raison du skip
 */
#define TEST_SKIP(reason) \
    do { \
        __test_print("SKIP: "); \
        __test_print(reason); \
        __test_newline(); \
    } while(0)

/* ============================================================
 * Macros d'assertion
 * ============================================================ */

/**
 * @brief Vérifie l'égalité de deux valeurs
 * @param actual Valeur obtenue
 * @param expected Valeur attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_EQ(actual, expected, msg) \
    do { \
        int __a = (int)(actual); \
        int __e = (int)(expected); \
        if (__a != __e) { \
            TEST_FAIL_VALUES(msg, __e, __a); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie l'inégalité de deux valeurs
 * @param actual Valeur obtenue
 * @param unexpected Valeur non attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_NEQ(actual, unexpected, msg) \
    do { \
        int __a = (int)(actual); \
        int __u = (int)(unexpected); \
        if (__a == __u) { \
            tests_failed++; \
            __test_print("ECHEC: "); \
            __test_print(msg); \
            __test_print(" (valeur interdite="); \
            __test_print_int(__u); \
            __test_print(")\n"); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est supérieure ou égale à une autre
 * @param actual Valeur obtenue
 * @param expected Valeur attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_GE(actual, expected, msg) \
    do { \
        int __a = (int)(actual); \
        int __e = (int)(expected); \
        if (__a < __e) { \
            TEST_FAIL_VALUES(msg, __e, __a); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est inférieure ou égale à une autre
 * @param actual Valeur obtenue
 * @param expected Valeur attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_LE(actual, expected, msg) \
    do { \
        int __a = (int)(actual); \
        int __e = (int)(expected); \
        if (__a > __e) { \
            TEST_FAIL_VALUES(msg, __e, __a); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est strictement supérieure à une autre
 * @param actual Valeur obtenue
 * @param expected Valeur attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_GT(actual, expected, msg) \
    do { \
        int __a = (int)(actual); \
        int __e = (int)(expected); \
        if (__a <= __e) { \
            TEST_FAIL_VALUES(msg, __e + 1, __a); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est strictement inférieure à une autre
 * @param actual Valeur obtenue
 * @param expected Valeur attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_LT(actual, expected, msg) \
    do { \
        int __a = (int)(actual); \
        int __e = (int)(expected); \
        if (__a >= __e) { \
            TEST_FAIL_VALUES(msg, __e - 1, __a); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une condition est vraie
 * @param cond Condition à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_TRUE(cond, msg) \
    do { \
        if (!(cond)) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une condition est fausse
 * @param cond Condition à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_FALSE(cond, msg) \
    do { \
        if (cond) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie que deux chaînes sont égales
 * @param actual Chaîne obtenue
 * @param expected Chaîne attendue
 * @param msg Message en cas d'échec
 */
#define ASSERT_STREQ(actual, expected, msg) \
    do { \
        const char* __a = (actual); \
        const char* __e = (expected); \
        int __i = 0; \
        while (__a[__i] != '\0' && __e[__i] != '\0') { \
            if (__a[__i] != __e[__i]) { \
                TEST_FAIL(msg); \
                return; \
            } \
            __i++; \
        } \
        if (__a[__i] != __e[__i]) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'un pointeur n'est pas NULL
 * @param ptr Pointeur à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_NOT_NULL(ptr, msg) \
    do { \
        if ((ptr) == (void*)0) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'un pointeur est NULL
 * @param ptr Pointeur à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_NULL(ptr, msg) \
    do { \
        if ((ptr) != (void*)0) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie que errno correspond à la valeur attendue
 * @param expected Code d'erreur attendu (E_INVAL, E_FAULT, etc.)
 * @param msg Message en cas d'échec
 */
#define ASSERT_ERRNO(expected, msg) \
    do { \
        int __err = __get_errno(); \
        int __exp = (expected); \
        if (__err != __exp) { \
            TEST_FAIL_VALUES(msg, __exp, __err); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est dans un intervalle [min, max]
 * @param val Valeur à vérifier
 * @param min Borne inférieure (incluse)
 * @param max Borne supérieure (incluse)
 * @param msg Message en cas d'échec
 */
#define ASSERT_IN_RANGE(val, min, max, msg) \
    do { \
        int __v = (int)(val); \
        int __min = (int)(min); \
        int __max = (int)(max); \
        if (__v < __min || __v > __max) { \
            tests_failed++; \
            __test_print("ECHEC: "); \
            __test_print(msg); \
            __test_print(" (valeur="); \
            __test_print_int(__v); \
            __test_print(", attendu dans ["); \
            __test_print_int(__min); \
            __test_print(","); \
            __test_print_int(__max); \
            __test_print("])\n"); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est strictement positive
 * @param val Valeur à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_POSITIVE(val, msg) \
    do { \
        int __v = (int)(val); \
        if (__v <= 0) { \
            TEST_FAIL_VALUES(msg, 1, __v); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est >= 0
 * @param val Valeur à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_NON_NEGATIVE(val, msg) \
    do { \
        int __v = (int)(val); \
        if (__v < 0) { \
            TEST_FAIL_VALUES(msg " (doit etre >= 0)", 0, __v); \
            return; \
        } \
    } while(0)

/**
 * @brief Vérifie qu'une valeur est négative (indique une erreur syscall)
 * @param val Valeur de retour à vérifier
 * @param msg Message en cas d'échec
 */
#define ASSERT_ERROR(val, msg) \
    do { \
        int __v = (int)(val); \
        if (__v >= 0) { \
            tests_failed++; \
            __test_print("ECHEC: "); \
            __test_print(msg); \
            __test_print(" (attendu erreur, obtenu="); \
            __test_print_int(__v); \
            __test_print(")\n"); \
            return; \
        } \
    } while(0)

/* ============================================================
 * Macros utilitaires
 * ============================================================ */

/**
 * @brief Affiche une information de debug pendant le test
 * @param msg Message à afficher
 */
#define TEST_INFO(msg) \
    do { \
        __test_print("  [INFO] "); \
        __test_print(msg); \
        __test_newline(); \
    } while(0)

/**
 * @brief Affiche une valeur pendant le test
 * @param label Label de la valeur
 * @param val Valeur à afficher
 */
#define TEST_INFO_VALUE(label, val) \
    do { \
        __test_print("  [INFO] "); \
        __test_print(label); \
        __test_print(" = "); \
        __test_print_int((int)(val)); \
        __test_newline(); \
    } while(0)

/**
 * @brief Efface errno avant un test
 */
#define CLEAR_ERRNO() __clear_errno()

#endif /* TEST_UTILITIES_H */