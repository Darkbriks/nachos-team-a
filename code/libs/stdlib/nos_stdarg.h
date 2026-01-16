#ifndef NOS_STDARG_H
#define NOS_STDARG_H

typedef char* va_list;

/**
 * @brief Extract the next argument in a variable argument list.
 * @param V The variable argument list.
 * @param P The type of the next argument to retrieve.
 * @return The next argument in the list, cast to paramType.
 */
#define va_arg(V, P) (*(P*)((V += sizeof(P)) - sizeof(P)))

/**
 * @brief Duplicate a variable argument list.
 * @param VSRC The source variable argument list to duplicate.
 * @param VDST The destination variable argument list.
 */
#define va_copy(VDST, VSRC) (VDST = VSRC)

/**
 * @brief Free a variable argument list.
 * @param V The variable argument list to free.
 */
#define va_end(V) (V = (va_list)0)

/**
 * @brief Initialize a variable argument list.
 * @param V The variable argument list to initialize.
 * @param P The last fixed parameter before the variable arguments.
 */
#define va_start(V, P) (V = (char*)&P + sizeof(P))

#endif
