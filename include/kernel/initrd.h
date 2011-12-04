#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

#define INITRD_HEADER_SIZE 64

struct vfs_node;
struct vfs_filesystem;

struct initrd_node
{

    char name[32];
    struct vfs_node base;
    unsigned int size;
    struct tar_header *header;
    void *data;

};

struct initrd_filesystem
{

    struct vfs_filesystem base;
    struct initrd_node nodes[INITRD_HEADER_SIZE];
    unsigned int nodesCount;

};

extern void initrd_init(unsigned int initrdc, void **initrdv);

#endif

