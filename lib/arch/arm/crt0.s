.extern main
.extern call_exit

.globl _start
_start:
    bl main
    bl call_exit

