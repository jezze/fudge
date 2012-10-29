#ifndef MODULES_CPUID_H
#define MODULES_CPUID_H

#define CPUID_VENDOR                    0x00000000
#define CPUID_FEATURES0                 0x00000001
#define CPUID_CACHE                     0x00000002
#define CPUID_SERIAL                    0x00000003
#define CPUID_DCACHE                    0x00000004
#define CPUID_MONITOR                   0x00000005
#define CPUID_THERMAL                   0x00000006
#define CPUID_DCA                       0x00000009
#define CPUID_PMON                      0x0000000A
#define CPUID_2XAPIC                    0x0000000B
#define CPUID_XSAVE                     0x0000000D
#define CPUID_LARGEST                   0x80000000
#define CPUID_FEATURES1                 0x80000001
#define CPUID_BRAND0                    0x80000002
#define CPUID_BRAND1                    0x80000003
#define CPUID_BRAND2                    0x80000004
#define CPUID_L2CACHE                   0x80000006
#define CPUID_APM                       0x80000007
#define CPUID_MEMSIZE                   0x80000008

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

#define CPUID_FEATURES0_ECX_SSE3        (1 << 0)
#define CPUID_FEATURES0_ECX_PCLMUL      (1 << 1)
#define CPUID_FEATURES0_ECX_DTES64      (1 << 2)
#define CPUID_FEATURES0_ECX_MONITOR     (1 << 3)
#define CPUID_FEATURES0_ECX_DS_CPL      (1 << 4)
#define CPUID_FEATURES0_ECX_VMX         (1 << 5)
#define CPUID_FEATURES0_ECX_SMX         (1 << 6)
#define CPUID_FEATURES0_ECX_EST         (1 << 7)
#define CPUID_FEATURES0_ECX_TM2         (1 << 8)
#define CPUID_FEATURES0_ECX_SSSE3       (1 << 9)
#define CPUID_FEATURES0_ECX_CID         (1 << 10)
#define CPUID_FEATURES0_ECX_FMA         (1 << 12)
#define CPUID_FEATURES0_ECX_CX16        (1 << 13)
#define CPUID_FEATURES0_ECX_ETPRD       (1 << 14)
#define CPUID_FEATURES0_ECX_PDCM        (1 << 15)
#define CPUID_FEATURES0_ECX_DCA         (1 << 18)
#define CPUID_FEATURES0_ECX_SSE4_1      (1 << 19)
#define CPUID_FEATURES0_ECX_SSE4_2      (1 << 20)
#define CPUID_FEATURES0_ECX_x2APIC      (1 << 21)
#define CPUID_FEATURES0_ECX_MOVBE       (1 << 22)
#define CPUID_FEATURES0_ECX_POPCNT      (1 << 23)
#define CPUID_FEATURES0_ECX_XSAVE       (1 << 26)
#define CPUID_FEATURES0_ECX_OSXSAVE     (1 << 27)
#define CPUID_FEATURES0_ECX_AVX         (1 << 28)

#define CPUID_FEATURES0_EDX_FPU         (1 << 0)
#define CPUID_FEATURES0_EDX_VME         (1 << 1)
#define CPUID_FEATURES0_EDX_DE          (1 << 2)
#define CPUID_FEATURES0_EDX_PSE         (1 << 3)
#define CPUID_FEATURES0_EDX_TSC         (1 << 4)
#define CPUID_FEATURES0_EDX_MSR         (1 << 5)
#define CPUID_FEATURES0_EDX_PAE         (1 << 6)
#define CPUID_FEATURES0_EDX_MCE         (1 << 7)
#define CPUID_FEATURES0_EDX_CX8         (1 << 8)
#define CPUID_FEATURES0_EDX_APIC        (1 << 9)
#define CPUID_FEATURES0_EDX_SEP         (1 << 11)
#define CPUID_FEATURES0_EDX_MTRR        (1 << 12)
#define CPUID_FEATURES0_EDX_PGE         (1 << 13)
#define CPUID_FEATURES0_EDX_MCA         (1 << 14)
#define CPUID_FEATURES0_EDX_CMOV        (1 << 15)
#define CPUID_FEATURES0_EDX_PAT         (1 << 16)
#define CPUID_FEATURES0_EDX_PSE36       (1 << 17)
#define CPUID_FEATURES0_EDX_PSN         (1 << 18)
#define CPUID_FEATURES0_EDX_CLF         (1 << 19)
#define CPUID_FEATURES0_EDX_DTES        (1 << 21)
#define CPUID_FEATURES0_EDX_ACPI        (1 << 22)
#define CPUID_FEATURES0_EDX_MMX         (1 << 23)
#define CPUID_FEATURES0_EDX_FXSR        (1 << 24)
#define CPUID_FEATURES0_EDX_SSE         (1 << 25)
#define CPUID_FEATURES0_EDX_SSE2        (1 << 26)
#define CPUID_FEATURES0_EDX_SS          (1 << 27)
#define CPUID_FEATURES0_EDX_HTT         (1 << 28)
#define CPUID_FEATURES0_EDX_TM1         (1 << 29)
#define CPUID_FEATURES0_EDX_IA64        (1 << 30)
#define CPUID_FEATURES0_EDX_PBE         (1 << 31)

struct cpuid_data
{

    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

};

unsigned int cpuid_check_cpuid();
void cpuid_get(unsigned int instruction, struct cpuid_data *data);
void cpuid_get_vendor(char *destination);
void cpuid_get_brand(char *destination);

#endif

