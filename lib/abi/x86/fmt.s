.code32

.section .text

.global channel_send_fmt
channel_send_fmt:
    leal 20(%esp), %eax
    pushl %eax
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    call channel_send_fmtv
    addl $20, %esp
    ret
