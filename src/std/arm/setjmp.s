.text

.type    setjmp, #function
.type    longjmp, #function
.balign    4

.globl setjmp
setjmp:
    stmia r0, {r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
    mov r0, #0
    mov pc, lr

.size setjmp,.-setjmp

.globl longjmp
longjmp:
    ldmia r0, {r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
    mov r0, r1
    mov pc, lr

.size longjmp,.-longjmp

