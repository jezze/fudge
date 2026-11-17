#define CPU_STATUS_EL0                  0x00
#define CPU_STATUS_EL1                  0x05
#define CPU_STATUS_MODEMASK             0x0F
#define CPU_EXCEPTION_SYNC              0
#define CPU_EXCEPTION_IRQ               1
#define CPU_EXCEPTION_FIQ               2
#define CPU_EXCEPTION_SERROR            3
#define CPU_INTERRUPT_IRQ               0
#define CPU_INTERRUPT_FIQ               1
#define CPU_CLASS_FPU                   0x07
#define CPU_CLASS_SVC                   0x15
#define CPU_CLASS_FETCHLOWER            0x20
#define CPU_CLASS_FETCH                 0x21
#define CPU_CLASS_DATALOWER             0x24
#define CPU_CLASS_DATA                  0x25
#define CPU_FAULT_TRANSLATIONMASK       0x3C
#define CPU_FAULT_TRANSLATION           0x04

union cpu_register
{

    unsigned long value;
    void *reference;

};

/* x0-x30 */
struct cpu_general
{

    union cpu_register x0;
    union cpu_register x1;
    union cpu_register x2;
    union cpu_register x3;
    union cpu_register x4;
    union cpu_register x5;
    union cpu_register x6;
    union cpu_register x7;
    union cpu_register x8;
    union cpu_register x9;
    union cpu_register x10;
    union cpu_register x11;
    union cpu_register x12;
    union cpu_register x13;
    union cpu_register x14;
    union cpu_register x15;
    union cpu_register x16;
    union cpu_register x17;
    union cpu_register x18;
    union cpu_register x19;
    union cpu_register x20;
    union cpu_register x21;
    union cpu_register x22;
    union cpu_register x23;
    union cpu_register x24;
    union cpu_register x25;
    union cpu_register x26;
    union cpu_register x27;
    union cpu_register x28;
    union cpu_register x29;
    union cpu_register x30;

};

struct cpu_interrupt
{

    union cpu_register sp;
    union cpu_register pc;
    union cpu_register status;

};

/* the layout trap.s saves on the kernel stack */
struct cpu_frame
{

    struct cpu_general general;
    struct cpu_interrupt interrupt;

};

unsigned long cpu_getttbr(void);
void cpu_setttbr(unsigned long value);
void cpu_enablemmu(unsigned long directory);
void cpu_flushtlb(void);
void cpu_setvector(void (*vector)(void));
void cpu_enablefpu(void);
void cpu_disablefpu(void);
void cpu_halt(void);
void cpu_savefpu(void *state);
void cpu_loadfpu(void *state);
void cpu_leave(struct cpu_frame *frame);
