#ifndef LIB_WRITE_H
#define LIB_WRITE_H

extern int write_char(char c);
extern int write_string(char *s);
extern void write_string_num(unsigned int n, unsigned int base);
extern void write_string_dec(unsigned int n);
extern void write_string_hex(unsigned int n);
extern void write_string_bcd(unsigned char n);

#endif

