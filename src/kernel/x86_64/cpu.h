#define CPU_CR0_PE                      (1 << 0)
#define CPU_CR0_MP                      (1 << 1)
#define CPU_CR0_EM                      (1 << 2)
#define CPU_CR0_TS                      (1 << 3)
#define CPU_CR0_ET                      (1 << 4)
#define CPU_CR0_NE                      (1 << 5)
#define CPU_CR0_WP                      (1 << 16)
#define CPU_CR0_AM                      (1 << 18)
#define CPU_CR0_NW                      (1 << 29)
#define CPU_CR0_CD                      (1 << 30)
#define CPU_CR0_PG                      (1UL << 31)
#define CPU_CR4_VME                     (1 << 0)
#define CPU_CR4_PVI                     (1 << 1)
#define CPU_CR4_TSD                     (1 << 2)
#define CPU_CR4_DE                      (1 << 3)
#define CPU_CR4_PSE                     (1 << 4)
#define CPU_CR4_PAE                     (1 << 5)
#define CPU_CR4_MCE                     (1 << 6)
#define CPU_CR4_PGE                     (1 << 7)
#define CPU_CR4_PCE                     (1 << 8)
#define CPU_CR4_OSFXSR                  (1 << 9)
#define CPU_CR4_OSMMEXCPT               (1 << 10)
#define CPU_CR4_UMIP                    (1 << 11)
#define CPU_CR4_LA57                    (1 << 12)
#define CPU_CR4_VMXE                    (1 << 13)
#define CPU_CR4_SMXE                    (1 << 14)
#define CPU_CR4_FSGSBASE                (1 << 16)
#define CPU_CR4_PCIDE                   (1 << 17)
#define CPU_CR4_OSXSAVE                 (1 << 18)
#define CPU_CR4_KL                      (1 << 19)
#define CPU_CR4_SMEP                    (1 << 20)
#define CPU_CR4_SMAP                    (1 << 21)
#define CPU_CR4_PKE                     (1 << 22)
#define CPU_CR4_CET                     (1 << 23)
#define CPU_CR4_PKS                     (1 << 24)
#define CPU_CR4_UINTR                   (1 << 25)
#define CPU_CR4_LASS                    (1 << 27)
#define CPU_CR4_LAMSUP                  (1 << 28)
#define CPU_CR4_FRED                    (1UL << 32)
#define CPU_FLAGS_CF                    (1 << 0)
#define CPU_FLAGS_PF                    (1 << 2)
#define CPU_FLAGS_AF                    (1 << 4)
#define CPU_FLAGS_ZF                    (1 << 6)
#define CPU_FLAGS_SF                    (1 << 7)
#define CPU_FLAGS_TF                    (1 << 8)
#define CPU_FLAGS_IF                    (1 << 9)
#define CPU_FLAGS_DF                    (1 << 10)
#define CPU_FLAGS_OF                    (1 << 11)
#define CPU_FLAGS_NT                    (1 << 14)
#define CPU_FLAGS_RF                    (1 << 16)
#define CPU_FLAGS_VM                    (1 << 17)
#define CPU_FLAGS_AC                    (1 << 18)
#define CPU_FLAGS_VIF                   (1 << 19)
#define CPU_FLAGS_VIP                   (1 << 20)
#define CPU_FLAGS_ID                    (1 << 21)

union cpu_register
{

    unsigned long value;
    void *reference;

};

struct cpu_general
{

    union cpu_register r15;
    union cpu_register r14;
    union cpu_register r13;
    union cpu_register r12;
    union cpu_register r11;
    union cpu_register r10;
    union cpu_register r9;
    union cpu_register r8;
    union cpu_register rdi;
    union cpu_register rsi;
    union cpu_register rbp;
    union cpu_register rbx;
    union cpu_register rdx;
    union cpu_register rcx;
    union cpu_register rax;

};

struct cpu_interrupt
{

    union cpu_register rip;
    union cpu_register cs;
    union cpu_register rflags;
    union cpu_register rsp;
    union cpu_register ss;

};

unsigned long cpu_getcr0(void);
unsigned long cpu_getcr2(void);
unsigned long cpu_getcr3(void);
unsigned long cpu_getcr4(void);
unsigned long cpu_getrflags(void);
void cpu_setcr0(unsigned long value);
void cpu_setcr3(unsigned long value);
void cpu_setcr4(unsigned long value);
void cpu_setgdt(void *pointer, unsigned int code, unsigned int data);
void cpu_setidt(void *pointer);
void cpu_settss(unsigned int value);
void cpu_halt(void);
void cpu_leave(struct cpu_general *general, struct cpu_interrupt *interrupt);
