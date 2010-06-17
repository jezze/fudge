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

extern uint32_t cpu_check_cpuid();

extern void cpu_get_vendor(char *destination);
extern void cpu_init();

#endif

