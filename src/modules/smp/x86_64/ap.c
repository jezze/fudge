#include <fudge.h>
#include <kernel.h>
#include <kernel/x86_64/cpu.h>
#include <kernel/x86_64/gdt.h>
#include <kernel/x86_64/arch.h>
#include <modules/apic/apic.h>
#include <modules/pat/pat.h>
#include <modules/sse/sse.h>

static struct gdt_pointer *gdt = (struct gdt_pointer *)ARCH_GDT_BASE;
static void *idt = (void *)ARCH_IDT_BASE;

void smp_setupap(unsigned int icore)
{

    struct core *core = pool_getcore(icore);

    cpu_setgdt(gdt, gdt_getselector(gdt, ARCH_KCODE), gdt_getselector(gdt, ARCH_KDATA));
    cpu_setidt(idt);

    if (core)
        core_register(core);

    cpu_settss(gdt_getselector(gdt, ARCH_TSS + icore * 2));
    apic_setup_ap();
    pat_setup();
    sse_setup();
    arch_leave();

}

