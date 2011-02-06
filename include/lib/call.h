#ifndef LIB_CALL_H
#define LIB_CALL_H

struct vfs_node;

extern struct vfs_node *call_open(char *name);
extern int call_map(unsigned int address);
extern int call_halt();
extern int call_reboot();

#endif

