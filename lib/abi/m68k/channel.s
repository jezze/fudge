.section .text

# channel_send_fmt(a, b, c, fmt, ...) calls channel_send_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments, which follow fmt on the stack
.global channel_send_fmt
channel_send_fmt:
    lea 20(%sp), %a0
    move.l %a0, -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    move.l 20(%sp), -(%sp)
    jsr channel_send_fmtv
    lea 20(%sp), %sp
    rts

