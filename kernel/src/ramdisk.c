#include <memory.h>
#include <string.h>
#include <tar.h>
#include <vfs.h>
#include <ramdisk.h>

static struct ramdisk_image ramdiskImage;
static struct ramdisk_filesystem ramdiskFilesystem;

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

static unsigned int parse(struct ramdisk_image *self, void *address)
{

    char *current;

    for (current = address; *current; current += TAR_BLOCK_SIZE)
    {

        struct tar_header *header = (struct tar_header *)current;
        unsigned int size = string_read_num(header->size, 8);

        if (!validate(header))
            return 0;

        self->headers[self->count] = header;
        self->count++;

        current += ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 1 : 0)) * TAR_BLOCK_SIZE;

    }

    return self->count;

}

static unsigned int parent(struct ramdisk_filesystem *filesystem, unsigned int id)
{

    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);
    unsigned int i;

    while (--length)
    {

        if (header->name[length - 1] == '/')
            break;

    }

    for (i = 0; i < id - 1; i++)
    {

        if (!memory_match(filesystem->image->headers[i]->name, header->name, length))
            continue;

        return i + 1;

    }

    return 1;

}

static unsigned int read_directory(struct ramdisk_filesystem *filesystem, struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    char temp[1024];
    unsigned int o = 0;
    unsigned int c = 1024;
    unsigned int i;
    unsigned int length = string_length(header->name);

    o += vfs_write(temp, c - o, "../\n", 4, o);

    for (i = 0; i < filesystem->image->count; i++)
    {

        unsigned int p = parent(filesystem, i + 1) - 1;

        if (filesystem->image->headers[i] == header)
            continue;

        if (filesystem->image->headers[p] != header)
            continue;

        o += vfs_write(temp, c - o, filesystem->image->headers[i]->name + length, string_length(filesystem->image->headers[i]->name) - length, o);
        o += vfs_write(temp, c - o, "\n", 2, o);

    }

    return vfs_read(buffer, count, temp, o, offset);

}

static unsigned int read_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return vfs_read(buffer, count, (void *)data, size, offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return read_directory(filesystem, header, offset, count, buffer);

    return read_file(header, offset, count, buffer);

}

static unsigned int write_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return vfs_write((void *)data, size, buffer, count, offset);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return 0;

    return write_file(header, offset, count, buffer);

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);
    unsigned int i;

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, parent(filesystem, id), count - 3, path + 3);

    for (i = id; i < filesystem->image->count; i++)
    {

        unsigned int clength = string_length(filesystem->image->headers[i]->name);

        if (clength < length)
            break;

        clength -= length;

        if (!memory_match(filesystem->image->headers[i]->name + length, path, clength))
            continue;

        return walk(self, i + 1, count - clength, path + clength);

    }

    return 0;

}

static unsigned int get_physical(struct vfs_interface *self, unsigned int id)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return data;

}

void ramdisk_init_image(struct ramdisk_image *image)
{

    memory_clear(image, sizeof (struct ramdisk_image));

    image->parse = parse;

}

void ramdisk_init_filesystem(struct ramdisk_filesystem *filesystem, struct ramdisk_image *image)
{

    memory_clear(filesystem, sizeof (struct ramdisk_filesystem));

    vfs_init_interface(&filesystem->interface, 1, "ramdisk", 0, 0, read, write, walk, get_physical);

    filesystem->image = image;

}

struct vfs_interface *ramdisk_setup(int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_init_image(&ramdiskImage);

    for (i = 0; i < ramdiskc; i++)
        ramdiskImage.parse(&ramdiskImage, *(ramdiskv + i));

    ramdisk_init_filesystem(&ramdiskFilesystem, &ramdiskImage);

    return &ramdiskFilesystem.interface;

}

