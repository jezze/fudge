#ifndef MODULES_CPUID_H
#define MODULES_CPUID_H

#define CPUID_INSTRUCTION_VENDOR        0x00000000
#define CPUID_INSTRUCTION_FEATURES0     0x00000001
#define CPUID_INSTRUCTION_CACHE         0x00000002
#define CPUID_INSTRUCTION_SERIAL        0x00000003
#define CPUID_INSTRUCTION_DCACHE        0x00000004
#define CPUID_INSTRUCTION_MONITOR       0x00000005
#define CPUID_INSTRUCTION_THERMAL       0x00000006
#define CPUID_INSTRUCTION_DCA           0x00000009
#define CPUID_INSTRUCTION_PMON          0x0000000A
#define CPUID_INSTRUCTION_2XAPIC        0x0000000B
#define CPUID_INSTRUCTION_XSAVE         0x0000000D
#define CPUID_INSTRUCTION_LARGEST       0x80000000
#define CPUID_INSTRUCTION_FEATURES1     0x80000001
#define CPUID_INSTRUCTION_BRAND0        0x80000002
#define CPUID_INSTRUCTION_BRAND1        0x80000003
#define CPUID_INSTRUCTION_BRAND2        0x80000004
#define CPUID_INSTRUCTION_L2CACHE       0x80000006
#define CPUID_INSTRUCTION_APM           0x80000007
#define CPUID_INSTRUCTION_MEMSIZE       0x80000008

#define CPUID_VENDOR_AMD                "AuthenticAMD"
#define CPUID_VENDOR_AMD_LEGACY         "AMDisbetter!"
#define CPUID_VENDOR_CENTAUR            "CentaurHauls"
#define CPUID_VENDOR_CYRIX              "CyrixInstead"
#define CPUID_VENDOR_INTEL              "GenuineIntel"
#define CPUID_VENDOR_NEXGEN             "NexGenDriven"
#define CPUID_VENDOR_NSC                "Geode by NSC"
#define CPUID_VENDOR_RISE               "RiseRiseRise"
#define CPUID_VENDOR_SIS                "SiS SiS SiS "
#define CPUID_VENDOR_TRANSMETA          "GeniuneTMx86"
#define CPUID_VENDOR_TRANSMETA_LEGACY   "TransmetaCPUID"
#define CPUID_VENDOR_UMC                "UMC UMC UMC "
#define CPUID_VENDOR_VIA                "CentaurHauls"

#define CPUID_FEATURES0_ECX_FLAG_SSE3    1 << 0
#define CPUID_FEATURES0_ECX_FLAG_PCLMUL  1 << 1
#define CPUID_FEATURES0_ECX_FLAG_DTES64  1 << 2
#define CPUID_FEATURES0_ECX_FLAG_MONITOR 1 << 3
#define CPUID_FEATURES0_ECX_FLAG_DS_CPL  1 << 4
#define CPUID_FEATURES0_ECX_FLAG_VMX     1 << 5
#define CPUID_FEATURES0_ECX_FLAG_SMX     1 << 6
#define CPUID_FEATURES0_ECX_FLAG_EST     1 << 7
#define CPUID_FEATURES0_ECX_FLAG_TM2     1 << 8
#define CPUID_FEATURES0_ECX_FLAG_SSSE3   1 << 9
#define CPUID_FEATURES0_ECX_FLAG_CID     1 << 10
#define CPUID_FEATURES0_ECX_FLAG_FMA     1 << 12
#define CPUID_FEATURES0_ECX_FLAG_CX16    1 << 13
#define CPUID_FEATURES0_ECX_FLAG_ETPRD   1 << 14
#define CPUID_FEATURES0_ECX_FLAG_PDCM    1 << 15
#define CPUID_FEATURES0_ECX_FLAG_DCA     1 << 18
#define CPUID_FEATURES0_ECX_FLAG_SSE4_1  1 << 19
#define CPUID_FEATURES0_ECX_FLAG_SSE4_2  1 << 20
#define CPUID_FEATURES0_ECX_FLAG_x2APIC  1 << 21
#define CPUID_FEATURES0_ECX_FLAG_MOVBE   1 << 22
#define CPUID_FEATURES0_ECX_FLAG_POPCNT  1 << 23
#define CPUID_FEATURES0_ECX_FLAG_XSAVE   1 << 26
#define CPUID_FEATURES0_ECX_FLAG_OSXSAVE 1 << 27
#define CPUID_FEATURES0_ECX_FLAG_AVX     1 << 28

#define CPUID_FEATURES0_EDX_FLAG_FPU     1 << 0
#define CPUID_FEATURES0_EDX_FLAG_VME     1 << 1
#define CPUID_FEATURES0_EDX_FLAG_DE      1 << 2
#define CPUID_FEATURES0_EDX_FLAG_PSE     1 << 3
#define CPUID_FEATURES0_EDX_FLAG_TSC     1 << 4
#define CPUID_FEATURES0_EDX_FLAG_MSR     1 << 5
#define CPUID_FEATURES0_EDX_FLAG_PAE     1 << 6
#define CPUID_FEATURES0_EDX_FLAG_MCE     1 << 7
#define CPUID_FEATURES0_EDX_FLAG_CX8     1 << 8
#define CPUID_FEATURES0_EDX_FLAG_APIC    1 << 9
#define CPUID_FEATURES0_EDX_FLAG_SEP     1 << 11
#define CPUID_FEATURES0_EDX_FLAG_MTRR    1 << 12
#define CPUID_FEATURES0_EDX_FLAG_PGE     1 << 13
#define CPUID_FEATURES0_EDX_FLAG_MCA     1 << 14
#define CPUID_FEATURES0_EDX_FLAG_CMOV    1 << 15
#define CPUID_FEATURES0_EDX_FLAG_PAT     1 << 16
#define CPUID_FEATURES0_EDX_FLAG_PSE36   1 << 17
#define CPUID_FEATURES0_EDX_FLAG_PSN     1 << 18
#define CPUID_FEATURES0_EDX_FLAG_CLF     1 << 19
#define CPUID_FEATURES0_EDX_FLAG_DTES    1 << 21
#define CPUID_FEATURES0_EDX_FLAG_ACPI    1 << 22
#define CPUID_FEATURES0_EDX_FLAG_MMX     1 << 23
#define CPUID_FEATURES0_EDX_FLAG_FXSR    1 << 24
#define CPUID_FEATURES0_EDX_FLAG_SSE     1 << 25
#define CPUID_FEATURES0_EDX_FLAG_SSE2    1 << 26
#define CPUID_FEATURES0_EDX_FLAG_SS      1 << 27
#define CPUID_FEATURES0_EDX_FLAG_HTT     1 << 28
#define CPUID_FEATURES0_EDX_FLAG_TM1     1 << 29
#define CPUID_FEATURES0_EDX_FLAG_IA64    1 << 30
#define CPUID_FEATURES0_EDX_FLAG_PBE     1 << 31

#define CPUID_DRIVER_TYPE 0x9421

struct cpuid_data
{

    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

};

struct cpuid_driver
{

    struct modules_driver base;
    unsigned int (*is_supported)(unsigned int instruction, unsigned int flag);

};

unsigned int cpuid_check_cpuid();
void cpuid_get(unsigned int instruction, struct cpuid_data *data);
void cpuid_get_vendor(char *destination);
void cpuid_get_brand(char *destination);
void cpuid_driver_init(struct cpuid_driver *driver);

#endif

