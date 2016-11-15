.code32

.extern syse_syscall

.global syse_interrupt
syse_interrupt:
    cli
    pusha
    call syse_syscall
    popa
    sti
    sysexit

