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
    unsigned int code;
    unsigned int eflags;
    unsigned int esp;
    unsigned int data;

};

unsigned int cpu_get_cr0();
unsigned int cpu_get_cr2();
unsigned int cpu_get_cr3();
unsigned int cpu_get_cr4();
unsigned int cpu_get_eflags();
void *cpu_get_gdt();
void *cpu_get_idt();
void cpu_set_cr0(unsigned int value);
void cpu_set_cr2(unsigned int value);
void cpu_set_cr3(unsigned int value);
void cpu_set_cr4(unsigned int value);
void cpu_set_eflags(unsigned int value);
void cpu_set_gdt(void *pointer, unsigned int code, unsigned int data);
void cpu_set_idt(void *pointer);
void cpu_set_tss(unsigned int value);
