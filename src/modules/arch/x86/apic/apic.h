#define APIC_ICR_NORMAL                 (0 << 8)
#define APIC_ICR_LOW                    (1 << 8)
#define APIC_ICR_SMI                    (2 << 8)
#define APIC_ICR_NMI                    (4 << 8)
#define APIC_ICR_INIT                   (5 << 8)
#define APIC_ICR_SIPI                   (6 << 8)
#define APIC_ICR_PHYSICAL               (0 << 11)
#define APIC_ICR_LOGICAL                (1 << 11)
#define APIC_ICR_STATUS                 (1 << 12)
#define APIC_ICR_ASSERT                 (1 << 14)
#define APIC_ICR_DEASSERT               (2 << 14)
#define APIC_ICR_SINGLE                 (0 << 18)
#define APIC_ICR_SELF                   (1 << 18)
#define APIC_ICR_ALLINCLSELF            (2 << 18)
#define APIC_ICR_ALLEXCLSELF            (3 << 18)

void apic_test(void);
void apic_spurious(void);
void apic_sendint(unsigned int id, unsigned int value);
