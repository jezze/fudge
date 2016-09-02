.code32

.extern arch_syscall2

.global syse_interrupt
syse_interrupt:
    pusha
    call arch_syscall2
    popa
    sysexit

