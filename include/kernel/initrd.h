#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

#define INITRD_HEADER_SIZE 64

struct modules_filesystem;

struct initrd_filesystem
{

    struct modules_filesystem base;
    struct tar_header *headers[INITRD_HEADER_SIZE];
    struct file_node *nodes[INITRD_HEADER_SIZE];

};

extern void initrd_init(unsigned int address);

#endif

