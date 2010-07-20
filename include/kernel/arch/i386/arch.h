#ifndef ARCH_H
#define ARCH_H

struct vfs_node;

extern struct vfs_node *arch_get_stdin();
extern struct vfs_node *arch_get_stdout();
extern void arch_disable_interrupts();
extern void arch_enable_interrupts();
extern void arch_init();
extern void arch_init_devices();
extern void arch_init_interrupts();
extern void arch_init_syscalls();

#endif

