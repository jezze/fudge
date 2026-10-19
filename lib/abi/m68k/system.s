.section .text

# system_run(target, fmt, ...) and system_feed(a, b, c, d, fmt, ...) call their v function with args pointing
# at the variadic arguments, which follow fmt on the stack
.global system_run
system_run:
    lea 12(%sp), %a0
    move.l %a0, -(%sp)
    move.l 12(%sp), -(%sp)
    move.l 12(%sp), -(%sp)
    jsr system_runv
    lea 12(%sp), %sp
    rts

.global system_feed
system_feed:
    lea 24(%sp), %a0
    move.l %a0, -(%sp)
    move.l 24(%sp), -(%sp)
    move.l 24(%sp), -(%sp)
    move.l 24(%sp), -(%sp)
    move.l 24(%sp), -(%sp)
    move.l 24(%sp), -(%sp)
    jsr system_feedv
    lea 24(%sp), %sp
    rts

