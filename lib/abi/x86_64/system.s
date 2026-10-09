.code64

.section .text

.global system_run
system_run:
    popq %r11
    pushq %r9
    pushq %r8
    pushq %rcx
    pushq %rdx
    movq %rsp, %rdx
    pushq %r11
    subq $8, %rsp
    call system_runv
    addq $8, %rsp
    popq %r11
    addq $32, %rsp
    pushq %r11
    ret

.global system_feed
system_feed:
    popq %r11
    pushq %r9
    movq %rsp, %r9
    pushq %r11
    call system_feedv
    popq %r11
    addq $8, %rsp
    pushq %r11
    ret
