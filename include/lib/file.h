#ifndef LIB_FILE_H
#define LIB_FILE_H

struct vfs_node;

extern char file_read_byte(struct vfs_node *node);
extern unsigned int file_write_bcd(struct vfs_node *node, unsigned char num);
extern unsigned int file_write_byte(struct vfs_node *node, char c);
extern unsigned int file_write_dec(struct vfs_node *node, unsigned int num);
extern unsigned int file_write_hex(struct vfs_node *node, unsigned int num);
extern unsigned int file_write_num(struct vfs_node *node, unsigned int num, unsigned int base);
extern unsigned int file_write_string(struct vfs_node *node, char *buffer);

#endif

