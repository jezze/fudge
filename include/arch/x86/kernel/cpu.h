#ifndef ARCH_CPU_H
#define ARCH_CPU_H

extern unsigned int cpu_read_cr0();
extern unsigned int cpu_read_cr2();
extern unsigned int cpu_read_cr3();
extern unsigned int cpu_read_cr4();
extern void cpu_write_cr0(unsigned int value);
extern void cpu_write_cr2(unsigned int value);
extern void cpu_write_cr3(unsigned int value);
extern void cpu_write_cr4(unsigned int value);

#endif

