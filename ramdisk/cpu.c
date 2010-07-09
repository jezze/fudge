#include <vfs.h>
#include <call.h>
#include "cpu.h"

void main(int argc, char *argv[])
{

    // CHECK CPUID

    if (!cpu_check_cpuid())
        return;

    // VENDOR

    char vendor[16];
    vendor[12] = '\0';

    cpu_get_vendor(vendor);

    call_puts("Vendor: ");
    call_puts(vendor);
    call_puts("\n");

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    call_puts("Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        call_puts("FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        call_puts("MMX ");

    call_puts("\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    call_puts("Brand: ");
    call_puts(brand);
    call_puts("\n");

}

