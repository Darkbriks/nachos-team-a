#include "nos_mem.h"
#include "nos_mem_macros.h"
#include "nos_mem_space.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "nos_stddef.h"
#include "syscall.h"
#include "nos_stdio.h"

static int allocator_initialized = 0;

static int is_valid_addr(const void* addr);
static int validate_block(mem_block_t* block);
static void set_footer(mem_block_t* block);
static void remove_from_free_list(mem_block_t* block);
static void insert_into_free_list(mem_block_t* new_block);
static int coalesce_blocks(mem_block_t* first, mem_block_t* second);
static mem_block_t* get_block_from_footer(mem_footer_t* footer);

#define VERBOSE_ALLOCATOR 0
#define DEBUG_ALLOCATOR 1

#define PRINT_STRING(str)        \
    do {                         \
        if (VERBOSE_ALLOCATOR) { \
            printf(str);  \
        }                        \
    } while (0)

#define PRINT_INT(val)           \
    do {                         \
        if (VERBOSE_ALLOCATOR) { \
            PutInt(val);         \
        }                        \
    } while (0)

// Print string, int, string, the most common pattern
#define PRINT_SIS(s1, i1, s2) \
    do {                             \
        if (VERBOSE_ALLOCATOR) {     \
            printf(s1);       \
            PutInt(i1);              \
            printf(s2);       \
        }                            \
    } while (0)

