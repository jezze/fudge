#ifndef KERNEL_ASSERT_H
#define KERNEL_ASSERT_H

#define ASSERT(x) ((x) ? (void)0 : kernel_assert(#x, __FILE__, __LINE__))
#define PANIC(x) kernel_panic(x, __FILE__, __LINE__)

extern void arch_disable_interrupts();

extern void kernel_assert(char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);

#endif

