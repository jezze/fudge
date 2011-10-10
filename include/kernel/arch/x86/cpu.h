#ifndef ARCH_CPU_H
#define ARCH_CPU_H

extern unsigned int cpu_get_cr0();
extern unsigned int cpu_get_cr2();
extern unsigned int cpu_get_cr3();
extern unsigned int cpu_get_cr4();
extern unsigned int cpu_get_eflags();
extern unsigned int cpu_get_stack();
extern void cpu_halt();
extern void cpu_idle();
extern void cpu_interrupts_off();
extern void cpu_interrupts_on();
extern void cpu_set_cr0(unsigned int value);
extern void cpu_set_cr2(unsigned int value);
extern void cpu_set_cr3(unsigned int value);
extern void cpu_set_cr4(unsigned int value);
extern void cpu_set_eflags(unsigned int value);
extern void cpu_set_stack(unsigned int address);
extern void cpu_usermode(void *callback);

#endif

