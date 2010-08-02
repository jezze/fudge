#ifndef ARCH_H
#define ARCH_H

struct vfs_node;

extern void arch_clear();
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

