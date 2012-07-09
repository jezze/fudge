#ifndef KERNEL_RAMDISK_H
#define KERNEL_RAMDISK_H

#define RAMDISK_NODE_SLOTS 64

struct ramdisk_node
{

    char *name;
    unsigned int size;
    struct tar_header *header;
    unsigned int offset;

};

struct ramdisk_image
{

    struct ramdisk_node nodes[RAMDISK_NODE_SLOTS];
    unsigned int count;

};

void ramdisk_node_init(struct ramdisk_node *node, char *name, unsigned int size, struct tar_header *header, unsigned int offset);
void ramdisk_image_init(struct ramdisk_image *image, unsigned int ramdiskc, void **ramdiskv);
struct ramdisk_image *ramdisk_setup(unsigned int ramdiskc, void **ramdiskv);

#endif

