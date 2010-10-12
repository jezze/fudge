#include <write.h>
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

    write_string("Vendor: ");
    write_string(vendor);
    write_string("\n");

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    write_string("Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        write_string("FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        write_string("MMX ");

    write_string("\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    write_string("Brand: ");
    write_string(brand);
    write_string("\n");

}

