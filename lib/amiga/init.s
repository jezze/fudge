.set STACK_SIZE,                        0x4000

.section .text.boot

# fudgeboot enters in supervisor mode with the MMU, caches and interrupts off, the ramdisk's address in a0 and its size in d0
.global _start
_start:
    move.w #0x2700, %sr
    lea stack + STACK_SIZE, %sp
    move.l %d0, -(%sp)
    move.l %a0, -(%sp)
    jsr amiga_setup

halt:
    stop #0x2700
    bra halt

.comm stack, STACK_SIZE, 4

