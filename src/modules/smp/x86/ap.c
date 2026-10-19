#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/arch.h>
#include <modules/apic/apic.h>
#include <modules/pat/pat.h>
#include <modules/sse/sse.h>

static struct gdt_pointer *gdt = (struct gdt_pointer *)ARCH_GDT_BASE;

void smp_setupap(unsigned int icore)
{

    struct core *core = pool_getcore(icore);

    if (core)
        core_register(core);

    cpu_settss(gdt_getselector(gdt, ARCH_TSS + icore));
    apic_setup_ap();
    pat_setup();
    sse_setup();
    arch_leave();

}

