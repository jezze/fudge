#ifndef LIB_CALL_H
#define LIB_CALL_H

extern struct file_node *call_open(char *name);
extern unsigned int call_open2(char *name);
extern void call_close(unsigned int index);
extern unsigned int call_read(unsigned int index, void *buffer, unsigned int count);
extern unsigned int call_write(unsigned int index, void *buffer, unsigned int count);
extern int call_map(unsigned int address);
extern int call_halt();
extern int call_reboot();

#endif

