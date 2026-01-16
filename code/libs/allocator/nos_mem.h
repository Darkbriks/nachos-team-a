#ifndef NOS_MEM_H
#define NOS_MEM_H

#include "types.h"

typedef struct mem_init_block_s mem_init_block_t;
typedef struct mem_header_s mem_header_t;
typedef struct mem_block_s mem_block_t;
typedef struct mem_footer_s mem_footer_t;
typedef mem_block_t mem_free_block_t;

typedef mem_free_block_t* (mem_fit_function_t)(mem_free_block_t*, size_t);

// Bloc initial
struct mem_init_block_s {
    mem_block_t *first;
    mem_fit_function_t *alloc;
};

// Header d'un bloc
struct mem_header_s {
    size_t size_and_flags; // Taille du bloc + flag d'etat sur le bit de poids faible
};

// Informations pour les blocs libres
struct free_info_s {
    struct mem_block_s *prev;
    struct mem_block_s *next;
};

// Bloc mémoire
struct mem_block_s {
    struct mem_header_s header; // Header du bloc

    union {
        struct free_info_s free_info; // si le bloc est libre
        char user_data[0]; // si le bloc est alloué
    };
};

// Footer d'un bloc (placé à la fin des données utilisateur)
struct mem_footer_s {
    size_t size_and_flags; // Copie de size_and_flags du header
};

/*------------------------------------------------------------------------------
 * Interface utilisateur
 *------------------------------------------------------------------------------*/

/**
 * @brief Initialize the memory allocator
 *
 * Must be called before any allocation.
 * Uses mem_space_init() to obtain memory from the system.
 *
 * @param size Initial heap size in bytes (minimum)
 * @return Pointer to usable memory start, or NULL on failure
 */
void* mem_init(size_t size);

/**
 * @brief Allocate a block of memory
 * @param size Size in bytes
 * @return Pointer to allocated memory, or NULL if out of memory
 */
void* mem_alloc(size_t size);

/**
 * @brief Free a previously allocated block
 * @param ptr Pointer returned by mem_alloc
 */
void mem_free(void* ptr);

/**
 * @brief Get the size of an allocated block
 * @param ptr Pointer returned by mem_alloc
 * @return Usable size of the block
 */
size_t mem_get_size(void* ptr);

/**
 * @brief Reallocate a block with a new size
 * @param ptr Pointer to reallocate (can be NULL)
 * @param size New size
 * @return New pointer, or NULL on failure
 */
void* mem_realloc(void* ptr, size_t size);

/**
 * @brief Allocate and zero-initialize memory
 * @param nmemb Number of elements
 * @param size Size of each element
 * @return Pointer to zeroed memory, or NULL on failure
 */
void* mem_calloc(size_t nmemb, size_t size);

/*------------------------------------------------------------------------------
 * Configuration
 *------------------------------------------------------------------------------*/

/**
 * @brief Set the allocation strategy
 * @param fit Function to use for block selection
 */
void mem_set_fit_handler(mem_fit_function_t* fit);

mem_fit_function_t mem_first_fit;
mem_fit_function_t mem_best_fit;
mem_fit_function_t mem_worst_fit;

/*------------------------------------------------------------------------------
 * Debug
 *------------------------------------------------------------------------------*/

/**
 * @brief Iterate over all blocks and call print function
 * @param print Callback: (address, size, is_free)
 */
void mem_show(void (*print)(void*, size_t, int));

#endif