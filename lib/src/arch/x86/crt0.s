.intel_syntax noprefix

.extern main
.extern call_exit

.section .text

.align 4

.global _start
_start:
    call main
    call call_exit

