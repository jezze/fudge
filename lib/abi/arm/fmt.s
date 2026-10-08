.section .text

.global channel_send_fmt
.type channel_send_fmt, %function
channel_send_fmt:
    mov r12, sp
    push {r12, lr}
    bl channel_send_fmtv
    pop {r12, pc}
