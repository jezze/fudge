.code16

.section .text

.global smp_init
smp_init:
    movw $0xB800, %ax
    movw %ax, %es
    xor %bx, %bx
    movw $0x4020, %ax
    movw %ax, %es:(%bx)
    jmp .

