.set MAIR,                              0x4400
.set TCR,                               0x800019
.set SCTLR,                             0x30D50819
.set CPACR_FPEN,                        0x300000

# MAIR: attribute 0 is Device-nGnRnE, 1 Normal non-cacheable. TCR: 39-bit addresses
# through TTBR0 (T0SZ 25, 4 KB pages, three levels), TTBR1 walks off, the physical
# size taken from ID_AA64MMFR0_EL1 (at most 48 bits). SCTLR: MMU on, caches off,
# stack alignment checks, WFI and WFE allowed in EL0.

.section .text

.global cpu_getttbr
cpu_getttbr:
    mrs x0, ttbr0_el1
    ret

.global cpu_setttbr
cpu_setttbr:
    msr ttbr0_el1, x0
    isb
    tlbi vmalle1
    dsb ish
    isb
    ret

.global cpu_enablemmu
cpu_enablemmu:
    ldr x1, =MAIR
    msr mair_el1, x1
    mrs x1, id_aa64mmfr0_el1
    and x1, x1, #7
    mov x2, #5
    cmp x1, x2
    csel x1, x2, x1, hi
    lsl x1, x1, #32
    ldr x2, =TCR
    orr x1, x1, x2
    msr tcr_el1, x1
    msr ttbr0_el1, x0
    isb
    tlbi vmalle1
    dsb ish
    isb
    ldr x1, =SCTLR
    msr sctlr_el1, x1
    isb
    ret

.global cpu_flushtlb
cpu_flushtlb:
    dsb ishst
    tlbi vmalle1
    dsb ish
    isb
    ret

.global cpu_setvector
cpu_setvector:
    msr vbar_el1, x0
    isb
    ret

.global cpu_enablefpu
cpu_enablefpu:
    mrs x1, cpacr_el1
    orr x1, x1, #CPACR_FPEN
    msr cpacr_el1, x1
    isb
    ret

.global cpu_disablefpu
cpu_disablefpu:
    mrs x1, cpacr_el1
    bic x1, x1, #CPACR_FPEN
    msr cpacr_el1, x1
    isb
    ret

.global cpu_halt
cpu_halt:
    wfi
    b cpu_halt

.global cpu_leave
cpu_leave:
    mov sp, x0
    b trap_exit

# q0-q31, fpcr and fpsr, 528 bytes at x0 (ARCH_FPUSTATESIZE, 16-byte aligned); both turn the FPU on first.
.global cpu_savefpu
cpu_savefpu:
    mrs x1, cpacr_el1
    orr x1, x1, #CPACR_FPEN
    msr cpacr_el1, x1
    isb
    stp q0, q1, [x0, #0]
    stp q2, q3, [x0, #32]
    stp q4, q5, [x0, #64]
    stp q6, q7, [x0, #96]
    stp q8, q9, [x0, #128]
    stp q10, q11, [x0, #160]
    stp q12, q13, [x0, #192]
    stp q14, q15, [x0, #224]
    stp q16, q17, [x0, #256]
    stp q18, q19, [x0, #288]
    stp q20, q21, [x0, #320]
    stp q22, q23, [x0, #352]
    stp q24, q25, [x0, #384]
    stp q26, q27, [x0, #416]
    stp q28, q29, [x0, #448]
    stp q30, q31, [x0, #480]
    mrs x1, fpcr
    mrs x2, fpsr
    add x3, x0, #512
    stp x1, x2, [x3]
    ret

.global cpu_loadfpu
cpu_loadfpu:
    mrs x1, cpacr_el1
    orr x1, x1, #CPACR_FPEN
    msr cpacr_el1, x1
    isb
    ldp q0, q1, [x0, #0]
    ldp q2, q3, [x0, #32]
    ldp q4, q5, [x0, #64]
    ldp q6, q7, [x0, #96]
    ldp q8, q9, [x0, #128]
    ldp q10, q11, [x0, #160]
    ldp q12, q13, [x0, #192]
    ldp q14, q15, [x0, #224]
    ldp q16, q17, [x0, #256]
    ldp q18, q19, [x0, #288]
    ldp q20, q21, [x0, #320]
    ldp q22, q23, [x0, #352]
    ldp q24, q25, [x0, #384]
    ldp q26, q27, [x0, #416]
    ldp q28, q29, [x0, #448]
    ldp q30, q31, [x0, #480]
    add x3, x0, #512
    ldp x1, x2, [x3]
    msr fpcr, x1
    msr fpsr, x2
    ret

