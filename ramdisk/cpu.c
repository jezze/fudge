#include <stdout.h>
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

    stdout_write("Vendor: ");
    stdout_write(vendor);
    stdout_write("\n");

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    stdout_write("Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        stdout_write("FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        stdout_write("MMX ");

    stdout_write("\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    stdout_write("Brand: ");
    stdout_write(brand);
    stdout_write("\n");

}

