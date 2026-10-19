.set FRAME_SR,                          64
.set FRAME_PC,                          66

.section .text

# Every exception masks interrupts, saves d0-d7/a0-a6 and the user stack pointer below the frame the cpu
# pushed (struct cpu_frame) and calls the handler with it. The handler returns 1 when the frame now holds a
# fresh context (another task, or the idle loop), or 0 to go back to the interrupted code where it was (a
# page fault or an interrupt in the kernel).
.macro entry name, handler
.global \name
\name:
    move.w #0x2700, %sr
    movem.l %d0-%d7/%a0-%a6, -(%sp)
    move.l %usp, %a0
    move.l %a0, -(%sp)
    move.l %sp, -(%sp)
    jsr \handler
    addq.l #4, %sp
    bra isr_exit
.endm

entry isr_accessfault, arch_accessfault
entry isr_fault, arch_fault
entry isr_interrupt, arch_interrupt
entry isr_syscall, arch_syscall

# A fresh context starts from a format 0 frame at the top of the kernel stack; going back uses the frame
# the cpu pushed, which a 68040 needs to continue an instruction an access fault interrupted.
.global isr_exit
isr_exit:
    move.l %sp, %a1
    lea FRAME_SR(%a1), %a2
    tst.l %d0
    beq 1f
    move.l isr_stack, %a2
    subq.l #8, %a2
    move.w FRAME_SR(%a1), (%a2)
    move.l FRAME_PC(%a1), 2(%a2)
    clr.w 6(%a2)
1:
    move.l (%a1), %a0
    move.l %a0, %usp
    move.l %a2, %sp
    movem.l 4(%a1), %d0-%d7/%a0-%a6
    rte

.section .data

.balign 4
.global isr_stack
isr_stack:
    .long 0

.balign 4
.global isr_vectors
isr_vectors:
    .long 0, 0
    .long isr_accessfault
    .rept 21
    .long isr_fault
    .endr
    .rept 8
    .long isr_interrupt
    .endr
    .long isr_syscall
    .rept 223
    .long isr_fault
    .endr

