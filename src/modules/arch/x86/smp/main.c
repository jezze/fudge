#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/mmu.h>
#include <kernel/x86/arch.h>
#include <modules/system/system.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/pat/pat.h>
#include <modules/arch/x86/pit/pit.h>
#include "smp.h"

#define INIT16PHYSICAL                  0x00008000
#define INIT32PHYSICAL                  0x00008200

static struct corerow {struct arch_tss tss; struct core core; struct list_item item;} corerows[256];
static struct list corelist;

static void enable(void)
{

    unsigned int id = apic_getid();
    unsigned int i;

    for (i = 0; i < 256; i++)
    {

        if (apic_checklapic(i))
        {

            if (i == id)
                continue;

            apic_sendint(i, APIC_REG_ICR_TYPE_INIT | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | 0x00);
            pit_wait(10);
            apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | (INIT16PHYSICAL >> 12));

        }

    }

}

static struct core *coreget(void)
{

    return &corerows[apic_getid()].core;

}

static void coreassign(struct list_item *item)
{

    spinlock_acquire(&corelist.spinlock);

    {

        struct list_item *coreitem = corelist.head;
        struct core *core = coreitem->data;

        list_move_unsafe(&corelist, &corelist, coreitem);
        list_add(&core->tasks, item);

        if (coreget() != core && core->state == CORE_STATE_ASLEEP)
            apic_sendint(core->id, APIC_REG_ICR_TYPE_NORMAL | APIC_REG_ICR_MODE_PHYSICAL | APIC_REG_ICR_LEVEL_ASSERT | APIC_REG_ICR_TRIGGER_EDGE | APIC_REG_ICR_TARGET_NORMAL | 0xFE);

    }

    spinlock_release(&corelist.spinlock);

}

void smp_setupbp(unsigned int stack, unsigned int task, struct list *tasks)
{

    unsigned int id = apic_getid();
    struct corerow *corerow = &corerows[id];
    struct list_item *taskitem;

    DEBUG_FMT1(DEBUG_INFO, "bp id %u", &id);
    DEBUG_FMT1(DEBUG_INFO, "bp stack 0x%H8u", &stack);

    core_init(&corerow->core, id, stack);
    core_register(&corerow->core);

    corerow->core.task = task;

    arch_configuretss(&corerow->tss, corerow->core.id, corerow->core.sp);
    apic_setup_bp();
    list_inititem(&corerow->item, &corerow->core);
    list_add(&corelist, &corerow->item);

    while ((taskitem = list_pickhead(tasks)))
        list_add(&corerow->core.tasks, taskitem);

    corerow->core.state = CORE_STATE_AWAKE;

}

void smp_setupap(unsigned int stack)
{

    unsigned int id = apic_getid();
    struct corerow *corerow = &corerows[id];

    DEBUG_FMT1(DEBUG_INFO, "ap id %u", &id);
    DEBUG_FMT1(DEBUG_INFO, "ap stack 0x%H8u", &stack);

    core_init(&corerow->core, id, stack);
    core_register(&corerow->core);
    arch_configuretss(&corerow->tss, corerow->core.id, corerow->core.sp);
    mmu_setdirectory((struct mmu_directory *)ARCH_KERNELMMUPHYSICAL);
    mmu_enable();
    apic_setup_ap();
    pat_setup();
    list_inititem(&corerow->item, &corerow->core);
    list_add(&corelist, &corerow->item);

    /*corerow->core.state = CORE_STATE_AWAKE;*/

    arch_leave(&corerow->core);

}

void module_init(void)
{

    struct core *core = kernel_getcore();

    list_init(&corelist);
    smp_setupbp(core->sp, core->task, &core->tasks);
    kernel_setcallback(coreget, coreassign);
    buffer_copy((void *)INIT16PHYSICAL, (void *)(unsigned int)smp_begin16, (unsigned int)smp_end16 - (unsigned int)smp_begin16);
    buffer_copy((void *)INIT32PHYSICAL, (void *)(unsigned int)smp_begin32, (unsigned int)smp_end32 - (unsigned int)smp_begin32);
    smp_prep(ARCH_KERNELSTACKPHYSICAL + 2 * ARCH_KERNELSTACKSIZE);
    enable();

}

