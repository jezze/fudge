.section .text

.global cpu_getsatp
cpu_getsatp:
    csrr a0, satp
    ret

.global cpu_setsatp
cpu_setsatp:
    csrw satp, a0
    sfence.vma
    ret

.global cpu_flushtlb
cpu_flushtlb:
    sfence.vma
    ret

.global cpu_gettime
cpu_gettime:
    csrr a0, time
    ret

# SBI timer extension (TIME), set_timer
.global cpu_settimer
cpu_settimer:
    li a7, 0x54494D45
    li a6, 0
    ecall
    ret

.global cpu_enableinterrupt
cpu_enableinterrupt:
    csrs sie, a0
    ret

.global cpu_disableinterrupt
cpu_disableinterrupt:
    csrc sie, a0
    ret

.global cpu_settrap
cpu_settrap:
    csrw stvec, a0
    ret

.global cpu_setstatus
cpu_setstatus:
    csrw sstatus, a0
    ret

.global cpu_halt
cpu_halt:
    wfi
    j cpu_halt

.global cpu_leave
cpu_leave:
    mv sp, a0
    j trap_exit

# The FPU's registers and fcsr, 264 bytes at a0 (ARCH_FPUSTATESIZE); both turn the FPU on first.
.option push
.option arch, +d

.global cpu_savefpu
cpu_savefpu:
    li t0, 0x6000
    csrs sstatus, t0
    fsd f0, 0(a0)
    fsd f1, 8(a0)
    fsd f2, 16(a0)
    fsd f3, 24(a0)
    fsd f4, 32(a0)
    fsd f5, 40(a0)
    fsd f6, 48(a0)
    fsd f7, 56(a0)
    fsd f8, 64(a0)
    fsd f9, 72(a0)
    fsd f10, 80(a0)
    fsd f11, 88(a0)
    fsd f12, 96(a0)
    fsd f13, 104(a0)
    fsd f14, 112(a0)
    fsd f15, 120(a0)
    fsd f16, 128(a0)
    fsd f17, 136(a0)
    fsd f18, 144(a0)
    fsd f19, 152(a0)
    fsd f20, 160(a0)
    fsd f21, 168(a0)
    fsd f22, 176(a0)
    fsd f23, 184(a0)
    fsd f24, 192(a0)
    fsd f25, 200(a0)
    fsd f26, 208(a0)
    fsd f27, 216(a0)
    fsd f28, 224(a0)
    fsd f29, 232(a0)
    fsd f30, 240(a0)
    fsd f31, 248(a0)
    frcsr t0
    sd t0, 256(a0)
    ret

.global cpu_loadfpu
cpu_loadfpu:
    li t0, 0x6000
    csrs sstatus, t0
    fld f0, 0(a0)
    fld f1, 8(a0)
    fld f2, 16(a0)
    fld f3, 24(a0)
    fld f4, 32(a0)
    fld f5, 40(a0)
    fld f6, 48(a0)
    fld f7, 56(a0)
    fld f8, 64(a0)
    fld f9, 72(a0)
    fld f10, 80(a0)
    fld f11, 88(a0)
    fld f12, 96(a0)
    fld f13, 104(a0)
    fld f14, 112(a0)
    fld f15, 120(a0)
    fld f16, 128(a0)
    fld f17, 136(a0)
    fld f18, 144(a0)
    fld f19, 152(a0)
    fld f20, 160(a0)
    fld f21, 168(a0)
    fld f22, 176(a0)
    fld f23, 184(a0)
    fld f24, 192(a0)
    fld f25, 200(a0)
    fld f26, 208(a0)
    fld f27, 216(a0)
    fld f28, 224(a0)
    fld f29, 232(a0)
    fld f30, 240(a0)
    fld f31, 248(a0)
    ld t0, 256(a0)
    fscsr t0
    ret

.option pop

