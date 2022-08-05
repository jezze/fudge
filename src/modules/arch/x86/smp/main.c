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

#define INIT16PHYSICAL                  0x00008000
#define INIT32PHYSICAL                  0x00008200

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

                apic_sendint(apic->id, APIC_REG_ICR_TYPE_INIT | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | 0x00);
                pit_wait(10);
                apic_sendint(apic->id, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | (INIT16PHYSICAL >> 12));

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

    struct list_item *coreitem = list_pickhead(&corelist);

    if (coreitem)
    {

        struct core *core = coreitem->data;

        list_add(&core->tasks, &task->item);
        list_add(&corelist, coreitem);
        apic_sendint(core->id, APIC_REG_ICR_TYPE_NORMAL | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | 0xFE);

    }

}

void smp_setupbp(unsigned int stack, struct list *tasks)
{

    unsigned int id = apic_getid();
    struct list_item *taskitem;

    core_init(&cores[id], id, stack);
    core_register(&cores[id]);
    arch_configuretss(&tss[id], cores[id].id, cores[id].sp);
    apic_setup_bp();
    list_add(&corelist, &cores[id].item);

    while ((taskitem = list_pickhead(tasks)))
        list_add(&cores[id].tasks, taskitem);

}

void smp_setupap(unsigned int stack)
{

    unsigned int id = apic_getid();

    core_init(&cores[id], id, stack);
    core_register(&cores[id]);
    arch_configuretss(&tss[id], cores[id].id, cores[id].sp);
    mmu_setdirectory((struct mmu_directory *)ARCH_KERNELMMUPHYSICAL);
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

    list_init(&corelist);
    smp_setupbp(core->sp, &core->tasks);
    kernel_setcallback(coreget, coreassign);

    if (!madt)
        return;

    buffer_copy((void *)INIT16PHYSICAL, (void *)(unsigned int)smp_begin16, (unsigned int)smp_end16 - (unsigned int)smp_begin16);
    buffer_copy((void *)INIT32PHYSICAL, (void *)(unsigned int)smp_begin32, (unsigned int)smp_end32 - (unsigned int)smp_begin32);
    smp_prep(ARCH_KERNELSTACKPHYSICAL + 2 * ARCH_KERNELSTACKSIZE);
    detect(madt);

}

