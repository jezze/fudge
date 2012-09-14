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

static unsigned int read_file(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int size = string_read_num(header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    if (c > count)
        c = count;

    memory_copy(buffer, (void *)(data + offset), c);

    return c;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
    {

        unsigned int i;
        unsigned int c = 0;
        char *out = buffer;

        if (offset > 0)
            return 0;

        for (i = 0; i < filesystem->image->count; i++)
        {

            struct tar_header *current = filesystem->image->headers[i];
            char *start;
            char *slash;
            unsigned int size;

            if (current == header)
                continue;

            if (!memory_find(current->name, header->name, string_length(current->name), string_length(header->name)))
                continue;

            start = current->name + string_length(header->name);
            size = string_length(start);

            if (!size)
                continue;

            slash = memory_find(start, "/", size, 1);

            if (slash && slash < start + size - 1)
                continue;

            memory_copy(out + c, start, size);
            memory_copy(out + c + size, "\n", 1);
            c += size + 1;

        }

        return c;

    }

    return read_file(self, id, offset, count, buffer);

}

static unsigned int write_file(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int size = string_read_num(header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    if (c > count)
        c = count;

    memory_copy((void *)(data + offset), buffer, c);

    return c;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return 0;

    return write_file(self, id, offset, count, buffer);

}

static unsigned int parent(struct vfs_interface *self, unsigned int id)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int count = string_length(header->name);
    unsigned int i;

    for (i = count; i > 0; i--)
    {

        if (header->name[i] == '/')
            count = i;

    }

    for (i = 0; i < id; i++)
    {

        if (memory_match(filesystem->image->headers[i]->name, header->name, count))
            return i + 1;

    }

    return 1;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int offset = string_length(header->name);

    unsigned int i;

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        walk(self, parent(self, id), count - 3, path + 3);

    for (i = id; i < filesystem->image->count; i++)
    {

        unsigned int length = string_length(filesystem->image->headers[i]->name);

        if (length < offset)
            break;

        length -= offset;

        if (!memory_match(filesystem->image->headers[i]->name + offset, path, length))
            continue;

        return walk(self, i + 1, count - length, path + length);

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
