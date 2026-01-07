#include "syscall.h"
#include "nos_mem_space.h"

#define PAGE_SIZE 128

static void* mem_base = (void*)0;
static unsigned int mem_size = 0;
static int initialized = 0;

static unsigned int round_up_to_page(unsigned int size) {
    return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

void* mem_space_init(unsigned int size) {
    if (initialized) { return mem_base; }

    unsigned int rounded_size = round_up_to_page(size);
    unsigned int pages_needed = rounded_size / PAGE_SIZE;

    if (pages_needed == 0) { pages_needed = 1; }

    int result = Sbrk((int)pages_needed);

    if (result == -1) { return (void*)0; }

    mem_base = (void*)result;
    mem_size = pages_needed * PAGE_SIZE;
    initialized = 1;

    return mem_base;
}

void* mem_space_get_addr(void) { return mem_base; }

unsigned int mem_space_get_size(void) { return mem_size; }

int mem_space_extend(unsigned int additional_size) {
    if (!initialized) { return -1; }

    unsigned int rounded_size = round_up_to_page(additional_size);
    unsigned int pages_needed = rounded_size / PAGE_SIZE;

    if (pages_needed == 0) { return 0; }

    int result = Sbrk((int)pages_needed);

    if (result == -1) { return -1; }

    mem_size += pages_needed * PAGE_SIZE;

    return 0;
}