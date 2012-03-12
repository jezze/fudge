#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/ramdisk.h>

struct ramdisk_node nodes[RAMDISK_HEADER_SIZE];
unsigned int nodesCount;

static unsigned int parse(void *address)
{

    unsigned int i;

    for (i = 0; *(char *)address; i++)
    {

        struct tar_header *header = address;
        unsigned int size = string_read_num(header->size, 8);

        ramdisk_node_init(&nodes[i], header->name + 11, size, header, address + TAR_BLOCK_SIZE);

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return i;

}

void ramdisk_node_init(struct ramdisk_node *node, char *name, unsigned int size, struct tar_header *header, void *data)
{

    memory_clear(node, sizeof (struct ramdisk_node));

    node->name = name;
    node->size = size;
    node->header = header;
    node->data = data;

}

void ramdisk_init(unsigned int ramdiskc, void **ramdiskv)
{

    nodesCount = 0;

    unsigned int i;

    for (i = 0; i < ramdiskc; i++)
        nodesCount += parse(*(ramdiskv + i));

}

