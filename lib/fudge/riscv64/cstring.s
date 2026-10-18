.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: a4-a7 are stored right below the ones on the stack so
# they form one array.
.globl cstring_write_fmt
cstring_write_fmt:
    addi sp, sp, -48
    sd ra, 8(sp)
    sd a4, 16(sp)
    sd a5, 24(sp)
    sd a6, 32(sp)
    sd a7, 40(sp)
    addi a4, sp, 16
    call cstring_write_fmtv
    ld ra, 8(sp)
    addi sp, sp, 48
    ret
