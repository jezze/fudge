#ifndef LIB_CALL_H
#define LIB_CALL_H

struct vfs_node;

extern char call_getc();
extern int call_putc(char c);
extern int call_puts(char *s);
extern void call_puts_num(unsigned int n, unsigned int base);
extern void call_puts_dec(unsigned int n);
extern void call_puts_hex(unsigned int n);
extern void call_puts_bcd(unsigned char n);

extern struct vfs_node *call_open(char *name);
extern int call_halt();
extern int call_reboot();

#endif

