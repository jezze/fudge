.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments, which follow fmt on the stack
.global cstring_write_fmt
cstring_write_fmt:
    lea 20(%sp), %a0
    move.l %a0, -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    jsr cstring_write_fmtv
    lea 20(%sp), %sp
    rts

