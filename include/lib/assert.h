#ifndef LIB_ASSERT_H
#define LIB_ASSERT_H

extern void arch_interrupts_disable();

extern void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);

#endif

