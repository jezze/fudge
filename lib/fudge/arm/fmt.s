.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: the named ones fill r0-r3, so the rest lie at sp, and
# args goes on the stack as the fifth argument.
.global cstring_write_fmt
.type cstring_write_fmt, %function
cstring_write_fmt:
    mov r12, sp
    push {r12, lr}
    bl cstring_write_fmtv
    pop {r12, pc}
