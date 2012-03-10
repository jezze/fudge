#ifndef LIB_FILE_H
#define LIB_FILE_H

#define FILE_STDIN  1
#define FILE_STDOUT 2
#define FILE_STDERR 3

void file_close(unsigned int fd);
unsigned int file_open(char *path);
unsigned int file_read(unsigned int fd, unsigned int count, void *buffer);
unsigned int file_read_byte(unsigned int fd, char c);
unsigned int file_write(unsigned int fd, unsigned int count, void *buffer);
unsigned int file_write_format(unsigned int fd, char *buffer, ...);

#endif

