#ifndef SHELL_H
#define SHELL_H

#define SHELL_BUFFER_SIZE 256

extern char arch_getc();
extern void arch_clear();
extern void arch_putc(char c);
extern void arch_puts(char *s);
extern void arch_puts_dec(unsigned int n);
extern void arch_puts_hex(unsigned int n);

extern void shell_init();

#endif
