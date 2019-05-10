.code32

.extern init
.extern main
.extern call_despawn

.section .text

.global _start
_start:
    call init
    call main
    call call_despawn

