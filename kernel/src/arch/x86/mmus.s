.intel_syntax noprefix

.extern isr_handle
.extern isr_common_cpu

.global mmu_routine
mmu_routine:
    cli
    push eax
    push 0x0E
    jmp isr_common_cpu

