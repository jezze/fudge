.set FRAME,                             272
.set FRAME_SP,                          248
.set FRAME_PC,                          256
.set FRAME_STATUS,                      264
.set STATUS_MODEMASK,                   0x0F

# Frames on the kernel stack are struct cpu_frame: x0-x30, sp, pc, status (272 bytes).
# The kernel runs on SP_EL1. Returning to a task resets it to the top of the kernel stack
# (trap_stack), so a trap from a task starts there and one from the kernel (a page fault
# while it touches task memory, an interrupt while halted) below the interrupted kernel's sp.

.macro vector type
.balign 0x80
    sub sp, sp, #FRAME
    stp x0, x1, [sp, #0]
    mov x0, #\type
    b trap_save
.endm

.section .text

.balign 0x800
.global trap_vector
trap_vector:
    vector 0
    vector 1
    vector 2
    vector 3
    vector 0
    vector 1
    vector 2
    vector 3
    vector 0
    vector 1
    vector 2
    vector 3
    vector 0
    vector 1
    vector 2
    vector 3

trap_save:
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    mov x2, x0
    mrs x1, spsr_el1
    str x1, [sp, #FRAME_STATUS]
    mrs x0, elr_el1
    str x0, [sp, #FRAME_PC]
    mrs x0, sp_el0
    tst x1, #STATUS_MODEMASK
    b.eq trap_call
    add x0, sp, #FRAME

trap_call:
    str x0, [sp, #FRAME_SP]
    mov x0, sp
    add x1, sp, #FRAME_SP
    mrs x3, esr_el1
    mrs x4, far_el1
    bl arch_trap

# Returns to the frame at sp, which the handler may have changed to another task's.
.global trap_exit
trap_exit:
    ldr x0, [sp, #FRAME_PC]
    msr elr_el1, x0
    ldr x1, [sp, #FRAME_STATUS]
    msr spsr_el1, x1
    ldr x2, [sp, #FRAME_SP]
    mov x30, sp
    tst x1, #STATUS_MODEMASK
    b.ne trap_restore
    msr sp_el0, x2
    ldr x2, =trap_stack
    ldr x2, [x2]

trap_restore:
    mov sp, x2
    ldp x0, x1, [x30, #0]
    ldp x2, x3, [x30, #16]
    ldp x4, x5, [x30, #32]
    ldp x6, x7, [x30, #48]
    ldp x8, x9, [x30, #64]
    ldp x10, x11, [x30, #80]
    ldp x12, x13, [x30, #96]
    ldp x14, x15, [x30, #112]
    ldp x16, x17, [x30, #128]
    ldp x18, x19, [x30, #144]
    ldp x20, x21, [x30, #160]
    ldp x22, x23, [x30, #176]
    ldp x24, x25, [x30, #192]
    ldp x26, x27, [x30, #208]
    ldp x28, x29, [x30, #224]
    ldr x30, [x30, #240]
    eret

.ltorg

.section .data

.balign 8
.global trap_stack
trap_stack:
    .quad 0

