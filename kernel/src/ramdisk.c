#include <memory.h>
#include <string.h>
#include <tar.h>
#include <ramdisk.h>

static struct ramdisk_image image;

static unsigned int parse(void *address)
{

    unsigned int i;
    char *current = address;

    for (i = 0; *current; i++)
    {

        struct tar_header *header = (struct tar_header *)current;
        unsigned int size = string_read_num(header->size, 8);

        if (!size)
        {

            ramdisk_node_init(&image.nodes[i], header, 0);

            current += TAR_BLOCK_SIZE;

        }

        else
        {

            ramdisk_node_init(&image.nodes[i], header, (unsigned int)(current + TAR_BLOCK_SIZE));

            current += ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;

        }

    }

    return i;

}

void ramdisk_node_init(struct ramdisk_node *node, struct tar_header *header, unsigned int offset)
{

    memory_clear(node, sizeof (struct ramdisk_node));

    node->header = header;
    node->offset = offset;

}

void ramdisk_image_init(struct ramdisk_image *image)
{

    memory_clear(image, sizeof (struct ramdisk_image));

}

struct ramdisk_image *ramdisk_setup(unsigned int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_image_init(&image);

    for (i = 0; i < ramdiskc; i++)
        image.count += parse(*(ramdiskv + i));

    return &image;

}

