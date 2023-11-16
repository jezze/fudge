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

            DEBUG_FMT1(DEBUG_INFO, "enable processor id %u", &i);

            apic_sendint(i, APIC_REG_ICR_TYPE_INIT | APIC_REG_ICR_LEVEL_ASSERT | 0x00);
            pit_wait(10);
            apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_LEVEL_ASSERT | (INIT16PHYSICAL >> 12));

        }

    }

}

static struct core *coreget(void)
{

    unsigned int directory = cpu_getcr3();
    unsigned int id;

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

    id = apic_getid();

    cpu_setcr3(directory);

    return &corerows[id].core;

}

static void coreassign(struct list_item *item)
{

    spinlock_acquire(&corelist.spinlock);

    {

        struct list_item *coreitem = corelist.head;
        struct core *core = coreitem->data;
        unsigned int directory = cpu_getcr3();

        list_move_unsafe(&corelist, &corelist, coreitem);
        list_add(&core->tasks, item);
        cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

        if (core->id != apic_getid())
            apic_sendint(core->id, APIC_REG_ICR_LEVEL_ASSERT | 0xFE);

        cpu_setcr3(directory);

    }

    spinlock_release(&corelist.spinlock);

}

static void smp_setupbp(unsigned int stack)
{

    struct core *core0 = kernel_getcore();
    unsigned int id = apic_getid();
    struct corerow *corerow = &corerows[id];

    DEBUG_FMT1(DEBUG_INFO, "bp id %u", &id);
    DEBUG_FMT1(DEBUG_INFO, "bp stack 0x%H8u", &stack);

    core_init(&corerow->core, id, stack);
    core_register(&corerow->core);
    core_migrate(&corerow->core, core0);

    arch_configuretss(&corerow->tss, corerow->core.id, corerow->core.sp);
    apic_setup_bp();
    list_inititem(&corerow->item, &corerow->core);
    list_add(&corelist, &corerow->item);

}

void smp_setupap(unsigned int stack)
{

    unsigned int id;
    struct corerow *corerow;

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);
    mmu_enable();

    id = apic_getid();
    corerow = &corerows[id];

    DEBUG_FMT1(DEBUG_INFO, "ap id %u", &id);
    DEBUG_FMT1(DEBUG_INFO, "ap stack 0x%H8u", &stack);

    core_init(&corerow->core, id, stack);
    core_register(&corerow->core);
    arch_configuretss(&corerow->tss, corerow->core.id, corerow->core.sp);
    apic_setup_ap();
    pat_setup();
    list_inititem(&corerow->item, &corerow->core);
    list_add(&corelist, &corerow->item);
    arch_leave();

}

void module_init(void)
{

    list_init(&corelist);
    smp_setupbp(ARCH_KERNELSTACKPHYSICAL + ARCH_KERNELSTACKSIZE);
    kernel_setcallback(coreget, coreassign);
    buffer_copy((void *)INIT16PHYSICAL, (void *)(unsigned int)smp_begin16, (unsigned int)smp_end16 - (unsigned int)smp_begin16);
    buffer_copy((void *)INIT32PHYSICAL, (void *)(unsigned int)smp_begin32, (unsigned int)smp_end32 - (unsigned int)smp_begin32);
    smp_prep(ARCH_KERNELSTACKPHYSICAL + 2 * ARCH_KERNELSTACKSIZE);
    pic_disable();
    apic_setupisrs();
    enable();

}

