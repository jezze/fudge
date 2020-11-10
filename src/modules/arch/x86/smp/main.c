#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/mmu.h>
#include <kernel/x86/arch.h>
#include <modules/system/system.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/pat/pat.h>
#include <modules/arch/x86/pit/pit.h>
#include "smp.h"

#define INIT16ADDRESS                   0x00008000
#define INIT32ADDRESS                   0x00008200

static struct arch_tss tss[256];
static struct core cores[256];
static struct list corelist;

static void detect(struct acpi_madt *madt)
{

    unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
    unsigned int madtend = (unsigned int)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

        if (entry->type == 0)
        {

            struct acpi_madt_apic *apic = (struct acpi_madt_apic *)entry;

            if (apic->id)
            {

                apic_sendint(apic->id, APIC_ICR_TYPE_INIT | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0x00);
                pit_wait(10);
                apic_sendint(apic->id, APIC_ICR_TYPE_SIPI | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | (INIT16ADDRESS >> 12));

            }

        }

        madttable += entry->length;

    }

}

static struct core *coreget(void)
{

    return &cores[apic_getid()];

}

static void coreassign(struct task *task)
{

    struct list_item *current = corelist.head;
    struct core *core = current->data;

    list_add(&core->tasks, &task->item);
    apic_sendint(core->id, APIC_ICR_TYPE_NORMAL | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0xFE);

}

void smp_setupbp(unsigned int stack, struct task *task, struct list *tasks)
{

    unsigned int id = apic_getid();
    struct list_item *current;

    core_init(&cores[id], id, stack, task);
    core_register(&cores[id]);
    arch_configuretss(&tss[id], cores[id].id, cores[id].sp);
    apic_setup_bp();
    list_add(&corelist, &cores[id].item);

    while ((current = list_pickhead(tasks)))
        list_add(&cores[id].tasks, current);

}

void smp_setupap(unsigned int stack)
{

    unsigned int id = apic_getid();

    core_init(&cores[id], id, stack, 0);
    core_register(&cores[id]);
    arch_configuretss(&tss[id], cores[id].id, cores[id].sp);
    mmu_setdirectory((struct mmu_directory *)ARCH_MMUKERNELADDRESS);
    mmu_enable();
    apic_setup_ap();
    pat_setup();
    list_add(&corelist, &cores[id].item);
    arch_leave(&cores[id]);

}

void module_init(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");
    struct core *core = kernel_getcore();

    smp_setupbp(core->sp, core->task, &core->tasks);
    kernel_setcallback(coreget, coreassign);

    if (!madt)
        return;

    memory_copy((void *)INIT16ADDRESS, (void *)(unsigned int)smp_begin16, (unsigned int)smp_end16 - (unsigned int)smp_begin16);
    memory_copy((void *)INIT32ADDRESS, (void *)(unsigned int)smp_begin32, (unsigned int)smp_end32 - (unsigned int)smp_begin32);
    smp_prep(ARCH_KERNELSTACKADDRESS + 2 * ARCH_KERNELSTACKSIZE);
    detect(madt);

}

