#include "nos_mem.h"
#include "nos_stdlib.h"
#include "nos_stddef.h"
#include "nos_mem_macros.h"
#include "types.h"
#include "syscall.h"

#define EXPECT_EQ(a,b) \
    if ((a) != (b)){   \
        printf_simple("ERREUR sur comparaiso\n"); \
        Halt();        \
    } else{  \
        printf_simple("ça marche :) !\n"); \
    }

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

int main(){
    mem_init(800);

    void * occupied[800]; 
    occupied[0] = mem_alloc(450);
    printf_simple("hey\n");
    int i = 1;
    while ( (occupied[i] = mem_alloc(MIN_BUSY_SIZE - 1)) != NULL){
        i++;
    }
    for (int j = 1; j < i; j++){
        printf_simple("foo\n");
        EXPECT_EQ(NULL, mem_realloc(occupied[j], MIN_BUSY_SIZE * 3));
        printf_simple("bar\n");
    }
    printf_simple("hey2\n");

    EXPECT_EQ(true, i > 16);
    mem_free(occupied[0]);
    printf_simple("1\n");
    EXPECT_EQ(occupied[0], mem_realloc(occupied[3], 450)); // relocalisation
    printf_simple("2\n");
    EXPECT_EQ(occupied[3], mem_alloc(MIN_BUSY_SIZE));
    mem_free(occupied[6]);
    printf_simple("3\n");
//     mem_show(print_block);
//     EXPECT_EQ(occupied[6], mem_realloc(occupied[7], MIN_BUSY_SIZE - 1 + MIN_BUSY_SIZE - 1 )); // on peut fusionner avec l'arrère pour gagner de la place
//
//     mem_free(occupied[9]);
//     printf_simple("4\n");
//     EXPECT_EQ(occupied[8], mem_realloc(occupied[8], MIN_BUSY_SIZE * 2 - 2 + BUSY_BLOCK_TOTAL(0) )); // on peut fusionner avec l'avant pour gagner de la place
//
//
//     mem_free(occupied[13]);
//     mem_free(occupied[15]);
//     printf_simple("5\n");
//     EXPECT_EQ(occupied[13], mem_realloc(occupied[14], MIN_BUSY_SIZE * 3 - 3 + BUSY_BLOCK_TOTAL(0) * 2 )); // on peut fusionner avec l'avant et l'arrière pour gagner de la place
//
}
