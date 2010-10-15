#ifndef LIB_FILE_H
#define LIB_FILE_H

extern char file_read_single();
extern int file_write_single(char c);
extern int file_write(char *s);
extern void file_write_num(unsigned int n, unsigned int base);
extern void file_write_dec(unsigned int n);
extern void file_write_hex(unsigned int n);
extern void file_write_bcd(unsigned char n);

#endif

