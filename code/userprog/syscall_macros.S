    /*
     * MIPS syscall stub macros
     *
     * These macros help define syscall stubs in assembly language.
     * They handle error checking and setting errno appropriately.
     *
     * Usage:
     *   DEFINE_SYSCALL_STUB name, scno
     *       Defines a syscall stub named 'name' for syscall number 'scno'.
     *       On error, sets errno and returns -1.
     *
     *   DEFINE_SIMPLE_SYSCALL name, scno
     *       Defines a simple syscall stub named 'name' for syscall number 'scno'.
     *       Does not handle errno; just returns the syscall result.
     *
     * Assumes:
     *   - $2 is used for syscall number and return value.
     *   - $3 is used for temporary storage of error code.
     *   - $gp points to the thread-local storage area where errno is stored at offset 12.
     *   - A global variable __global_errno exists for storing errno when $gp is null.
     */


    # WARNING: Any change here must be reflected in tls.h
    .equ TLS_OFFSET_ERRNO, 12

    .macro HANDLE_SYSCALL_ERROR
    subu  $3,$0,$2
    beqz  $gp, __errno_global_\@
    nop
    sw    $3, TLS_OFFSET_ERRNO($gp)
    j     __errno_done_\@
    nop
__errno_global_\@:
    sw    $3, __global_errno
__errno_done_\@:
    .endm


    .macro CLEAR_ERRNO
    beqz  $gp, __clear_global_\@
    nop
    sw    $0, TLS_OFFSET_ERRNO($gp)
    j     __clear_done_\@
    nop
__clear_global_\@:
    sw    $0, __global_errno
__clear_done_\@:
    .endm


    .macro DEFINE_SYSCALL_STUB name, scno
    .globl \name
    .ent   \name
\name:
    addiu $2,$0,\scno
    syscall
    bgez  $2, __ok_\name
    nop
    HANDLE_SYSCALL_ERROR
    addiu $2,$0,-1
    j     $31
    nop
__ok_\name:
    CLEAR_ERRNO
    j     $31
    nop
    .end  \name
    .endm


    .macro DEFINE_SIMPLE_SYSCALL name, scno
    .globl \name
    .ent   \name
\name:
    addiu $2,$0,\scno
    syscall
    j     $31
    nop
    .end  \name
    .endm
