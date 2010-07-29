#ifndef INITRD_H
#define INITRD_H

struct vfs_node;

extern void arch_set_stdin(struct vfs_node *node);
extern void arch_set_stdout(struct vfs_node *node);

struct initrd_header
{

    unsigned int nfiles;

};

struct initrd_file_header
{

    unsigned char magic;
    char name[64];
    unsigned int offset;
    unsigned int length;

};

extern struct vfs_node *initrd_init(unsigned int location);

#endif

