extern main
extern call_exit

global _start
_start:
    jmp main
    call call_exit

