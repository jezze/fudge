#define APIC_ICR_TYPE_NORMAL            (0 << 8)
#define APIC_ICR_TYPE_LOW               (1 << 8)
#define APIC_ICR_TYPE_SMI               (2 << 8)
#define APIC_ICR_TYPE_NMI               (4 << 8)
#define APIC_ICR_TYPE_INIT              (5 << 8)
#define APIC_ICR_TYPE_SIPI              (6 << 8)
#define APIC_ICR_MODE_PHYSICAL          (0 << 11)
#define APIC_ICR_MODE_LOGICAL           (1 << 11)
#define APIC_ICR_STATUS_IDLE            (0 << 12)
#define APIC_ICR_STATUS_PENDING         (1 << 12)
#define APIC_ICR_LEVEL_DEASSERT         (0 << 14)
#define APIC_ICR_LEVEL_ASSERT           (1 << 14)
#define APIC_ICR_TRIGGER_EDGE           (0 << 15)
#define APIC_ICR_TRIGGER_LEVEL          (1 << 15)
#define APIC_ICR_TARGET_NORMAL          (0 << 18)
#define APIC_ICR_TARGET_SELF            (1 << 18)
#define APIC_ICR_TARGET_ALLINCLSELF     (2 << 18)
#define APIC_ICR_TARGET_ALLEXCLSELF     (3 << 18)

unsigned int apic_getid(void);
void apic_test(void);
void apic_spurious(void);
void apic_setup();
void apic_sendint(unsigned int id, unsigned int value);
