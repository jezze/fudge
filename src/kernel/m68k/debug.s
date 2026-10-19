.section .text

# debug_fmt(level, file, line, fmt, ...) calls debug_fmtv(level, file, line, fmt, args), args pointing at
# the variadic arguments, which follow fmt on the stack
.global debug_fmt
debug_fmt:
    lea 20(%sp), %a0
    move.l %a0, -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    jsr debug_fmtv
    lea 20(%sp), %sp
    rts

