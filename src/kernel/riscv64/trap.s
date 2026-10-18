.set FRAME,                             272
.set FRAME_SP,                          240
.set FRAME_PC,                          248
.set FRAME_STATUS,                      256
.set STATUS_SPP,                        0x100

# Frames on the kernel stack are struct cpu_frame: x1, x3-x31, sp, pc, status, pad (272 bytes).
# While a task runs sscratch holds the top of the kernel stack (trap_stack), in the kernel 0,
# so a trap from a task starts at the top and one from the kernel (a page fault while it
# touches task memory, an interrupt while halted) below the interrupted kernel's sp.

.section .text

.balign 4
.global trap_entry
trap_entry:
    csrrw sp, sscratch, sp
    bnez sp, trap_save
    csrr sp, sscratch

trap_save:
    addi sp, sp, -FRAME
    sd x1, 0(sp)
    sd x3, 8(sp)
    sd x4, 16(sp)
    sd x5, 24(sp)
    sd x6, 32(sp)
    sd x7, 40(sp)
    sd x8, 48(sp)
    sd x9, 56(sp)
    sd x10, 64(sp)
    sd x11, 72(sp)
    sd x12, 80(sp)
    sd x13, 88(sp)
    sd x14, 96(sp)
    sd x15, 104(sp)
    sd x16, 112(sp)
    sd x17, 120(sp)
    sd x18, 128(sp)
    sd x19, 136(sp)
    sd x20, 144(sp)
    sd x21, 152(sp)
    sd x22, 160(sp)
    sd x23, 168(sp)
    sd x24, 176(sp)
    sd x25, 184(sp)
    sd x26, 192(sp)
    sd x27, 200(sp)
    sd x28, 208(sp)
    sd x29, 216(sp)
    sd x30, 224(sp)
    sd x31, 232(sp)
    csrrw t0, sscratch, zero
    sd t0, FRAME_SP(sp)
    csrr t0, sepc
    sd t0, FRAME_PC(sp)
    csrr t0, sstatus
    sd t0, FRAME_STATUS(sp)
    mv a0, sp
    addi a1, sp, FRAME_SP
    csrr a2, scause
    csrr a3, stval
    call arch_trap

# Returns to the frame at sp, which the handler may have changed to another task's.
.global trap_exit
trap_exit:
    ld t0, FRAME_STATUS(sp)
    csrw sstatus, t0
    ld t1, FRAME_PC(sp)
    csrw sepc, t1
    andi t0, t0, STATUS_SPP
    bnez t0, trap_restore
    la t1, trap_stack
    ld t1, 0(t1)
    csrw sscratch, t1

trap_restore:
    ld x1, 0(sp)
    ld x3, 8(sp)
    ld x4, 16(sp)
    ld x5, 24(sp)
    ld x6, 32(sp)
    ld x7, 40(sp)
    ld x8, 48(sp)
    ld x9, 56(sp)
    ld x10, 64(sp)
    ld x11, 72(sp)
    ld x12, 80(sp)
    ld x13, 88(sp)
    ld x14, 96(sp)
    ld x15, 104(sp)
    ld x16, 112(sp)
    ld x17, 120(sp)
    ld x18, 128(sp)
    ld x19, 136(sp)
    ld x20, 144(sp)
    ld x21, 152(sp)
    ld x22, 160(sp)
    ld x23, 168(sp)
    ld x24, 176(sp)
    ld x25, 184(sp)
    ld x26, 192(sp)
    ld x27, 200(sp)
    ld x28, 208(sp)
    ld x29, 216(sp)
    ld x30, 224(sp)
    ld x31, 232(sp)
    ld sp, FRAME_SP(sp)
    sret

.section .data

.balign 8
.global trap_stack
trap_stack:
    .dword 0
