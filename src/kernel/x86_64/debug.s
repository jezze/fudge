.code64

.section .text

.global debug_fmt
debug_fmt:
    popq %r11
    pushq %r9
    pushq %r8
    movq %rsp, %r8
    pushq %r11
    subq $8, %rsp
    call debug_fmtv
    addq $8, %rsp
    popq %r11
    addq $16, %rsp
    pushq %r11
    ret
