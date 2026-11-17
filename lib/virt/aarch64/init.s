.set STACK_SIZE,                        0x4000
.set HCR_RW,                            0x80000000
.set SPSR_EL1,                          0x3C5
.set SCTLR,                             0x30D00800
.set BLOCK_DEVICE,                      0x00000401
.set BLOCK_NORMAL,                      0x40000705

.section .text.boot

# Entered in EL1 (QEMU) or EL2 (firmware, QEMU with virtualization=on), which drops to EL1
.global _start
_start:
    msr daifset, #0xF
    mrs x0, currentel
    lsr x0, x0, #2
    cmp x0, #2
    b.ne start_el1
    ldr x0, =HCR_RW
    msr hcr_el2, x0
    mrs x0, cnthctl_el2
    orr x0, x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr
    mov x0, #SPSR_EL1
    msr spsr_el2, x0
    adr x0, start_el1
    msr elr_el2, x0
    eret

start_el1:
    ldr x0, =SCTLR
    msr sctlr_el1, x0
    msr cpacr_el1, xzr
    isb
    ldr x0, =stack
    add x0, x0, #STACK_SIZE
    mov sp, x0
    ldr x0, =boottable
    ldr x1, =BLOCK_DEVICE
    str x1, [x0]
    ldr x1, =BLOCK_NORMAL
    str x1, [x0, #8]
    bl cpu_enablemmu
    bl virt_setup

halt:
    wfi
    b halt

.section .bss

# Until arch_setup2 has the kernel's tables: the first GB as device memory, the second (RAM) as normal memory
.balign 4096
boottable:
    .space 4096

.balign 16
stack:
    .space STACK_SIZE

