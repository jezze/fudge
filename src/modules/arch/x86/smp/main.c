#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/mmu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/pat/pat.h>
#include <modules/arch/x86/pit/pit.h>
#include "smp.h"

#define INIT16ADDRESS                   0x00008000
#define INIT32ADDRESS                   0x00008200

static struct arch_tss tss[POOL_CORES];
static struct list usedcores;

static struct core *coreget(void)
{

    unsigned int id = apic_getid();

    return pool_getcore(id);

}

static void coreassign(struct list_item *item)
{

    struct list_item *coreitem = list_pickhead(&usedcores);

    if (coreitem)
    {

        struct core *core = coreitem->data;

        list_add(&usedcores, coreitem);
        list_add(&core->tasks, item);

        if (core->id != apic_getid())
            apic_sendint(core->id, APIC_REG_ICR_LEVEL_ASSERT | 0xFE);

    }

}

static void smp_setupbp(void)
{

    unsigned int id = apic_getid();
    struct core *core = pool_getcore(id);
    struct list_item *coreitem = pool_getcoreitem(id);

    arch_configuretss(&tss[id], core->id, core->sp);
    apic_setup_bp();
    list_add(&usedcores, coreitem);

}

void smp_setupap(void)
{

    unsigned int id = apic_getid();
    struct core *core = pool_getcore(id);
    struct list_item *coreitem = pool_getcoreitem(id);

    arch_configuretss(&tss[id], core->id, core->sp);
    mmu_setdirectory(ARCH_MMUKERNELADDRESS);
    mmu_enable();
    pat_setup();
    apic_setup_ap();
    list_add(&usedcores, coreitem);
    arch_leave();

}

void module_init(void)
{

    unsigned int id = apic_getid();
    unsigned int i;

    list_init(&usedcores);
    smp_setupbp();
    kernel_setcallback(coreget, coreassign);
    buffer_copy((void *)INIT16ADDRESS, (void *)(unsigned long)smp_begin16, (unsigned long)smp_end16 - (unsigned long)smp_begin16);
    buffer_copy((void *)INIT32ADDRESS, (void *)(unsigned long)smp_begin32, (unsigned long)smp_end32 - (unsigned long)smp_begin32);
    pic_disable();
    apic_setupisrs();

    for (i = 0; i < POOL_CORES; i++)
    {

        if (apic_checklapic(i))
        {

            if (i != id)
            {

                apic_sendint(i, APIC_REG_ICR_TYPE_INIT | APIC_REG_ICR_LEVEL_ASSERT | 0x00);
                pit_wait(10);
                apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_LEVEL_ASSERT | (INIT16ADDRESS >> 12));
                pit_wait(1);
                apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_LEVEL_ASSERT | (INIT16ADDRESS >> 12));

            }

        }

    }

}

