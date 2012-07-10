interrupt_vector_table:
    b .
    b .
    b .
    b .
    b .
    b .
    b .
    b .

.comm stack, 0x10000

.global init
init:
    ldr sp, =stack+0x10000
    bl arch_setup
1:
    b 1b


