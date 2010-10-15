#include <file.h>
#include <vfs.h>
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

    file_write("Vendor: ");
    file_write(vendor);
    file_write("\n");

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    file_write("Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        file_write("FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        file_write("MMX ");

    file_write("\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    file_write("Brand: ");
    file_write(brand);
    file_write("\n");

}

