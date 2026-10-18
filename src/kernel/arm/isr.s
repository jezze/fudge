.set MODE_SVC,                          0x13
.set FLAGS_IF,                          0xC0

# Frames on the kernel stack are struct cpu_frame: cpsr, sp, lr, pc, r0-r12, pad (72 bytes).
# Entries from user mode start at the top of the kernel stack (isr_stack), entries from
# kernel mode (aborts while the kernel touches task memory, interrupts while halted) below
# the interrupted kernel's sp.

.section .text

.global isr_vectors
isr_vectors:
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    ldr pc, [pc, #24]
    .word isr_undefined
    .word isr_undefined
    .word isr_syscall
    .word isr_prefetchabort
    .word isr_dataabort
    .word isr_undefined
    .word isr_irq
    .word isr_undefined

# In the exception mode: save r0, r1, the return address, spsr and the handler in the
# mode's scratch area (its sp), then continue in svc mode with r1 pointing at it.
.macro enter offset, handler
    sub lr, lr, #\offset
    stmia sp, {r0, r1, lr}
    mrs r0, spsr
    ldr r1, =\handler
    str r0, [sp, #12]
    str r1, [sp, #16]
    mov r1, sp
    msr cpsr_c, #(MODE_SVC | FLAGS_IF)
    b isr_common
.endm

.global isr_undefined
isr_undefined:
    enter 4, arch_undefined

.global isr_prefetchabort
isr_prefetchabort:
    enter 4, arch_prefetchabort

.global isr_dataabort
isr_dataabort:
    enter 8, arch_dataabort

.global isr_irq
isr_irq:
    enter 4, arch_irq

isr_common:
    ldr r0, [r1, #12]
    tst r0, #0x0F
    ldreq sp, =isr_stack
    ldreq sp, [sp]
    sub sp, sp, #72
    str r0, [sp]
    add r0, sp, #24
    stmia r0, {r2-r12}
    ldmia r1, {r2-r6}
    str r2, [sp, #16]
    str r3, [sp, #20]
    str r4, [sp, #12]
    tst r5, #0x0F
    addeq r0, sp, #4
    stmeqia r0, {sp, lr}^
    addne r0, sp, #72
    strne r0, [sp, #4]
    strne lr, [sp, #8]
    add r0, sp, #16
    mov r1, sp
    blx r6
    b isr_exit

.global isr_syscall
isr_syscall:
    ldr sp, =isr_stack
    ldr sp, [sp]
    sub sp, sp, #56
    stmia sp, {r0-r12, lr}
    sub sp, sp, #16
    str lr, [sp, #12]
    mrs r0, spsr
    str r0, [sp]
    add r0, sp, #4
    stmia r0, {sp, lr}^
    add r0, sp, #16
    mov r1, sp
    bl arch_syscall

# Returns to the frame at sp, which the handler may have changed to another task's.
.global isr_exit
isr_exit:
    ldr r0, [sp]
    msr spsr_cxsf, r0
    tst r0, #0x0F
    bne isr_exitkernel
    add r0, sp, #4
    ldmia r0, {sp, lr}^
    nop
    ldr lr, [sp, #12]
    add sp, sp, #16
    ldmia sp, {r0-r12}
    add sp, sp, #56
    movs pc, lr

# The kernel continues with sp just above the frame.
isr_exitkernel:
    ldr r0, [sp, #12]
    str r0, [sp, #68]
    ldr lr, [sp, #8]
    add sp, sp, #16
    ldmia sp!, {r0-r12, pc}^

.section .data

.global isr_stack
isr_stack:
    .word 0
