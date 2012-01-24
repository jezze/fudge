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
        log_write("FPU ");

    if (edx & CPU_FEATURE_EDX_VME)
        log_write("VME ");

    if (edx & CPU_FEATURE_EDX_DE)
        log_write("DE ");

    if (edx & CPU_FEATURE_EDX_PSE)
        log_write("PSE ");

    if (edx & CPU_FEATURE_EDX_TSC)
        log_write("TSC ");

    if (edx & CPU_FEATURE_EDX_MSR)
        log_write("MSR ");

    if (edx & CPU_FEATURE_EDX_PAE)
        log_write("PAE ");

    if (edx & CPU_FEATURE_EDX_MCE)
        log_write("MCE ");

    if (edx & CPU_FEATURE_EDX_CX8)
        log_write("CX8 ");

    if (edx & CPU_FEATURE_EDX_APIC)
        log_write("APIC ");

    if (edx & CPU_FEATURE_EDX_SEP)
        log_write("SEP ");

    if (edx & CPU_FEATURE_EDX_MTRR)
        log_write("MTRR ");

    if (edx & CPU_FEATURE_EDX_PGE)
        log_write("PGE ");

    if (edx & CPU_FEATURE_EDX_MCA)
        log_write("MCA ");

    if (edx & CPU_FEATURE_EDX_CMOV)
        log_write("CMOV ");

    if (edx & CPU_FEATURE_EDX_PAT)
        log_write("PAT ");

    if (edx & CPU_FEATURE_EDX_PSE36)
        log_write("PSE36 ");

    if (edx & CPU_FEATURE_EDX_PSN)
        log_write("PSN ");

    if (edx & CPU_FEATURE_EDX_CLF)
        log_write("CLF ");

    if (edx & CPU_FEATURE_EDX_DTES)
        log_write("DTES ");

    if (edx & CPU_FEATURE_EDX_ACPI)
        log_write("ACPI ");

    if (edx & CPU_FEATURE_EDX_MMX)
        log_write("MMX ");

    if (edx & CPU_FEATURE_EDX_FXSR)
        log_write("FXSR ");

    if (edx & CPU_FEATURE_EDX_SSE)
        log_write("SSE ");

    if (edx & CPU_FEATURE_EDX_SSE2)
        log_write("SSE2 ");

    if (edx & CPU_FEATURE_EDX_SS)
        log_write("SS ");

    if (edx & CPU_FEATURE_EDX_HTT)
        log_write("HTT ");

    if (edx & CPU_FEATURE_EDX_TM1)
        log_write("TM1 ");

    if (edx & CPU_FEATURE_EDX_IA64)
        log_write("IA64 ");

    if (edx & CPU_FEATURE_EDX_PBE)
        log_write("PBE ");

    log_write("\n");

    char brand[48];

    cpu_get_brand(brand);

    log_write("[cpu] Brand: %s\n", brand);

}

