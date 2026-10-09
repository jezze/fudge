.code64

.section .text

# cstring_write_fmt(a, b, c, fmt, ...) calls cstring_write_fmtv(a, b, c, fmt, args), args pointing at the
# variadic arguments: on x86_64 they start in r8 and r9 and continue on the stack above the return address,
# so the return address is moved below r8 and r9 to make them one array.
.global cstring_write_fmt
cstring_write_fmt:
    popq %r11
    pushq %r9
    pushq %r8
    movq %rsp, %r8
    pushq %r11
    subq $8, %rsp
    call cstring_write_fmtv
    addq $8, %rsp
    popq %r11
    addq $16, %rsp
    pushq %r11
    ret
