.intel_syntax noprefix

.extern isr_handle

.global syscall_routine
syscall_routine:
    cli
    push eax
    push 0x80
    jmp isr_common

