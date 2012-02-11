#ifndef MODULES_CPUID_H
#define MODULES_CPUID_H

#define CPUID_VENDOR_AMD              "AuthenticAMD"
#define CPUID_VENDOR_AMD_LEGACY       "AMDisbetter!"
#define CPUID_VENDOR_CENTAUR          "CentaurHauls"
#define CPUID_VENDOR_CYRIX            "CyrixInstead"
#define CPUID_VENDOR_INTEL            "GenuineIntel"
#define CPUID_VENDOR_NEXGEN           "NexGenDriven"
#define CPUID_VENDOR_NSC              "Geode by NSC"
#define CPUID_VENDOR_RISE             "RiseRiseRise"
#define CPUID_VENDOR_SIS              "SiS SiS SiS "
#define CPUID_VENDOR_TRANSMETA        "GeniuneTMx86"
#define CPUID_VENDOR_TRANSMETA_LEGACY "TransmetaCPUID"
#define CPUID_VENDOR_UMC              "UMC UMC UMC "
#define CPUID_VENDOR_VIA              "CentaurHauls"

#define CPUID_FEATURE_ECX_SSE3    1 << 0
#define CPUID_FEATURE_ECX_PCLMUL  1 << 1
#define CPUID_FEATURE_ECX_DTES64  1 << 2
#define CPUID_FEATURE_ECX_MONITOR 1 << 3
#define CPUID_FEATURE_ECX_DS_CPL  1 << 4
#define CPUID_FEATURE_ECX_VMX     1 << 5
#define CPUID_FEATURE_ECX_SMX     1 << 6
#define CPUID_FEATURE_ECX_EST     1 << 7
#define CPUID_FEATURE_ECX_TM2     1 << 8
#define CPUID_FEATURE_ECX_SSSE3   1 << 9
#define CPUID_FEATURE_ECX_CID     1 << 10
#define CPUID_FEATURE_ECX_FMA     1 << 12
#define CPUID_FEATURE_ECX_CX16    1 << 13
#define CPUID_FEATURE_ECX_ETPRD   1 << 14
#define CPUID_FEATURE_ECX_PDCM    1 << 15
#define CPUID_FEATURE_ECX_DCA     1 << 18
#define CPUID_FEATURE_ECX_SSE4_1  1 << 19
#define CPUID_FEATURE_ECX_SSE4_2  1 << 20
#define CPUID_FEATURE_ECX_x2APIC  1 << 21
#define CPUID_FEATURE_ECX_MOVBE   1 << 22
#define CPUID_FEATURE_ECX_POPCNT  1 << 23
#define CPUID_FEATURE_ECX_XSAVE   1 << 26
#define CPUID_FEATURE_ECX_OSXSAVE 1 << 27
#define CPUID_FEATURE_ECX_AVX     1 << 28

#define CPUID_FEATURE_EDX_FPU     1 << 0
#define CPUID_FEATURE_EDX_VME     1 << 1
#define CPUID_FEATURE_EDX_DE      1 << 2
#define CPUID_FEATURE_EDX_PSE     1 << 3
#define CPUID_FEATURE_EDX_TSC     1 << 4
#define CPUID_FEATURE_EDX_MSR     1 << 5
#define CPUID_FEATURE_EDX_PAE     1 << 6
#define CPUID_FEATURE_EDX_MCE     1 << 7
#define CPUID_FEATURE_EDX_CX8     1 << 8
#define CPUID_FEATURE_EDX_APIC    1 << 9
#define CPUID_FEATURE_EDX_SEP     1 << 11
#define CPUID_FEATURE_EDX_MTRR    1 << 12
#define CPUID_FEATURE_EDX_PGE     1 << 13
#define CPUID_FEATURE_EDX_MCA     1 << 14
#define CPUID_FEATURE_EDX_CMOV    1 << 15
#define CPUID_FEATURE_EDX_PAT     1 << 16
#define CPUID_FEATURE_EDX_PSE36   1 << 17
#define CPUID_FEATURE_EDX_PSN     1 << 18
#define CPUID_FEATURE_EDX_CLF     1 << 19
#define CPUID_FEATURE_EDX_DTES    1 << 21
#define CPUID_FEATURE_EDX_ACPI    1 << 22
#define CPUID_FEATURE_EDX_MMX     1 << 23
#define CPUID_FEATURE_EDX_FXSR    1 << 24
#define CPUID_FEATURE_EDX_SSE     1 << 25
#define CPUID_FEATURE_EDX_SSE2    1 << 26
#define CPUID_FEATURE_EDX_SS      1 << 27
#define CPUID_FEATURE_EDX_HTT     1 << 28
#define CPUID_FEATURE_EDX_TM1     1 << 29
#define CPUID_FEATURE_EDX_IA64    1 << 30
#define CPUID_FEATURE_EDX_PBE     1 << 31

#define CPUID_DRIVER_TYPE 0x9421

struct cpuid_driver
{

    struct modules_driver base;

};

extern unsigned int cpuid_check_cpuid();
extern unsigned int cpuid_get_feature_ecx();
extern unsigned int cpuid_get_feature_edx();
extern void cpuid_get_vendor(char *destination);
extern void cpuid_get_brand(char *destination);
extern void cpuid_driver_init(struct cpuid_driver *driver);

#endif

