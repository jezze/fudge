.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: a4-a7 are stored right below the ones on the stack so
# they form one array.
.globl cstring_write_fmt
cstring_write_fmt:
    addi sp, sp, -32
    sw ra, 0(sp)
    sw a4, 16(sp)
    sw a5, 20(sp)
    sw a6, 24(sp)
    sw a7, 28(sp)
    addi a4, sp, 16
    call cstring_write_fmtv
    lw ra, 0(sp)
    addi sp, sp, 32
    ret

