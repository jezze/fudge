.code32

.extern smp_setup

.section .text

.global smp_begin
smp_begin:

.code16
init:
    cli
    movw $0x8800, %ax
    movw %ax, %sp
    call print
    call finish

print:
    movw $0xB800, %ax
    movw %ax, %es
    xor %bx, %bx
    movw $0x4020, %ax
    movw %ax, %es:(%bx)
    ret

finish:
    hlt

.code32

.global smp_end
smp_end:
