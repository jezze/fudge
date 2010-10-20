#ifndef LIB_FILE_H
#define LIB_FILE_H

struct vfs_node;

extern void file_close(struct vfs_node *node);
extern struct vfs_node *file_find(struct vfs_node *node, char *name);
extern void file_open(struct vfs_node *node);
extern unsigned int file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
extern char file_read_byte(struct vfs_node *node);
extern struct vfs_node *file_walk(struct vfs_node *node, unsigned int index);
extern unsigned int file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer);
extern unsigned int file_write_bcd(struct vfs_node *node, unsigned char num);
extern unsigned int file_write_byte(struct vfs_node *node, char c);
extern unsigned int file_write_dec(struct vfs_node *node, unsigned int num);
extern unsigned int file_write_hex(struct vfs_node *node, unsigned int num);
extern unsigned int file_write_num(struct vfs_node *node, unsigned int num, unsigned int base);
extern unsigned int file_write_string(struct vfs_node *node, char *buffer);

#endif

