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

static struct arch_x86 x86;
static struct mmu_directory directory;
static struct mmu_table tables[3];

void arch_pagefault(struct arch_mmu_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    error_register(1, address);
    error_register(2, registers->type);

}

unsigned short arch_syscall(struct arch_syscall_registers *registers)
{

    runtime_set_registers(x86.running, registers->interrupt.eip, registers->interrupt.esp, registers->general.ebp, registers->general.eax);

    x86.running->registers.status = syscall_raise(registers->general.eax, x86.running, (void *)registers->interrupt.esp);
    x86.running = x86.running->notify_interrupt(x86.running, registers->general.eax);

    if (x86.running->status.used && !x86.running->status.idle)
    {

        registers->interrupt.cs = x86.segments.cs3;
        registers->interrupt.eip = x86.running->registers.ip;
        registers->interrupt.esp = x86.running->registers.sp;
        registers->general.ebp = x86.running->registers.fp;
        registers->general.eax = x86.running->registers.status;

        return x86.segments.ds3;

    }

    registers->interrupt.cs = x86.segments.cs0;
    registers->interrupt.eip = (unsigned int)arch_halt;
    registers->interrupt.esp = ARCH_STACK_BASE;
    registers->general.ebp = 0;
    registers->general.eax = 0;

    return x86.segments.ds0;

}

static void setup_tables(struct gdt_pointer *gdtp, struct idt_pointer *idtp)
{

    struct tss_pointer *tssp = tss_setup_pointer();
    unsigned int tss0 = gdt_set_entry(gdtp, GDT_INDEX_TSS, (unsigned int)tssp->base, (unsigned int)tssp->base + sizeof (struct tss_entry) * TSS_ENTRY_SLOTS, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    x86.segments.cs0 = gdt_set_entry(gdtp, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    x86.segments.ds0 = gdt_set_entry(gdtp, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    x86.segments.cs3 = gdt_set_entry(gdtp, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_EXECUTE | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    x86.segments.ds3 = gdt_set_entry(gdtp, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    tss_set_entry(tssp, TSS_INDEX_NULL, x86.segments.ds0, ARCH_STACK_BASE);
    cpu_set_gdt(gdtp);
    cpu_set_idt(idtp);
    cpu_set_tss(tss0);

}

static void setup_routines(struct idt_pointer *idtp)
{

    idt_set_entry(idtp, IDT_INDEX_PF, arch_isr_pagefault, x86.segments.cs0, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, IDT_INDEX_SYSCALL, arch_isr_syscall, x86.segments.cs0, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);

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

    x86.running = kernel_setup(ramdiskc, ramdiskv);

    arch_disable_pic();
    arch_usermode(x86.segments.cs3, x86.segments.ds3, x86.running->registers.ip, x86.running->registers.sp);

}

