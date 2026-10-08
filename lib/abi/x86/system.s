.code32

.section .text

.global system_run
system_run:
    leal 12(%esp), %eax
    pushl %eax
    pushl 12(%esp)
    pushl 12(%esp)
    call system_runv
    addl $12, %esp
    ret
