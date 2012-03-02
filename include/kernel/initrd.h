#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

#define INITRD_HEADER_SIZE 64

struct initrd_node
{

    char *name;
    unsigned int size;
    struct tar_header *header;
    void *data;

};

void initrd_node_init(struct initrd_node *node, char *name, unsigned int size, struct tar_header *header, void *data);
void initrd_init(unsigned int initrdc, void **initrdv);

#endif

