#ifndef CPU_H
#define CPU_H

#define CPU_VENDOR_AMD              "AuthenticAMD"
#define CPU_VENDOR_AMD_LEGACY       "AMDisbetter!"
#define CPU_VENDOR_CENTAUR          "CentaurHauls"
#define CPU_VENDOR_CYRIX            "CyrixInstead"
#define CPU_VENDOR_INTEL            "GenuineIntel"
#define CPU_VENDOR_NEXGEN           "NexGenDriven"
#define CPU_VENDOR_NSC              "Geode by NSC"
#define CPU_VENDOR_RISE             "RiseRiseRise"
#define CPU_VENDOR_SIS              "SiS SiS SiS "
#define CPU_VENDOR_TRANSMETA        "GeniuneTMx86"
#define CPU_VENDOR_TRANSMETA_LEGACY "TransmetaCPU"
#define CPU_VENDOR_UMC              "UMC UMC UMC "
#define CPU_VENDOR_VIA              "CentaurHauls"

#define CPU_FEATURE_ECX_SSE3    1 << 0
#define CPU_FEATURE_ECX_PCLMUL  1 << 1
#define CPU_FEATURE_ECX_DTES64  1 << 2
#define CPU_FEATURE_ECX_MONITOR 1 << 3
#define CPU_FEATURE_ECX_DS_CPL  1 << 4
#define CPU_FEATURE_ECX_VMX     1 << 5
#define CPU_FEATURE_ECX_SMX     1 << 6
#define CPU_FEATURE_ECX_EST     1 << 7
#define CPU_FEATURE_ECX_TM2     1 << 8
#define CPU_FEATURE_ECX_SSSE3   1 << 9
#define CPU_FEATURE_ECX_CID     1 << 10
#define CPU_FEATURE_ECX_FMA     1 << 12
#define CPU_FEATURE_ECX_CX16    1 << 13
#define CPU_FEATURE_ECX_ETPRD   1 << 14
#define CPU_FEATURE_ECX_PDCM    1 << 15
#define CPU_FEATURE_ECX_DCA     1 << 18
#define CPU_FEATURE_ECX_SSE4_1  1 << 19
#define CPU_FEATURE_ECX_SSE4_2  1 << 20
#define CPU_FEATURE_ECX_x2APIC  1 << 21
#define CPU_FEATURE_ECX_MOVBE   1 << 22
#define CPU_FEATURE_ECX_POPCNT  1 << 23
#define CPU_FEATURE_ECX_XSAVE   1 << 26
#define CPU_FEATURE_ECX_OSXSAVE 1 << 27
#define CPU_FEATURE_ECX_AVX     1 << 28

#define CPU_FEATURE_EDX_FPU     1 << 0
#define CPU_FEATURE_EDX_VME     1 << 1
#define CPU_FEATURE_EDX_DE      1 << 2
#define CPU_FEATURE_EDX_PSE     1 << 3
#define CPU_FEATURE_EDX_TSC     1 << 4
#define CPU_FEATURE_EDX_MSR     1 << 5
#define CPU_FEATURE_EDX_PAE     1 << 6
#define CPU_FEATURE_EDX_MCE     1 << 7
#define CPU_FEATURE_EDX_CX8     1 << 8
#define CPU_FEATURE_EDX_APIC    1 << 9
#define CPU_FEATURE_EDX_SEP     1 << 11
#define CPU_FEATURE_EDX_MTRR    1 << 12
#define CPU_FEATURE_EDX_PGE     1 << 13
#define CPU_FEATURE_EDX_MCA     1 << 14
#define CPU_FEATURE_EDX_CMOV    1 << 15
#define CPU_FEATURE_EDX_PAT     1 << 16
#define CPU_FEATURE_EDX_PSE36   1 << 17
#define CPU_FEATURE_EDX_PSN     1 << 18
#define CPU_FEATURE_EDX_CLF     1 << 19
#define CPU_FEATURE_EDX_DTES    1 << 21
#define CPU_FEATURE_EDX_ACPI    1 << 22
#define CPU_FEATURE_EDX_MMX     1 << 23
#define CPU_FEATURE_EDX_FXSR    1 << 24
#define CPU_FEATURE_EDX_SSE     1 << 25
#define CPU_FEATURE_EDX_SSE2    1 << 26
#define CPU_FEATURE_EDX_SS      1 << 27
#define CPU_FEATURE_EDX_HTT     1 << 28
#define CPU_FEATURE_EDX_TM1     1 << 29
#define CPU_FEATURE_EDX_IA64    1 << 30
#define CPU_FEATURE_EDX_PBE     1 << 31

extern unsigned int cpu_check_cpuid();
extern unsigned int cpu_get_feature_ecx();
extern unsigned int cpu_get_feature_edx();
extern void cpu_get_vendor(char *destination);
extern void cpu_get_brand(char *destination);

#endif

