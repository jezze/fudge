.set STACK_SIZE,                        0x4000

.section .text.boot

# qemu enters in supervisor mode with the MMU off; the bootinfo it leaves after the kernel is not used
.global _start
_start:
    move.w #0x2700, %sr
    lea stack + STACK_SIZE, %sp
    jsr m68kvirt_setup

halt:
    stop #0x2700
    bra halt

.comm stack, STACK_SIZE, 4

