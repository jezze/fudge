#ifndef KERNEL_RAMDISK_H
#define KERNEL_RAMDISK_H

#define RAMDISK_NODE_SLOTS 64

struct tar_header;

struct ramdisk_node
{

    char *name;
    unsigned int size;
    struct tar_header *header;
    void *data;

};

void ramdisk_node_init(struct ramdisk_node *node, char *name, unsigned int size, struct tar_header *header, void *data);
void ramdisk_init(unsigned int ramdiskc, void **ramdiskv);

#endif

