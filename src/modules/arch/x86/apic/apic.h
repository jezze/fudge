#define APIC_REG_ID                     0x0020
#define APIC_REG_VERSION                0x0030
#define APIC_REG_TPR                    0x0080
#define APIC_REG_APR                    0x0090
#define APIC_REG_CPR                    0x00A0
#define APIC_REG_EOI                    0x00B0
#define APIC_REG_REMOTEREAD             0x00C0
#define APIC_REG_LDR                    0x00D0
#define APIC_REG_DFR                    0x00E0
#define APIC_REG_SV                     0x00F0
#define APIC_REG_ISR0                   0x0100
#define APIC_REG_ISR1                   0x0110
#define APIC_REG_ISR2                   0x0120
#define APIC_REG_ISR3                   0x0130
#define APIC_REG_ISR4                   0x0140
#define APIC_REG_ISR5                   0x0150
#define APIC_REG_ISR6                   0x0160
#define APIC_REG_ISR7                   0x0170
#define APIC_REG_TMR0                   0x0180
#define APIC_REG_TMR1                   0x0190
#define APIC_REG_TMR2                   0x01A0
#define APIC_REG_TMR3                   0x01B0
#define APIC_REG_TMR4                   0x01C0
#define APIC_REG_TMR5                   0x01D0
#define APIC_REG_TMR6                   0x01E0
#define APIC_REG_TMR7                   0x01F0
#define APIC_REG_IRR0                   0x0200
#define APIC_REG_IRR1                   0x0210
#define APIC_REG_IRR2                   0x0220
#define APIC_REG_IRR3                   0x0230
#define APIC_REG_IRR4                   0x0240
#define APIC_REG_IRR5                   0x0250
#define APIC_REG_IRR6                   0x0260
#define APIC_REG_IRR7                   0x0270
#define APIC_REG_ERRORSTATUS            0x0280
#define APIC_REG_LVTCMCI                0x02F0
#define APIC_REG_ICR0                   0x0300
#define APIC_REG_ICR1                   0x0310
#define APIC_REG_ICR_TYPE_NORMAL        (0 << 8)
#define APIC_REG_ICR_TYPE_LOW           (1 << 8)
#define APIC_REG_ICR_TYPE_SMI           (2 << 8)
#define APIC_REG_ICR_TYPE_NMI           (4 << 8)
#define APIC_REG_ICR_TYPE_INIT          (5 << 8)
#define APIC_REG_ICR_TYPE_SIPI          (6 << 8)
#define APIC_REG_ICR_MODE_PHYSICAL      (0 << 11)
#define APIC_REG_ICR_MODE_LOGICAL       (1 << 11)
#define APIC_REG_ICR_STATUS_IDLE        (0 << 12)
#define APIC_REG_ICR_STATUS_PENDING     (1 << 12)
#define APIC_REG_ICR_LEVEL_DEASSERT     (0 << 14)
#define APIC_REG_ICR_LEVEL_ASSERT       (1 << 14)
#define APIC_REG_ICR_TRIGGER_EDGE       (0 << 15)
#define APIC_REG_ICR_TRIGGER_LEVEL      (1 << 15)
#define APIC_REG_ICR_TARGET_NORMAL      (0 << 18)
#define APIC_REG_ICR_TARGET_SELF        (1 << 18)
#define APIC_REG_ICR_TARGET_ALLINCLSELF (2 << 18)
#define APIC_REG_ICR_TARGET_ALLEXCLSELF (3 << 18)
#define APIC_REG_LVTTIMER               0x0320
#define APIC_REG_LVTTHERMAL             0x0330
#define APIC_REG_LVTPERF                0x0340
#define APIC_REG_LVTLINT0               0x0350
#define APIC_REG_LVTLINT1               0x0360
#define APIC_REG_LVTERROR               0x0370
#define APIC_REG_INITIALCOUNT           0x0380
#define APIC_REG_CURRENTCOUNT           0x0390
#define APIC_REG_DIVIDECONFIG           0x03E0

void apic_debug_ioapic(void);
void apic_debug(void);
void apic_setupisrs(void);
unsigned int apic_checklapic(unsigned int id);
unsigned int apic_checkioapic(unsigned int id);
unsigned int apic_getid(void);
void apic_test(void);
void apic_isr(void);
void apic_spurious(void);
unsigned int apic_setroutine(unsigned int irq, void (*routine)(unsigned int irq));
unsigned int apic_unsetroutine(unsigned int irq);
void apic_setup_ap(void);
void apic_setup_bp(void);
void apic_sendint(unsigned int id, unsigned int value);
