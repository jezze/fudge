#ifndef CALL_H
#define CALL_H

#define CALL_VFS_READ  0x10
#define CALL_VFS_WRITE 0x11
#define CALL_VFS_WALK  0x12
#define CALL_VFS_FIND  0x13

#define CALL_HALT    0x50
#define CALL_REBOOT 0x51

extern int call_putc(char c);
extern int call_puts(char *s);
extern char call_getc();

extern vfs_node_t *call_vfs_walk(unsigned int index);
extern vfs_node_t *call_vfs_find(char *name);
extern int call_halt();
extern int call_reboot();

#endif

