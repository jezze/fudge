#ifndef LIB_FILE_H
#define LIB_FILE_H

#define FILE_STDIN  0
#define FILE_STDOUT 1
#define FILE_STDERR 2

struct file_node
{

    char name[128];
    unsigned int inode;
    unsigned int cursor;
    unsigned int length;
    void (*open)(struct file_node *node);
    void (*close)(struct file_node *node);
    unsigned int (*read)(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
    struct file_node *(*walk)(struct file_node *node, unsigned int index);

};

struct file_descriptor
{

    struct file_node *node;
    unsigned int permissions;

};

extern void file_close(unsigned int fd);
extern int file_open(char *name);
extern unsigned int file_read(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_read_byte(unsigned int fd, char c);
extern unsigned int file_write(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_write_bcd(unsigned int fd, unsigned char num);
extern unsigned int file_write_byte(unsigned int fd, char c);
extern unsigned int file_write_dec(unsigned int fd, unsigned int num);
extern unsigned int file_write_hex(unsigned int fd, unsigned int num);
extern unsigned int file_write_num(unsigned int fd, unsigned int num, unsigned int base);
extern unsigned int file_write_string(unsigned int fd, char *buffer);
extern unsigned int file_write_string_format(unsigned int fd, char *buffer, void **args);

#endif

