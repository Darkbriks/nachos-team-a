#ifndef MEM_MACROS_NACHOS_H
#define MEM_MACROS_NACHOS_H

// Alignement des blocs (doit être une puissance de 2)
#ifndef ALIGNMENT
#define ALIGNMENT 4
#endif

#if (ALIGNMENT & (ALIGNMENT - 1)) != 0
#error "ALIGNMENT must be a power of 2"
#endif

// Macro d'alignement vers le haut (ALIGN(13) = 16)
#define ALIGN(size)  (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Macro d'alignement vers le bas (ALIGN_FLOOR(13) = 12)
#define ALIGN_FLOOR(size)  ((size) & ~(ALIGNMENT - 1))

// Macros de manipulation des blocs
#define FREE_FLAG 0x1
#define get_size(block) ((block)->header.size_and_flags & ~FREE_FLAG)
#define is_free(block) ((block)->header.size_and_flags & FREE_FLAG)
#define is_free_size(size) ((size) & FREE_FLAG)
#define set_free(block) ((block)->header.size_and_flags |= FREE_FLAG)
#define set_used(block) ((block)->header.size_and_flags &= ~FREE_FLAG)

// Taille commune
#define HEADER_SIZE ALIGN(sizeof(mem_header_t))
#define FOOTER_SIZE ALIGN(sizeof(mem_footer_t))
#define FREE_INFO_SIZE ALIGN(sizeof(struct free_info_s))
#define INIT_BLOCK_SIZE ALIGN(sizeof(mem_init_block_t))

// Overheads
#define FREE_OVERHEAD  (HEADER_SIZE + FREE_INFO_SIZE)
#define BUSY_OVERHEAD  HEADER_SIZE

// Taille totale d’un bloc libre (header + chainage + user + footer)
#define FREE_BLOCK_TOTAL(user_size)  (FREE_OVERHEAD + ALIGN(user_size) + FOOTER_SIZE)

// Taille totale d’un bloc occupé (header + user + footer)
#define BUSY_BLOCK_TOTAL(user_size)  (BUSY_OVERHEAD + ALIGN(user_size) + FOOTER_SIZE)

// Taille utilisable pour l’utilisateur à partir d’un bloc libre de taille X
#define FREE_USER_SIZE(block_size) ((block_size) - FREE_OVERHEAD - FOOTER_SIZE)

// Taille utilisable pour l’utilisateur à partir d’un bloc occupé
#define BUSY_USER_SIZE(block_size) ((block_size) - BUSY_OVERHEAD - FOOTER_SIZE)

// Taille minimale occupée (pour pouvoir être convertie en bloc libre)
#define MIN_BUSY_SIZE ALIGN(FREE_BLOCK_TOTAL(0) - BUSY_BLOCK_TOTAL(0))

#endif /* MEM_MACROS_NACHOS_H */