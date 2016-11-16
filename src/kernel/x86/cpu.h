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

    unsigned int value;
    void *reference;

};

struct cpu_general
{

    union cpu_register edi;
    union cpu_register esi;
    union cpu_register ebp;
    union cpu_register esp;
    union cpu_register ebx;
    union cpu_register edx;
    union cpu_register ecx;
    union cpu_register eax;

};

struct cpu_interrupt
{

    union cpu_register eip;
    union cpu_register cs;
    union cpu_register eflags;
    union cpu_register esp;
    union cpu_register ss;

};

unsigned int cpu_getcr0(void);
unsigned int cpu_getcr2(void);
unsigned int cpu_getcr3(void);
unsigned int cpu_getcr4(void);
unsigned int cpu_geteflags(void);
void *cpu_getgdt(void);
void *cpu_getidt(void);
void cpu_setcr0(unsigned int value);
void cpu_setcr2(unsigned int value);
void cpu_setcr3(unsigned int value);
void cpu_setcr4(unsigned int value);
void cpu_seteflags(unsigned int value);
void cpu_setgdt(void *pointer, unsigned int code, unsigned int data);
void cpu_setidt(void *pointer);
void cpu_settss(unsigned int value);
void cpu_halt(void);
void cpu_leave(struct cpu_interrupt interrupt);
