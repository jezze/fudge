.code32

.extern main
.extern call_despawn

.global _start
_start:
    call main
    call call_despawn

