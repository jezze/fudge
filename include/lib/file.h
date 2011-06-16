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

extern struct file_node *file_find(struct file_node *node, char *name);
extern unsigned int file_read(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_read_byte(struct file_node *node, char c);
extern struct file_node *file_walk(struct file_node *node, unsigned int index);
extern unsigned int file_write(struct file_node *node, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_write_bcd(struct file_node *node, unsigned char num);
extern unsigned int file_write_byte(struct file_node *node, char c);
extern unsigned int file_write_dec(struct file_node *node, unsigned int num);
extern unsigned int file_write_hex(struct file_node *node, unsigned int num);
extern unsigned int file_write_num(struct file_node *node, unsigned int num, unsigned int base);
extern unsigned int file_write_string(struct file_node *node, char *buffer);
extern unsigned int file_write_string_format(struct file_node *node, char *buffer, void **args);

extern unsigned int file_read2(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_read_byte2(unsigned int fd, char c);
extern unsigned int file_write2(unsigned int fd, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_write_bcd2(unsigned int fd, unsigned char num);
extern unsigned int file_write_byte2(unsigned int fd, char c);
extern unsigned int file_write_dec2(unsigned int fd, unsigned int num);
extern unsigned int file_write_hex2(unsigned int fd, unsigned int num);
extern unsigned int file_write_num2(unsigned int fd, unsigned int num, unsigned int base);
extern unsigned int file_write_string2(unsigned int fd, char *buffer);
extern unsigned int file_write_string_format2(unsigned int fd, char *buffer, void **args);

#endif

