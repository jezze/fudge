.section .text

# the kernel runs on the same tables as the task (supervisor accesses use srp), so both roots change together
.global cpu_getroot
cpu_getroot:
    movec %srp, %d0
    rts

.global cpu_gettc
cpu_gettc:
    movec %tc, %d0
    rts

.global cpu_setroot
cpu_setroot:
    move.l 4(%sp), %d0
    movec %d0, %srp
    movec %d0, %urp
    pflusha
    rts

.global cpu_settc
cpu_settc:
    move.l 4(%sp), %d0
    movec %d0, %tc
    pflusha
    rts

.global cpu_setvbr
cpu_setvbr:
    move.l 4(%sp), %d0
    movec %d0, %vbr
    rts

.global cpu_flushtlb
cpu_flushtlb:
    pflusha
    rts

.global cpu_setcacr
cpu_setcacr:
    move.l 4(%sp), %d0
    movec %d0, %cacr
    rts

# the instruction cache does not see the data writes that put code in memory
.global cpu_invalidatecode
cpu_invalidatecode:
    cinva %ic
    rts

.global cpu_halt
cpu_halt:
    stop #0x2000
    bra cpu_halt

.global cpu_leave
cpu_leave:
    move.l 4(%sp), %sp
    moveq #1, %d0
    bra isr_exit

# The FPU's state, 208 bytes (ARCH_FPUSTATESIZE): the fsave frame, then fp0-fp7 and the control registers unless the frame is null (an all-zero state restores a reset FPU).
.global cpu_savefpu
cpu_savefpu:
    move.l 4(%sp), %a0
    fsave (%a0)
    tst.b (%a0)
    beq 1f
    fmovem.x %fp0-%fp7, 96(%a0)
    fmovem.l %fpcr/%fpsr/%fpiar, 192(%a0)
1:
    rts

.global cpu_loadfpu
cpu_loadfpu:
    move.l 4(%sp), %a0
    tst.b (%a0)
    beq 1f
    fmovem.l 192(%a0), %fpcr/%fpsr/%fpiar
    fmovem.x 96(%a0), %fp0-%fp7
1:
    frestore (%a0)
    rts

