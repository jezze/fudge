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
#define CPUID_VENDOR_AMD2               "AMDisbetter!"
#define CPUID_VENDOR_CENTAUR            "CentaurHauls"
#define CPUID_VENDOR_CYRIX              "CyrixInstead"
#define CPUID_VENDOR_INTEL              "GenuineIntel"
#define CPUID_VENDOR_NEXGEN             "NexGenDriven"
#define CPUID_VENDOR_NSC                "Geode by NSC"
#define CPUID_VENDOR_RISE               "RiseRiseRise"
#define CPUID_VENDOR_SIS                "SiS SiS SiS "
#define CPUID_VENDOR_TRANSMETA          "GeniuneTMx86"
#define CPUID_VENDOR_TRANSMETA2         "TransmetaCPU"
#define CPUID_VENDOR_UMC                "UMC UMC UMC "
#define CPUID_VENDOR_VIA                "CentaurHauls"

#define CPUID_FEATURES00_SSE3           (1 << 0)
#define CPUID_FEATURES00_PCLMUL         (1 << 1)
#define CPUID_FEATURES00_DTES64         (1 << 2)
#define CPUID_FEATURES00_MONITOR        (1 << 3)
#define CPUID_FEATURES00_DSCPL          (1 << 4)
#define CPUID_FEATURES00_VMX            (1 << 5)
#define CPUID_FEATURES00_SMX            (1 << 6)
#define CPUID_FEATURES00_EST            (1 << 7)
#define CPUID_FEATURES00_TM2            (1 << 8)
#define CPUID_FEATURES00_SSSE3          (1 << 9)
#define CPUID_FEATURES00_CID            (1 << 10)
#define CPUID_FEATURES00_FMA            (1 << 12)
#define CPUID_FEATURES00_CX16           (1 << 13)
#define CPUID_FEATURES00_ETPRD          (1 << 14)
#define CPUID_FEATURES00_PDCM           (1 << 15)
#define CPUID_FEATURES00_DCA            (1 << 18)
#define CPUID_FEATURES00_SSE41          (1 << 19)
#define CPUID_FEATURES00_SSE42          (1 << 20)
#define CPUID_FEATURES00_2XAPIC         (1 << 21)
#define CPUID_FEATURES00_MOVBE          (1 << 22)
#define CPUID_FEATURES00_POPCNT         (1 << 23)
#define CPUID_FEATURES00_XSAVE          (1 << 26)
#define CPUID_FEATURES00_OSXSAVE        (1 << 27)
#define CPUID_FEATURES00_AVX            (1 << 28)

#define CPUID_FEATURES01_FPU            (1 << 0)
#define CPUID_FEATURES01_VME            (1 << 1)
#define CPUID_FEATURES01_DE             (1 << 2)
#define CPUID_FEATURES01_PSE            (1 << 3)
#define CPUID_FEATURES01_TSC            (1 << 4)
#define CPUID_FEATURES01_MSR            (1 << 5)
#define CPUID_FEATURES01_PAE            (1 << 6)
#define CPUID_FEATURES01_MCE            (1 << 7)
#define CPUID_FEATURES01_CX8            (1 << 8)
#define CPUID_FEATURES01_APIC           (1 << 9)
#define CPUID_FEATURES01_SEP            (1 << 11)
#define CPUID_FEATURES01_MTRR           (1 << 12)
#define CPUID_FEATURES01_PGE            (1 << 13)
#define CPUID_FEATURES01_MCA            (1 << 14)
#define CPUID_FEATURES01_CMOV           (1 << 15)
#define CPUID_FEATURES01_PAT            (1 << 16)
#define CPUID_FEATURES01_PSE36          (1 << 17)
#define CPUID_FEATURES01_PSN            (1 << 18)
#define CPUID_FEATURES01_CLF            (1 << 19)
#define CPUID_FEATURES01_DTES           (1 << 21)
#define CPUID_FEATURES01_ACPI           (1 << 22)
#define CPUID_FEATURES01_MMX            (1 << 23)
#define CPUID_FEATURES01_FXSR           (1 << 24)
#define CPUID_FEATURES01_SSE            (1 << 25)
#define CPUID_FEATURES01_SSE2           (1 << 26)
#define CPUID_FEATURES01_SS             (1 << 27)
#define CPUID_FEATURES01_HTT            (1 << 28)
#define CPUID_FEATURES01_TM1            (1 << 29)
#define CPUID_FEATURES01_IA64           (1 << 30)
#define CPUID_FEATURES01_PBE            (1 << 31)

struct cpuid_data
{

    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

};

unsigned int cpuid_exist(void);
void cpuid_getdata(unsigned int instruction, struct cpuid_data *data);
void cpuid_getvendor(char *destination);
void cpuid_getbrand(char *destination);
