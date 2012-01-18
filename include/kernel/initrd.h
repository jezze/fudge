#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

#define INITRD_HEADER_SIZE 64

struct vfs_node;

struct initrd_node
{

    struct vfs_node base;
    unsigned int size;
    struct tar_header *header;
    void *data;

};

extern void initrd_init(unsigned int initrdc, void **initrdv);

#endif

