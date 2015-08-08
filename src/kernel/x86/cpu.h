struct cpu_general
{

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

};

struct cpu_interrupt
{

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;

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
