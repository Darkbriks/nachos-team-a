#ifndef NOS_MEM_SPACE_H
#define NOS_MEM_SPACE_H

/**
 * @brief Initialize the memory space with the given size
 *
 * This function must be called before any other mem_* functions.
 * It allocates the initial memory space using Sbrk().
 *
 * @param size Minimum size in bytes (will be rounded up to page boundary)
 * @return Pointer to the start of the memory space, or NULL on failure
 */
void* mem_space_init(unsigned int size);

/**
 * @brief Get the base address of the memory space
 * @return Base address, or NULL if not initialized
 */
void* mem_space_get_addr();

/**
 * @brief Get the current size of the memory space
 * @return Size in bytes
 */
unsigned int mem_space_get_size();

/**
 * @brief Extend the memory space by the given number of bytes
 *
 * This function uses Sbrk() to request more pages from the kernel.
 * The actual extension will be rounded up to page boundary.
 *
 * @param additional_size Number of additional bytes needed
 * @return 0 on success, -1 on failure
 */
int mem_space_extend(unsigned int additional_size);

#endif