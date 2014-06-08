interrupt_vector_table:
    b isr_undefined
    b isr_undefined
    b isr_undefined
    b isr_undefined
    b isr_undefined
    b isr_undefined
    b isr_undefined
    b isr_undefined

.comm stack, 0x10000

.global init
init:
    ldr sp, =stack+0x10000
    bl arch_setup
1:
    b 1b

