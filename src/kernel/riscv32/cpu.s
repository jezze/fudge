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

# SBI timer extension (TIME), set_timer, with the high half from time: a time below now has wrapped
.global cpu_settimer
cpu_settimer:
    csrr a1, timeh
    csrr t0, time
    csrr t1, timeh
    bne a1, t1, cpu_settimer
    bgeu a0, t0, settimer
    addi a1, a1, 1

settimer:
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

