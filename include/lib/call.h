#ifndef LIB_CALL_H
#define LIB_CALL_H

struct file_node;

extern struct file_node *call_open(char *name);
extern void call_close(unsigned int index);
extern int call_map(unsigned int address);
extern int call_halt();
extern int call_reboot();

#endif

