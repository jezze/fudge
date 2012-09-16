.intel_syntax noprefix

.extern isr_common

.global mmu_routine
mmu_routine:
    cli
    push eax
    push 0x0E
    jmp isr_common