#define MEM_ASSERT(cond, msg, retval) \
    do {                              \
        if (!(cond)) {                \
            if (DEBUG_ALLOCATOR) {    \
                printf(msg);   \
            }                         \
            return retval;            \
        }                             \
    } while (0)

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
void* mem_init(size_t size) {
    size_t total_needed = INIT_BLOCK_SIZE + FREE_BLOCK_TOTAL(0) + size;

    void* space = mem_space_init((unsigned int)total_needed);
    MEM_ASSERT(space != NULL, "mem_init: mem_space_init failed\n", NULL);

    size_t total_size = mem_space_get_size();
    MEM_ASSERT(total_size >= total_needed, "mem_init: Insufficient memory from mem_space_init\n", NULL);

    // Placer le bloc initial
    mem_init_block_t* init_block = (mem_init_block_t*)space;

    // Calculer l'espace disponible pour les allocations
    char* allocatable_start = (char*)space + INIT_BLOCK_SIZE;
    size_t available_size = total_size - INIT_BLOCK_SIZE;
    size_t aligned_size = ALIGN_FLOOR(available_size);
    MEM_ASSERT(aligned_size >= FREE_BLOCK_TOTAL(0), "mem_init: Not enough space for initial free block\n", NULL);

    // Créer le premier bloc libre
    mem_block_t* first_free = (mem_block_t*)allocatable_start;
    size_t user_size = FREE_USER_SIZE(aligned_size);

    first_free->header.size_and_flags = user_size | FREE_FLAG;
    first_free->free_info.prev = NULL;
    first_free->free_info.next = NULL;

	// Placer le footer
    set_footer(first_free);

	// Configurer le bloc initial
    init_block->first = first_free;
    init_block->alloc = mem_first_fit;

    allocator_initialized = 1;

    PRINT_SIS("mem_init: Espace mémoire initialisé de ", (int)total_size, " bytes\n");
    PRINT_SIS("mem_init: Espace utilisable de ", (int)available_size, " bytes\n");

    PRINT_SIS("mem_init: Espace utilisable aligné de ", (int)aligned_size, " bytes");
    PRINT_SIS(" (aligné à ", (int)ALIGNMENT, ")\n");

    PRINT_SIS("mem_init: Premier bloc libre à ", (int)first_free, ",");
    PRINT_SIS(" taille=", (int)get_size(first_free), "\n");

    PRINT_SIS("mem_init: Taille de l'header : ", (int)HEADER_SIZE, "\n");
    PRINT_SIS("mem_init: Taille du footer : ", (int)FOOTER_SIZE, "\n");
    PRINT_SIS("mem_init: Taille totale d'un bloc libre vide : ", (int)FREE_BLOCK_TOTAL(0), "\n");
    PRINT_SIS("mem_init: Taille totale d'un bloc occupé vide : ", (int)BUSY_BLOCK_TOTAL(0), "\n");

    return space;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void* mem_alloc(size_t size) {
    MEM_ASSERT(size > 0, "mem_alloc: Requested size is zero\n", NULL);
    MEM_ASSERT(allocator_initialized, "mem_alloc: Allocator not initialized\n", NULL);

    // Aligner la taille demandée
    size_t aligned_user_size = ALIGN(size);

    // S'assurer que le bloc peut être converti en bloc libre plus tard
    size_t min_user_size = FREE_OVERHEAD - BUSY_OVERHEAD;
    size_t final_user_size = MAX(aligned_user_size, min_user_size);

    PRINT_SIS("mem_alloc: Demande d'allocation de ", (int)size, " bytes\n");
    PRINT_SIS("mem_alloc: Taille alignée à ", (int)aligned_user_size, " bytes\n");
    PRINT_SIS("mem_alloc: Taille finale du bloc utilisateur : ", (int)final_user_size, " bytes\n");

    // Calculer la taille totale nécessaire
    size_t needed_total_size = BUSY_BLOCK_TOTAL(final_user_size);

    // Récupérer la stratégie d'allocation
    mem_init_block_t* init = (mem_init_block_t*)mem_space_get_addr();
    mem_fit_function_t* fit_strategy = init->alloc;

    if (fit_strategy != mem_first_fit && fit_strategy != mem_best_fit && fit_strategy != mem_worst_fit){
        PRINT_STRING("mem_alloc : Fit function has changed not allow !!\n");
        fit_strategy = mem_first_fit;
        init->alloc = fit_strategy;
    }
    // Chercher un bloc libre approprié
    mem_block_t* chosen_block = fit_strategy(init->first, final_user_size);

    if (chosen_block == (mem_block_t*)0) {
        // TODO: implémenter l'extension automatique de l'espace mémoire
        PRINT_STRING("mem_alloc: Aucune mémoire libre disponible pour l'allocation\n");
        return NULL;
    }

	// Retirer le bloc de la liste des libres
    remove_from_free_list(chosen_block);

    size_t block_user_size = get_size(chosen_block);
    size_t block_total_size = FREE_BLOCK_TOTAL(block_user_size);
    size_t remaining_size = block_total_size - needed_total_size;

	// Vérifier si on peut partitionner
    if (remaining_size >= FREE_BLOCK_TOTAL(0)) {
        PRINT_SIS("mem_alloc: Partitionnement du bloc libre de taille ", (int)block_total_size, " bytes\n");
        PRINT_SIS("mem_alloc: Nouveau bloc occupé de taille ", (int)needed_total_size, " bytes\n");
        PRINT_SIS("mem_alloc: Bloc résiduel libre de taille ", (int)remaining_size, " bytes\n");

		// Ajuster le bloc occupé
        chosen_block->header.size_and_flags = final_user_size;
        set_used(chosen_block);
        set_footer(chosen_block);

		// Créer le bloc résiduel juste après le bloc occupé
        char* remainder_addr = (char*)chosen_block + needed_total_size;
        mem_block_t* remainder = (mem_block_t*)remainder_addr;

        size_t remainder_user_size = FREE_USER_SIZE(remaining_size);
        remainder->header.size_and_flags = remainder_user_size | FREE_FLAG;
        remainder->free_info.prev = (mem_block_t*)0;
        remainder->free_info.next = (mem_block_t*)0;

        set_footer(remainder);
        insert_into_free_list(remainder);
    } else {
		// Pas assez de place pour un nouveau bloc -> donner tout le bloc
        size_t occupied_user_size = BUSY_USER_SIZE(block_total_size);
        size_t safe_user_size = MAX(occupied_user_size, min_user_size);

        PRINT_SIS("mem_alloc: Pas de partitionnement (résidu trop petit: ", (int)remaining_size, " bytes)\n");
        PRINT_SIS("mem_alloc: Conversion libre -> occupé, taille du bloc : ", (int)block_total_size, " bytes, ");
        PRINT_SIS("taille utilisateur : ", (int)safe_user_size, " bytes\n");

        chosen_block->header.size_and_flags = safe_user_size;
        set_used(chosen_block);
        set_footer(chosen_block);
    }

    return (char*)chosen_block + BUSY_OVERHEAD;
}

//-------------------------------------------------------------
// mem_calloc
//-------------------------------------------------------------
void* mem_calloc(size_t nmemb, size_t size) {
    MEM_ASSERT(nmemb > 0, "mem_calloc: Number of members is zero\n", NULL);
    MEM_ASSERT(size > 0, "mem_calloc: Size of each member is zero\n", NULL);

    size_t total_size = nmemb * size;
    PRINT_SIS("mem_calloc: Demande d'allocation de ", (int)nmemb, " membres de taille ");
    PRINT_SIS("", (int)size, " bytes (total ");
    PRINT_SIS("", (int)total_size, " bytes)\n");

    MEM_ASSERT(total_size / nmemb == size, "mem_calloc: Size overflow detected\n", NULL);

    void* ptr = mem_alloc(total_size);
    if (ptr) { char* p = ptr; for (size_t i = 0; i < total_size; i++) { p[i] = 0; } }
    return ptr;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void* ptr) {
    MEM_ASSERT(ptr, "mem_get_size: Pointer is NULL\n", 0);
    MEM_ASSERT(allocator_initialized, "mem_get_size: Allocator not initialized\n", 0);

    mem_block_t* block = (mem_block_t*)((char*)ptr - BUSY_OVERHEAD);
    MEM_ASSERT(validate_block(block), "mem_get_size: Invalid block pointer\n", 0);
    MEM_ASSERT(!is_free(block), "mem_get_size: Block is free\n", 0);

    return get_size(block);
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
void mem_free(void* ptr) {
    MEM_ASSERT(ptr, "mem_free: Pointer is NULL\n", );
    MEM_ASSERT(allocator_initialized, "mem_free: Allocator not initialized\n", );

    mem_block_t* block = (mem_block_t*)((char*)ptr - BUSY_OVERHEAD);

    MEM_ASSERT(validate_block(block), "mem_free: Invalid block pointer\n", );
    MEM_ASSERT(!is_free(block), "mem_free: Block is already free\n", );

    PRINT_SIS("mem_free: Libération du bloc à ", (int)block, ", taille=");
    PRINT_SIS("", (int)get_size(block), " bytes\n");

    size_t block_size = BUSY_BLOCK_TOTAL(get_size(block));

    // Marquer le bloc comme libre
    block->header.size_and_flags = FREE_USER_SIZE(block_size) | FREE_FLAG;
    block->free_info.prev = (mem_block_t*)0;
    block->free_info.next = (mem_block_t*)0;
    set_footer(block);

	// Si le bloc précédent est libre, fusionner avec lui
    mem_footer_t* prev_footer = (mem_footer_t*)((char*)block - FOOTER_SIZE);
    if (is_valid_addr(prev_footer)) {
        mem_block_t* prev_block = get_block_from_footer(prev_footer);
        if (prev_block != (mem_block_t*)0 && is_free(prev_block)) {
            PRINT_SIS("mem_free: Fusion avec le bloc précédent à ", (int)prev_block, ", taille=");
            PRINT_SIS("", (int)FREE_BLOCK_TOTAL(get_size(prev_block)), " bytes\n");
            remove_from_free_list(prev_block);
            coalesce_blocks(prev_block, block);
            block = prev_block; // Le bloc résultant est à l'adresse du bloc précédent
        }
    }

	// Si le bloc suivant est libre, fusionner avec lui
    char* next_header = (char*)block + FREE_BLOCK_TOTAL(get_size(block));
    if (is_valid_addr(next_header)) {
        mem_block_t* next_block = (mem_block_t*)next_header;
        if (validate_block(next_block) && is_free(next_block)) {
            PRINT_SIS("mem_free: Fusion avec le bloc suivant à ", (int)next_block, ", taille=");
            PRINT_SIS("", (int)FREE_BLOCK_TOTAL(get_size(next_block)), " bytes\n");
            remove_from_free_list(next_block);
            coalesce_blocks(block, next_block);
        }
    }

    insert_into_free_list(block);
    PRINT_SIS("mem_free: Bloc libre inséré à ", (int)block, ", taille=");
    PRINT_SIS("", (int)FREE_BLOCK_TOTAL(get_size(block)), " bytes\n");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void*, size_t, int free)) {
    MEM_ASSERT(allocator_initialized, "mem_show: Allocator not initialized\n", );

    void* space = mem_space_get_addr();
    size_t total_size = mem_space_get_size();

    MEM_ASSERT(space != NULL, "mem_show: mem_space_get_addr returned NULL\n", );
    MEM_ASSERT(print != NULL, "mem_show: print function is NULL\n", );

    char* current = (char*)space + INIT_BLOCK_SIZE;
    char* end = (char*)space + total_size;

    while (current < end) {
        mem_block_t* block = (mem_block_t*)current;
        MEM_ASSERT(is_valid_addr(block), "mem_show: Invalid block address encountered\n", );

        size_t user_size = get_size(block);
        int is_block_free = is_free(block);

        void* user_addr;
        size_t block_total_size;

        if (is_block_free) {
            user_addr = (char*)block + FREE_OVERHEAD;
            block_total_size = FREE_BLOCK_TOTAL(user_size);
        } else {
            user_addr = (char*)block + BUSY_OVERHEAD;
            block_total_size = BUSY_BLOCK_TOTAL(user_size);
        }

        print(user_addr, user_size, is_block_free);
        current += block_total_size;
    }
    MEM_ASSERT(current == end, "mem_show: Memory traversal did not end correctly\n", );
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t* fit) {
    MEM_ASSERT(allocator_initialized, "mem_set_fit_handler: Allocator not initialized\n", );
    MEM_ASSERT(fit != NULL, "mem_set_fit_handler: Fit function is NULL\n", );
    MEM_ASSERT(fit == mem_first_fit || fit == mem_best_fit || fit == mem_worst_fit, "mem_set_fit_handler: Invalid fit function provided\n", );

    ((mem_init_block_t*)mem_space_get_addr())->alloc = fit;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t* mem_first_fit(mem_free_block_t* first_free, size_t wanted_size) {
    mem_free_block_t* cur = first_free;
    while (cur) {
        if (FREE_BLOCK_TOTAL(get_size(cur)) >= BUSY_BLOCK_TOTAL(wanted_size)) {
            return cur;
        }
        cur = cur->free_info.next;
    }
    return NULL;
}

mem_free_block_t* mem_best_fit(mem_free_block_t* first_free, size_t wanted_size) {
    mem_free_block_t* cur = first_free;
    mem_free_block_t* best = (mem_free_block_t*)0;
    size_t best_size = (size_t)-1;
    while (cur) {
        size_t cur_size = get_size(cur);
        if (FREE_BLOCK_TOTAL(cur_size) >= BUSY_BLOCK_TOTAL(wanted_size)) {
            if (FREE_BLOCK_TOTAL(cur_size) == BUSY_BLOCK_TOTAL(wanted_size)) {
                return cur;
            }
            if (cur_size < best_size) {
                best_size = cur_size;
                best = cur;
            }
        }
        cur = cur->free_info.next;
    }
    return best;
}

mem_free_block_t* mem_worst_fit(mem_free_block_t* first_free, size_t wanted_size) {
    mem_free_block_t* cur = first_free;
    mem_free_block_t* worst = (mem_free_block_t*)0;
    size_t max_size = 0;
    while (cur) {
        size_t cur_size = get_size(cur);
        if (FREE_BLOCK_TOTAL(cur_size) >= BUSY_BLOCK_TOTAL(wanted_size)) {
            if (cur_size > max_size) {
                max_size = cur_size;
                worst = cur;
            }
        }
        cur = cur->free_info.next;
    }
    return worst;
}

//-------------------------------------------------------------
// mem_realloc
//-------------------------------------------------------------
void* mem_realloc(void* ptr, size_t size) {
    MEM_ASSERT(allocator_initialized, "mem_realloc: Allocator not initialized\n", NULL);

    if (ptr == NULL && size != 0) { return mem_alloc(size); }
    if (ptr != NULL && size == 0) { mem_free(ptr); return NULL; }
    if (ptr == NULL && size == 0) { return NULL; }

    mem_block_t* actual_block = (mem_block_t*)((char*)ptr - BUSY_OVERHEAD);
    MEM_ASSERT(validate_block(actual_block), "mem_realloc: Invalid block pointer\n", NULL);
    MEM_ASSERT(!is_free(actual_block), "mem_realloc: Block is free\n", NULL);

    size_t actual_size = get_size(actual_block);
    size_t used_size = mem_get_size(ptr);
    if (actual_size >= size) { // Taille déjà assez grande
        PRINT_SIS("mem_realloc: Taille actuelle suffisante (", (int)actual_size, " bytes)\n");
        return ptr;
    }

    mem_block_t* next_block = (mem_block_t*)((char*)actual_block + BUSY_BLOCK_TOTAL(actual_size));
    MEM_ASSERT(validate_block(next_block), "mem_realloc: Next block is corrupted\n", NULL);

    if (is_free(next_block) && FREE_BLOCK_TOTAL(get_size(next_block)) + actual_size >= size) {
        PRINT_SIS("mem_realloc: Expansion dans le bloc libre adjacent à ", (int)next_block, "\n");

        // Place suffisante pour réallouer sur place
        remove_from_free_list(next_block);
        size_t new_total = BUSY_BLOCK_TOTAL(size);
        size_t available_total = BUSY_BLOCK_TOTAL(get_size(actual_block)) + FREE_BLOCK_TOTAL(get_size(next_block));
        size_t remaining_size = available_total - new_total;

        // Mettre à jour le bloc courant
        actual_block->header.size_and_flags = ALIGN(size);
        set_used(actual_block);
        set_footer(actual_block);

        if (remaining_size >= FREE_BLOCK_TOTAL(0)) {
            mem_block_t *remainder = (mem_block_t*)((char*)actual_block + new_total);
            remainder->header.size_and_flags = FREE_USER_SIZE(remaining_size) | FREE_FLAG;
            remainder->free_info.prev = NULL;
            remainder->free_info.next = NULL;
            set_footer(remainder);
            insert_into_free_list(remainder);
        }
        return ptr;
    }

    PRINT_SIS("mem_realloc: Allocation d'un nouveau bloc pour la taille ", (int)size, " bytes\n");
    mem_block_t *chosen_block = (mem_block_t*)mem_alloc(size);
    MEM_ASSERT(chosen_block != NULL, "mem_realloc: Unable to allocate new block\n", NULL);
    memcpy(chosen_block, ptr, used_size);
    mem_free(ptr);

    return chosen_block;
}

static int is_valid_addr(const void* addr) {
    if (addr == NULL) { return 0; }

    void* space = mem_space_get_addr();
    size_t total_size = mem_space_get_size();

    char* start = (char*)space + INIT_BLOCK_SIZE;
    char* end = (char*)space + ALIGN_FLOOR(total_size);

    return (addr >= (void*)start && addr < (void*)end);
}

static int validate_block(mem_block_t* block) {
    if (!is_valid_addr(block)) { return 0; }

    mem_footer_t* footer;

    if (is_free(block)) { footer = (mem_footer_t*)((char*)block + FREE_OVERHEAD + get_size(block)); }
    else { footer = (mem_footer_t*)((char*)block + BUSY_OVERHEAD + get_size(block)); }

    if (!is_valid_addr(footer)) { return 0; }

    return footer->size_and_flags == block->header.size_and_flags;
}

static void set_footer(mem_block_t* block) {
    mem_footer_t* footer;

    if (is_free(block)) { footer = (mem_footer_t*)((char*)block + FREE_OVERHEAD + get_size(block)); }
    else{ footer = (mem_footer_t*)((char*)block + BUSY_OVERHEAD + get_size(block)); }

    footer->size_and_flags = block->header.size_and_flags;
}

static void remove_from_free_list(mem_block_t* block) {
    mem_init_block_t* init = (mem_init_block_t*)mem_space_get_addr();

    if (block->free_info.prev != (mem_block_t*)0) { block->free_info.prev->free_info.next = block->free_info.next; }
    else { init->first = block->free_info.next; }

    if (block->free_info.next != (mem_block_t*)0) { block->free_info.next->free_info.prev = block->free_info.prev; }

    block->free_info.prev = (mem_block_t*)0;
    block->free_info.next = (mem_block_t*)0;
}

static void insert_into_free_list(mem_block_t* new_block) {
    mem_init_block_t* init = (mem_init_block_t*)mem_space_get_addr();
    mem_block_t* current = init->first;
    mem_block_t* prev = (mem_block_t*)0;

    while (current != (mem_block_t*)0 && current < new_block) {
        prev = current;
        current = current->free_info.next;
    }

    new_block->free_info.prev = prev;
    new_block->free_info.next = current;

    if (prev != (mem_block_t*)0) { prev->free_info.next = new_block; }
    else { init->first = new_block; }

    if (current != (mem_block_t*)0) { current->free_info.prev = new_block; }

    set_footer(new_block);
}

static int coalesce_blocks(mem_block_t* first, mem_block_t* second) {
    size_t first_total = is_free(first) ? FREE_BLOCK_TOTAL(get_size(first)) : BUSY_BLOCK_TOTAL(get_size(first));
    size_t second_total = is_free(second) ? FREE_BLOCK_TOTAL(get_size(second)) : BUSY_BLOCK_TOTAL(get_size(second));

    if ((char*)first + first_total != (char*)second) { return 0; }

    size_t new_total_size = first_total + second_total;
    size_t new_user_size = is_free(first) ? FREE_USER_SIZE(new_total_size) : BUSY_USER_SIZE(new_total_size);

    first->header.size_and_flags = new_user_size | (first->header.size_and_flags & FREE_FLAG);

    set_footer(first);

    return 1;
}

static mem_block_t* get_block_from_footer(mem_footer_t* footer) {
    size_t size_and_flags = footer->size_and_flags;
    size_t user_size = size_and_flags & ~FREE_FLAG;

    size_t block_total_size = is_free_size(size_and_flags) ? FREE_BLOCK_TOTAL(user_size) : BUSY_BLOCK_TOTAL(user_size);

    char* header_addr = (char*)footer - block_total_size + FOOTER_SIZE;

    if (!is_valid_addr(header_addr)) { return NULL; }

    return (mem_block_t*)header_addr;
}
