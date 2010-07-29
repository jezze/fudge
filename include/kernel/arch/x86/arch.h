#ifndef ARCH_H
#define ARCH_H

struct vfs_node;

extern char arch_getc();
extern void arch_clear();
extern void arch_putc(char c);
extern void arch_puts(char *s);
extern void arch_puts_num(unsigned int n, unsigned int base);
extern void arch_puts_dec(unsigned int n);
extern void arch_puts_hex(unsigned int n);
extern void arch_puts_bcd(unsigned char n);

extern struct vfs_node *arch_get_stdin();
extern struct vfs_node *arch_get_stdout();
extern void arch_set_stdin(struct vfs_node *node);
extern void arch_set_stdout(struct vfs_node *node);
extern void arch_disable_interrupts();
extern void arch_enable_interrupts();
extern void arch_reboot();
extern void arch_init();
extern void arch_init_devices();
extern void arch_init_interrupts();
extern void arch_init_syscalls();

#endif

