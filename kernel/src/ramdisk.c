#include <memory.h>
#include <string.h>
#include <tar.h>
#include <ramdisk.h>

static unsigned int validate(struct tar_header *header)
{

    unsigned int i;
    unsigned char *address = (unsigned char *)header;
    unsigned int checksum = string_read_num(header->checksum, 8);

    for (i = 0; i < 148; i++)
        checksum -= address[i];

    for (i = 148; i < 156; i++)
        checksum -= 32;

    for (i = 156; i < TAR_BLOCK_SIZE; i++)
        checksum -= address[i];

    return !checksum;

}

static unsigned int parse(struct ramdisk_image *image, void *address)
{

    char *current;

    for (current = address; *current; current += TAR_BLOCK_SIZE)
    {

        struct tar_header *header = (struct tar_header *)current;
        unsigned int size = string_read_num(header->size, 8);

        if (!validate(header))
            break;

        image->headers[image->count] = header;
        image->count++;

        current += ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 1 : 0)) * TAR_BLOCK_SIZE;

    }

    return image->count;

}

void ramdisk_image_init(struct ramdisk_image *image)
{

    memory_clear(image, sizeof (struct ramdisk_image));

    image->parse = parse;

}

