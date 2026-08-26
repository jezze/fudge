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

#define INIT16ADDRESS                   0x00008000
#define INIT32ADDRESS                   0x00008200

extern void smp_begin16(void);
extern void smp_end16(void);
extern void smp_begin32(void);
extern void smp_end32(void);
static struct arch_tss tss[POOL_CORES];
static struct list usedcores;

static struct core *coreget(void)
{

    unsigned int id = apic_getid();

    return pool_getcore(id);

}

static void coreassign(unsigned int itask)
{

    unsigned int icore = pool_pickcorefrom(&usedcores);
    struct core *core = pool_getcore(icore);

    pool_placecore(icore, &usedcores);
    pool_placetask(itask, &core->tasks);

    if (core->id != apic_getid())
        apic_sendint(core->id, APIC_REG_ICR_LEVEL_ASSERT | 0xFE);

}

static void smp_setupbp(unsigned int icore, unsigned int sp)
{

    arch_configuretss(&tss[icore], icore, sp);
    apic_setup_bp();

}

void smp_setupap(unsigned int icore, unsigned int sp)
{

    arch_configuretss(&tss[icore], icore, sp);
    apic_setup_ap();
    mmu_setdirectory(ARCH_MMUKERNELADDRESS);
    mmu_enable();
    pat_setup();
    arch_leave();

}

void module_init(void)
{

    unsigned int icore = apic_getid();
    unsigned int i;

    list_init(&usedcores);
    smp_setupbp(apic_getid(), 0x00602000);
    kernel_setcallback(coreget, coreassign);
    buffer_copy((void *)INIT16ADDRESS, (void *)(unsigned long)smp_begin16, (unsigned long)smp_end16 - (unsigned long)smp_begin16);
    buffer_copy((void *)INIT32ADDRESS, (void *)(unsigned long)smp_begin32, (unsigned long)smp_end32 - (unsigned long)smp_begin32);
    pic_disable();
    apic_setupisrs();

    for (i = 0; i < POOL_CORES; i++)
    {

        if (apic_checklapic(i))
        {

            if (i != icore)
            {

                pool_placecore(icore, &usedcores);
                apic_sendint(i, APIC_REG_ICR_TYPE_INIT | APIC_REG_ICR_LEVEL_ASSERT | 0x00);
                pit_wait(10);
                apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_LEVEL_ASSERT | (INIT16ADDRESS >> 12));
                pit_wait(1);
                apic_sendint(i, APIC_REG_ICR_TYPE_SIPI | APIC_REG_ICR_LEVEL_ASSERT | (INIT16ADDRESS >> 12));

            }

        }

    }

}

