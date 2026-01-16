#include "syscall.h"
#include "nos_mem.h"

#define INITIAL_HEAP_SIZE 512

void print_block(void* addr, unsigned int size, int is_free) {
    PutString("  Block at ", 12);
    PutInt((int)addr);
    PutString(", size=", 7);
    PutInt((int)size);
    if (is_free) {
        PutString(" [FREE]\n", 8);
    } else {
        PutString(" [USED]\n", 8);
    }
}

int main() {
    PutString("=== Test allocateur memoire NachOS ===\n", 40);

    PutString("--- Test 0: Initialisation de l'allocateur ---\n", 42);
    void* base = mem_init(INITIAL_HEAP_SIZE);

    if (base == (void*)0) {
        PutString("ERREUR: mem_init a echoue!\n", 28);
        return 1;
    }

    PutString("Allocateur initialise a l'adresse: ", 36);
    PutInt((int)base);
    PutChar('\n');

    PutString("\n--- Test 0: Etat initial memoire ---\n", 34);
    mem_show(print_block);

    PutString("\n--- Test 1: Allocation simple ---\n", 36);

    void* ptr1 = mem_alloc(32);
    if (ptr1 == (void*)0) {
        PutString("ERREUR: Allocation de 32 bytes echouee\n", 40);
        return 1;
    }
    PutString("Alloue 32 bytes a: ", 19);
    PutInt((int)ptr1);
    PutChar('\n');

    void* ptr2 = mem_alloc(64);
    if (ptr2 == (void*)0) {
        PutString("ERREUR: Allocation de 64 bytes echouee\n", 40);
        return 1;
    }
    PutString("Alloue 64 bytes a: ", 19);
    PutInt((int)ptr2);
    PutChar('\n');

    void* ptr3 = mem_alloc(16);
    if (ptr3 == (void*)0) {
        PutString("ERREUR: Allocation de 16 bytes echouee\n", 40);
        return 1;
    }
    PutString("Alloue 16 bytes a: ", 19);
    PutInt((int)ptr3);
    PutChar('\n');

    PutString("\n--- Test 1: Etat memoire apres allocations ---\n", 45);
    mem_show(print_block);

    PutString("\n--- Test 2: Ecriture/Lecture ---\n", 35);

    char* str = (char*)ptr1;
    str[0] = 'H';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = '\0';

    PutString("Ecrit 'Hello' dans ptr1: ", 25);
    PutString(str, 6);
    PutChar('\n');

    PutString("\n--- Test 2: Etat memoire apres ecriture ---\n", 46);
    mem_show(print_block);

    PutString("\n--- Test 3: mem_get_size ---\n", 31);

    PutString("Taille de ptr1: ", 16);
    PutInt((int)mem_get_size(ptr1));
    PutString(" (attendu 32)\n", 17);

    PutString("Taille de ptr2: ", 16);
    PutInt((int)mem_get_size(ptr2));
    PutString(" (attendu 64)\n", 17);

    PutString("\n--- Test 3: Etat memoire apres get_size ---\n", 44);
    mem_show(print_block);

    PutString("\n--- Test 4: Liberation ---\n", 29);

    PutString("Liberation de ptr2...\n", 22);
    mem_free(ptr2);

    PutString("\n--- Test 4: Etat memoire apres liberation ---\n", 45);
    mem_show(print_block);

    PutString("\n--- Test 5: Reallocation du trou ---\n", 39);

    void* ptr4 = mem_alloc(32);
    if (ptr4 == (void*)0) {
        PutString("ERREUR: Allocation de 32 bytes echouee\n", 40);
        return 1;
    }
    PutString("Alloue 32 bytes a: ", 19);
    PutInt((int)ptr4);
    PutChar('\n');

    PutString("\n--- Test 5: Etat memoire apres realloc ---\n", 44);
    mem_show(print_block);

    PutString("\n--- Test 6: mem_calloc ---\n", 29);

    int* arr = (int*)mem_calloc(10, sizeof(int));
    if (arr == (void*)0) {
        PutString("ERREUR: calloc echoue\n", 22);
        return 1;
    }

    PutString("Calloc 10 ints a: ", 18);
    PutInt((int)arr);
    PutString("\nVerification zeros: ", 21);

    int all_zero = 1;
    for (int i = 0; i < 10; i++) {
        if (arr[i] != 0) {
            all_zero = 0;
            break;
        }
    }

    if (all_zero) {
        PutString("OK\n", 3);
    } else {
        PutString("ERREUR\n", 7);
    }

    PutString("\n--- Test 6: Etat memoire apres calloc ---\n", 43);
    mem_show(print_block);

    PutString("\n--- Nettoyage ---\n", 19);
    mem_free(ptr1);
    mem_free(ptr3);
    mem_free(ptr4);
    mem_free(arr);

    PutString("\nEtat apres liberation totale:\n", 32);
    mem_show(print_block);

    PutString("\n=== Tests termines avec succes ===\n", 37);

    return 0;
}