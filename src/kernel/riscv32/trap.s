.set FRAME,                             144
.set FRAME_SP,                          120
.set FRAME_PC,                          124
.set FRAME_STATUS,                      128
.set STATUS_SPP,                        0x100

# Frames on the kernel stack are struct cpu_frame: x1, x3-x31, sp, pc, status, pad (144 bytes).
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
    sw x1, 0(sp)
    sw x3, 4(sp)
    sw x4, 8(sp)
    sw x5, 12(sp)
    sw x6, 16(sp)
    sw x7, 20(sp)
    sw x8, 24(sp)
    sw x9, 28(sp)
    sw x10, 32(sp)
    sw x11, 36(sp)
    sw x12, 40(sp)
    sw x13, 44(sp)
    sw x14, 48(sp)
    sw x15, 52(sp)
    sw x16, 56(sp)
    sw x17, 60(sp)
    sw x18, 64(sp)
    sw x19, 68(sp)
    sw x20, 72(sp)
    sw x21, 76(sp)
    sw x22, 80(sp)
    sw x23, 84(sp)
    sw x24, 88(sp)
    sw x25, 92(sp)
    sw x26, 96(sp)
    sw x27, 100(sp)
    sw x28, 104(sp)
    sw x29, 108(sp)
    sw x30, 112(sp)
    sw x31, 116(sp)
    csrrw t0, sscratch, zero
    sw t0, FRAME_SP(sp)
    csrr t0, sepc
    sw t0, FRAME_PC(sp)
    csrr t0, sstatus
    sw t0, FRAME_STATUS(sp)
    mv a0, sp
    addi a1, sp, FRAME_SP
    csrr a2, scause
    csrr a3, stval
    call arch_trap

# Returns to the frame at sp, which the handler may have changed to another task's.
.global trap_exit
trap_exit:
    lw t0, FRAME_STATUS(sp)
    csrw sstatus, t0
    lw t1, FRAME_PC(sp)
    csrw sepc, t1
    andi t0, t0, STATUS_SPP
    bnez t0, trap_restore
    la t1, trap_stack
    lw t1, 0(t1)
    csrw sscratch, t1

trap_restore:
    lw x1, 0(sp)
    lw x3, 4(sp)
    lw x4, 8(sp)
    lw x5, 12(sp)
    lw x6, 16(sp)
    lw x7, 20(sp)
    lw x8, 24(sp)
    lw x9, 28(sp)
    lw x10, 32(sp)
    lw x11, 36(sp)
    lw x12, 40(sp)
    lw x13, 44(sp)
    lw x14, 48(sp)
    lw x15, 52(sp)
    lw x16, 56(sp)
    lw x17, 60(sp)
    lw x18, 64(sp)
    lw x19, 68(sp)
    lw x20, 72(sp)
    lw x21, 76(sp)
    lw x22, 80(sp)
    lw x23, 84(sp)
    lw x24, 88(sp)
    lw x25, 92(sp)
    lw x26, 96(sp)
    lw x27, 100(sp)
    lw x28, 104(sp)
    lw x29, 108(sp)
    lw x30, 112(sp)
    lw x31, 116(sp)
    lw sp, FRAME_SP(sp)
    sret

.section .data

.balign 4
.global trap_stack
trap_stack:
    .word 0

