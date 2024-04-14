union cpu_register
{

    unsigned int value;
    void *reference;

};

struct cpu_registers
{

    union cpu_register pc;
    union cpu_register sp;

};

unsigned int cpu_get_cpsr(void);
void cpu_set_cpsr(unsigned int value);
void cpu_disable_interrupts(void);
void cpu_enable_interrupts(void);
void cpu_halt(void);
void ivt_enable(void);
