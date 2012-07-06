#include <memory.h>
#include <string.h>
#include <tar.h>
#include <ramdisk.h>
#include <vfs/ramdisk.h>

static struct ramdisk_node nodes[RAMDISK_NODE_SLOTS];

static unsigned int parse(void *address)
{

    unsigned int i;
    char *current = address;

    for (i = 0; *current; i++)
    {

        struct tar_header *header = (struct tar_header *)current;
        unsigned int size = string_read_num(header->size, 8);

        ramdisk_node_init(&nodes[i], header->name + 6, size, header, (unsigned int)(current + TAR_BLOCK_SIZE));

        current += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            current += TAR_BLOCK_SIZE;

    }

    return i;

}

void ramdisk_node_init(struct ramdisk_node *node, char *name, unsigned int size, struct tar_header *header, unsigned int offset)
{

    memory_clear(node, sizeof (struct ramdisk_node));

    node->name = name;
    node->size = size;
    node->header = header;
    node->offset = offset;

}

void ramdisk_setup(unsigned int ramdiskc, void **ramdiskv)
{

    unsigned int i;
    unsigned int count = 0;

    for (i = 0; i < ramdiskc; i++)
        count += parse(*(ramdiskv + i));

    vfs_ramdisk_setup(nodes, count);

}

