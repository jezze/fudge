#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

#define INITRD_HEADER_SIZE 64

struct vfs_filesystem;

struct initrd_filesystem
{

    struct vfs_filesystem base;
    struct tar_header *headers[INITRD_HEADER_SIZE];
    struct vfs_node nodes[INITRD_HEADER_SIZE];

};

extern void initrd_init(unsigned int *address);

#endif

