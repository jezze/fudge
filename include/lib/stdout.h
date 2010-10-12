#ifndef LIB_STDOUT_H
#define LIB_STDOUT_H

extern int stdout_write_single(char c);
extern int stdout_write(char *s);
extern void stdout_write_num(unsigned int n, unsigned int base);
extern void stdout_write_dec(unsigned int n);
extern void stdout_write_hex(unsigned int n);
extern void stdout_write_bcd(unsigned char n);

#endif

