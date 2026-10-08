.code32

.section .text

.global debug_fmt
debug_fmt:
    leal 20(%esp), %eax
    pushl %eax
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    pushl 20(%esp)
    call debug_fmtv
    addl $20, %esp
    ret
