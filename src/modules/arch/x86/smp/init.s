.code16

.section .text

.global smp_begin
smp_begin:
    movw $0xB800, %ax
    movw %ax, %es
    xor %bx, %bx
    movw $0x4020, %ax
    movw %ax, %es:(%bx)
    jmp .

.global smp_end
smp_end:
