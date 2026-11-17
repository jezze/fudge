.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: x4-x7 are stored right below the ones on the stack so
# they form one array.
.globl cstring_write_fmt
cstring_write_fmt:
    stp x29, x30, [sp, #-48]!
    stp x4, x5, [sp, #16]
    stp x6, x7, [sp, #32]
    add x4, sp, #16
    bl cstring_write_fmtv
    ldp x29, x30, [sp], #48
    ret

