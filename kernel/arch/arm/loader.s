interrupt_vector_table:
    b . @ Reset
    b .
    b . @ SWI instruction
    b .
    b .
    b .
    b .
    b .

.comm stack, 0x10000 @ Reserve 64k stack in the BSS

.global _loader
_loader:
    ldr sp, =stack+0x10000 @ Set up the stack
    bl arch_init @ Jump to init function

1:
    b 1b @ Halt


