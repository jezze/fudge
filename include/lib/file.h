#ifndef LIB_FILE_H
#define LIB_FILE_H

#define FILE_STDIN  0
#define FILE_STDOUT 1
#define FILE_STDERR 2

struct file_info
{

    unsigned int id;
    unsigned int length;

};

extern void file_close(unsigned int fd);
extern unsigned int file_info(unsigned int fd, struct file_info *info);
extern int file_open(char *name);
extern unsigned int file_read(unsigned int fd, unsigned int count, void *buffer);
extern unsigned int file_read_byte(unsigned int fd, char c);
extern unsigned int file_write(unsigned int fd, unsigned int count, void *buffer);
extern unsigned int file_write_bcd(unsigned int fd, unsigned char num);
extern unsigned int file_write_byte(unsigned int fd, char c);
extern unsigned int file_write_format(unsigned int fd, char *buffer, ...);

#endif

