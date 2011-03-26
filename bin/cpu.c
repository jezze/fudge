#include <file.h>
#include <session.h>
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

    file_write_string(session_get_out(), "Vendor: ");
    file_write_string(session_get_out(), vendor);
    file_write_string(session_get_out(), "\n");

    // ECX FEATURES

    //int ecx = cpu_get_feature_ecx();

    // EDX FEATURES

    unsigned int edx = cpu_get_feature_edx();

    file_write_string(session_get_out(), "Features: ");

    if (edx & CPU_FEATURE_EDX_FPU)
        file_write_string(session_get_out(), "FDU ");

    if (edx & CPU_FEATURE_EDX_MMX)
        file_write_string(session_get_out(), "MMX ");

    file_write_string(session_get_out(), "\n");

    // BRAND

    char brand[48];

    cpu_get_brand(brand);

    file_write_string(session_get_out(), "Brand: ");
    file_write_string(session_get_out(), brand);
    file_write_string(session_get_out(), "\n");

}

