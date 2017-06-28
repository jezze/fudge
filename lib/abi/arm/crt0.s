.section .text

.extern main
.extern call_despawn

.globl _start
_start:
    bl main
    bl call_despawn

