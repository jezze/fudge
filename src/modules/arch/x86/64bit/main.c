#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>

int *pml4t = (void *)0x1000;
int *pdpt = (void *)0x2000;
int *pdt = (void *)0x3000;
int *pt = (void *)0x4000;

static void setuptables(void)
{

    unsigned int i;
    int *c = pt;

    memory_clear(pml4t, 0x1000);
    memory_clear(pml4t, 0x1000);
    memory_clear(pdt, 0x1000);
    memory_clear(pt, 0x200000);

    pml4t[0] = (unsigned int)pdpt | 3;
    pdpt[0] = (unsigned int)pdt | 3;
    pdt[0] = (unsigned int)pt | 3;

    for (i = 0; i < 512; i++)
    {

        *c = (i * 0x1000) | 3;
        c++;
        *c = 0;
        c++;

    }

}

static void setlongmode(void)
{

    struct msr_data msrdata;

    msr_get(0xC0000080, &msrdata);

    msrdata.eax |= (1 << 8);

    msr_set(0xC0000080, &msrdata);

}

static unsigned int check(void)
{

    struct cpuid_data data;

    cpuid_getdata(CPUID_LARGEST, &data);

    if (data.eax < CPUID_FEATURES1)
        return 0;

    cpuid_getdata(CPUID_FEATURES1, &data);

    if (!(data.edx & (1 << 29)))
        return 0;

    return 1;

}

void module_init(void)
{

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    if (!check())
        return;

    cpu_setcr0(cpu_getcr0() & ~0x80000000);
    setuptables();
    cpu_setcr3(0x1000);
    cpu_setcr4(cpu_getcr4() | (1 << 5));
    setlongmode();
    cpu_setcr0(cpu_getcr0() | 0x80000000);

    for (;;);

}

