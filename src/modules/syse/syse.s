.code32

.section .text

.global syse_interrupt
syse_interrupt:
    pusha
    call syse_syscall
    popa
    sti
    sysexit

