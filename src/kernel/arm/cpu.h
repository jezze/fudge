unsigned int cpu_get_cpsr(void);
void cpu_set_cpsr(unsigned int value);
void cpu_disable_interrupts(void);
void cpu_enable_interrupts(void);
void ivt_enable(void);
void swi_test(void);
