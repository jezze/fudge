#ifndef SHELL_H
#define SHELL_H

#define SHELL_BUFFER_SIZE 256

extern void arch_clear();
extern void arch_putc(char c);
extern void arch_puts(char *s);

extern void shell_init();

#endif
