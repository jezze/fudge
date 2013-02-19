#include <fudge/kernel.h>
#include <kernel/error.h>
#include <kernel/runtime.h>
#include <kernel/kernel.h>
#include <kernel/syscall.h>
#include "arch.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "mmu.h"
#include "tss.h"

static struct arch_state state;
static struct mmu_directory directory;
static struct mmu_table tables[3];

void arch_pagefault(struct arch_registers_mmu *registers)
{

    unsigned int address = cpu_get_cr2();

    error_register(1, address);
    error_register(2, registers->type);
    error_panic("PAGE FAULT", __FILE__, __LINE__);

}

unsigned short arch_syscall(struct arch_registers_syscall *registers)
{

    state.running->registers.ip = registers->interrupt.eip;
    state.running->registers.sp = registers->interrupt.esp;
    state.running->registers.fp = registers->general.ebp;
    state.running->registers.status = syscall_raise(registers->general.eax, state.running);

    if (state.running->container->notify_interrupt)
        state.running = state.running->container->notify_interrupt(state.running, registers->general.eax);

    if (state.running->status.used && !state.running->status.idle)
    {

        registers->interrupt.cs = state.selectors.ucode;
        registers->interrupt.eip = state.running->registers.ip;
        registers->interrupt.esp = state.running->registers.sp;
        registers->general.ebp = state.running->registers.fp;
        registers->general.eax = state.running->registers.status;

        return state.selectors.udata;

    }

    registers->interrupt.cs = state.selectors.kcode;
    registers->interrupt.eip = (unsigned int)arch_halt;
    registers->interrupt.esp = ARCH_STACK_BASE;
    registers->general.ebp = 0;
    registers->general.eax = 0;

    return state.selectors.kdata;

}

static void setup_tables(struct gdt_pointer *gdtp, struct idt_pointer *idtp)
{

    struct tss_pointer *tssp = tss_setup_pointer();
    unsigned int tss = gdt_set_entry(gdtp, GDT_INDEX_TSS, (unsigned int)tssp->base, (unsigned int)tssp->base + tssp->limit, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    state.selectors.kcode = gdt_set_entry(gdtp, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.ucode = gdt_set_entry(gdtp, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.kdata = gdt_set_entry(gdtp, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.udata = gdt_set_entry(gdtp, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    tss_set_entry(tssp, TSS_INDEX_DEFAULT, state.selectors.kdata, ARCH_STACK_BASE);
    cpu_set_gdt(gdtp);
    cpu_set_idt(idtp);
    cpu_set_tss(tss);

}

static void setup_routines(struct idt_pointer *idtp)
{

    idt_set_entry(idtp, IDT_INDEX_PF, arch_isr_pagefault, state.selectors.kcode, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, IDT_INDEX_SYSCALL, arch_isr_syscall, state.selectors.kcode, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);

}

static void setup_mmu()
{

    mmu_map_memory(&directory, &tables[0], ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_map_memory(&directory, &tables[1], RUNTIME_TASKADDRESS_PHYSICAL, RUNTIME_TASKADDRESS_VIRTUAL, RUNTIME_TASKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_map_memory(&directory, &tables[2], RUNTIME_STACKADDRESS_PHYSICAL, RUNTIME_STACKADDRESS_VIRTUAL - RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_load_memory(&directory);
    mmu_enable();

}

void arch_setup(unsigned int ramdiskc, void **ramdiskv)
{

    struct gdt_pointer *gdtp = gdt_setup_pointer();
    struct idt_pointer *idtp = idt_setup_pointer();

    setup_tables(gdtp, idtp);
    setup_routines(idtp);
    setup_mmu();

    state.running = kernel_setup(ramdiskc, ramdiskv);

    arch_disable_pic();
    arch_usermode(state.selectors.ucode, state.selectors.udata, state.running->registers.ip, state.running->registers.sp);

}

