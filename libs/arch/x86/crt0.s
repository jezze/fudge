.extern main 
.extern call_exit 

.global _start 
_start:  
    call main
    call call_exit

