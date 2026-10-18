#define CPU_MODE_USR                    0x10
#define CPU_MODE_FIQ                    0x11
#define CPU_MODE_IRQ                    0x12
#define CPU_MODE_SVC                    0x13
#define CPU_MODE_ABT                    0x17
#define CPU_MODE_UND                    0x1B
#define CPU_MODE_SYS                    0x1F
#define CPU_MODE_MASK                   0x1F
#define CPU_FLAGS_F                     (1 << 6)
#define CPU_FLAGS_I                     (1 << 7)
#define CPU_FLAGS_NZCVQ                 0xF8000000
#define CPU_CONTROL_MMU                 (1 << 0)
#define CPU_DOMAIN0_CLIENT              0x00000001

union cpu_register
{

    unsigned int value;
    void *reference;

};

struct cpu_general
{

    union cpu_register r0;
    union cpu_register r1;
    union cpu_register r2;
    union cpu_register r3;
    union cpu_register r4;
    union cpu_register r5;
    union cpu_register r6;
    union cpu_register r7;
    union cpu_register r8;
    union cpu_register r9;
    union cpu_register r10;
    union cpu_register r11;
    union cpu_register r12;

};

/* sp and lr are the interrupted mode's banked registers */
struct cpu_interrupt
{

    union cpu_register cpsr;
    union cpu_register sp;
    union cpu_register lr;
    union cpu_register pc;

};

/* the layout isr.s saves on the kernel stack */
struct cpu_frame
{

    struct cpu_interrupt interrupt;
    struct cpu_general general;
    unsigned int pad;

};

unsigned int cpu_getcontrol(void);
void cpu_setcontrol(unsigned int value);
void cpu_setdomains(unsigned int value);
unsigned int cpu_getttbr(void);
void cpu_setttbr(unsigned int value);
void cpu_flushtlb(void);
unsigned int cpu_getfsr(void);
unsigned int cpu_getfar(void);
void cpu_setmodestack(unsigned int mode, void *stack);
void cpu_halt(void);
void cpu_leave(struct cpu_frame *frame);
