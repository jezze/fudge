#ifndef ARCH_CPU_H
#define ARCH_CPU_H

void cpu_disable_interrupts();
void cpu_enable_fpu();
void cpu_enable_interrupts();
void cpu_enable_tss();
void cpu_enter_usermode(unsigned int ip, unsigned int sp);
unsigned int cpu_get_cr0();
unsigned int cpu_get_cr2();
unsigned int cpu_get_cr3();
unsigned int cpu_get_cr4();
unsigned int cpu_get_eflags();
unsigned int cpu_get_stack();
void cpu_halt();
void cpu_idle();
void cpu_set_cr0(unsigned int value);
void cpu_set_cr2(unsigned int value);
void cpu_set_cr3(unsigned int value);
void cpu_set_cr4(unsigned int value);
void cpu_set_gdt(void *pointer);
void cpu_set_idt(void *pointer);
void cpu_set_eflags(unsigned int value);
void cpu_set_stack(void *address);

#endif

