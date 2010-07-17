#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/isr.h>
#include <kernel/kbd.h>
#include <kernel/screen.h>
#include <kernel/initrd.h>

struct initrd_header *initrdHeader;
struct initrd_file_header *initrdFileHeaders;
struct vfs_node initrdRoot;
struct vfs_node initrdNodes[32];

static unsigned int initrd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_file_header header = initrdFileHeaders[node->inode];

    if (offset > header.length)
        return 0;

    if (offset + count > header.length)
        count = header.length - offset;

    memory_copy(buffer, (unsigned char*)(header.offset + offset), count);

    return count;

}

static struct vfs_node *initrd_walk(struct vfs_node *node, unsigned int index)
{

    if (index < initrdHeader->nfiles)
        return &initrdNodes[index];
    else
        return 0;

}

struct vfs_node *initrd_init(unsigned int location)
{

    initrdHeader = (struct initrd_header *)location;
    initrdFileHeaders = (struct initrd_file_header *)(location + sizeof (struct initrd_header));

    string_copy(initrdRoot.name, "initrd");
    initrdRoot.inode = 0;
    initrdRoot.length = 0;
    initrdRoot.open = 0;
    initrdRoot.close = 0;
    initrdRoot.read = 0;
    initrdRoot.write = 0;
    initrdRoot.walk = initrd_walk;

    unsigned int i;

    for (i = 0; i < initrdHeader->nfiles; i++)
    {

        initrdFileHeaders[i].offset += location;
        string_copy(initrdNodes[i].name, initrdFileHeaders[i].name);
        initrdNodes[i].inode = i;
        initrdNodes[i].length = initrdFileHeaders[i].length;
        initrdNodes[i].open = 0;
        initrdNodes[i].close = 0;
        initrdNodes[i].read = initrd_read;
        initrdNodes[i].write = 0;
        initrdNodes[i].walk = 0;

    }

    initrdHeader->nfiles++;
    initrdNodes[i++] = *kbd_get_node();

    initrdHeader->nfiles++;
    initrdNodes[i++] = *screen_get_node();

    return &initrdRoot;

}

