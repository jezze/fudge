#ifndef LIB_FILE_H
#define LIB_FILE_H

struct vfs_node;

extern char file_read_single(struct vfs_node *node);
extern int file_write_single(struct vfs_node *node, char c);
extern int file_write(struct vfs_node *node, char *buffer);
extern void file_write_num(struct vfs_node *node, unsigned int num, unsigned int base);
extern void file_write_dec(struct vfs_node *node, unsigned int num);
extern void file_write_hex(struct vfs_node *node, unsigned int num);
extern void file_write_bcd(struct vfs_node *node, unsigned char num);

#endif

