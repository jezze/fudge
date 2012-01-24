#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/cpu/cpu.h>

void init()
{

    if (!cpu_check_cpuid())
        return;

    char vendor[16];
    vendor[12] = '\0';

    cpu_get_vendor(vendor);

    log_write("[cpu] Vendor: %s\n", vendor);

    //int ecx = cpu_get_feature_ecx();

    unsigned int edx = cpu_get_feature_edx();

    log_write("[cpu] Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        log_write("FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        log_write("MMX ");

    log_write("\n");

    char brand[48];

    cpu_get_brand(brand);

    log_write("[cpu] Brand: %s\n", brand);

}

