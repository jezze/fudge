#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/mmu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include <modules/arch/x86/uart/uart.h>

unsigned int *pml4t = (void *)0x4000;
unsigned int *pdpt = (void *)0x5000;
unsigned int *pdt = (void *)0x6000;
unsigned int *pt = (void *)0x7000;

static void idmap(unsigned int x)
{

    unsigned int *c;
    unsigned int i;

    pdt[x] = (unsigned int)&pt[x] | 3;
    c = &pt[x];

    for (i = x * 512; i < (x + 1) * 512; i++)
    {

        *c = (i * 0x1000) | 3;
        c++;
        *c = 0;
        c++;

    }

}

static void setuptables(void)
{

    buffer_clear(pml4t, 0x1000);

    pml4t[0] = (unsigned int)pdpt | 3;
    pdpt[0] = (unsigned int)pdt | 3;

    idmap(0);
    idmap(1);
    idmap(2);
    idmap(3);
    idmap(4);
    idmap(5);
    idmap(6);
    idmap(7);

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

    uart_put('a');
    mmu_disable();
    uart_put('b');
    setuptables();
    uart_put('c');
    cpu_setcr3((unsigned int)pml4t);
    uart_put('d');
    cpu_setcr4(cpu_getcr4() | (1 << 5));
    uart_put('e');
    setlongmode();
    uart_put('f');
    mmu_enable();
    uart_put('g');

}

