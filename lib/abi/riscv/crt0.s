.section .text

.extern main
.extern call_despawn

.globl _start
_start:
    tail main
    tail call_despawn

