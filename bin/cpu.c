#include <fudge.h>
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

    file_write_format(FILE_STDOUT, "Vendor: %s\n", vendor);

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    file_write_format(FILE_STDOUT, "Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        file_write_format(FILE_STDOUT, "FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        file_write_format(FILE_STDOUT, "MMX ");

    file_write_format(FILE_STDOUT, "\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    file_write_format(FILE_STDOUT, "Brand: %s\n", brand);

    call_execute("/shell", 1, 0);

}

